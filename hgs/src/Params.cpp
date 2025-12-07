#include "Params.h"

// The universal constructor for both executable and shared library
// When the executable is run from the commandline,
// it will first generate an CVRPLIB instance from .vrp file, then supply necessary information.
Params::Params(
	const std::vector<double>& x_coords,
	const std::vector<double>& y_coords,
	const std::vector<std::vector<double>>& dist_mtx,
	const std::vector<double>& service_time,
	const std::vector<double>& demands,
	const std::vector<double> due_data,
	const std::vector<double> ready_time,
	double vehicleCapacity,
	int nbVeh,
	bool isDurationConstraint,
	bool verbose,
	const AlgorithmParameters& ap
)	//这一串代码可以提高效率，必要的初始化可以放到成员初始化列表
	: ap(ap), isDurationConstraint(isDurationConstraint), nbVehicles(nbVeh),
	  vehicleCapacity(vehicleCapacity), timeCost(dist_mtx), verbose(verbose)//看看车辆是否进一步提供
{
	// This marks the starting time of the algorithm
	startTime = clock();	///记录算法开始的时间
	boolNum=7000;
	boolinner=500;

	nbClients = (int)demands.size() - 1; // Need to substract the depot from the number of nodes
	//这边要转化为int的原因：size返回的是无符号的整形std::vector::size_type 类型
	totalDemand = 0.;
	maxDemand = 0.;

	// Initialize RNG，确定一个种子
	ran.seed(ap.seed);

	// check if valid coordinates are provided
	// 判断坐标的数量是否合理
	areCoordinatesProvided = (demands.size() == x_coords.size()) && (demands.size() == y_coords.size());
	pretimeWindow = std::vector<std::vector<double>>(nbClients+1);
	reducelimit = std::vector<double>(nbClients + 1, 0.0);
	cli = std::vector<Client>(nbClients + 1);	//初始化一下
	for (int i = 0; i <= nbClients; i++)
	{
		// If useSwapStar==false, x_coords and y_coords may be empty.
		if (ap.useSwapStar == 1 && areCoordinatesProvided)
		{
			cli[i].coordX = x_coords[i];
			cli[i].coordY = y_coords[i];
			cli[i].polarAngle = CircleSector::positive_mod(
				32768. * atan2(cli[i].coordY - cli[0].coordY, cli[i].coordX - cli[0].coordX) / PI);
		}
		else
		{
			cli[i].coordX = 0.0;
			cli[i].coordY = 0.0;
			cli[i].polarAngle = 0.0;
		}

		cli[i].serviceDuration = service_time[i];
		cli[i].demand = demands[i];
		cli[i].timeWindow[0] = ready_time[i];
		cli[i].timeWindow[1] = due_data[i];
		pretimeWindow[i].push_back(ready_time[i]);	//另外再存储一点数据
		pretimeWindow[i].push_back(due_data[i]);
		reducelimit.push_back((due_data[i]-ready_time[i])*0.7);	//初始化一下，后面会用到
		if (cli[i].demand > maxDemand) maxDemand = cli[i].demand;
		//maxdemand主要是在后面自适应惩罚参数的调整里面要用到
		totalDemand += cli[i].demand;
	}
	mincar=static_cast<int>(std::ceil(totalDemand/vehicleCapacity));	//获得当前最小车辆数
	if (verbose && ap.useSwapStar == 1 && !areCoordinatesProvided)
		std::cout << "----- NO COORDINATES HAVE BEEN PROVIDED, SWAP* NEIGHBORHOOD WILL BE DEACTIVATED BY DEFAULT" << std::endl;

	// Default initialization if the number of vehicles has not been provided by the user，如果车辆数量没有提供的话
	if (nbVehicles == INT_MAX)
	{
		nbVehicles = (int)std::ceil(1.3*totalDemand/vehicleCapacity) + 3;  // Safety margin: 30% + 3 more vehicles than the trivial bin packing LB
		if (verbose) 
			std::cout << "----- FLEET SIZE WAS NOT SPECIFIED: DEFAULT INITIALIZATION TO " << nbVehicles << " VEHICLES" << std::endl;
	}
	else
	{
		if (verbose)
			std::cout << "----- FLEET SIZE SPECIFIED: SET TO " << nbVehicles << " VEHICLES" << std::endl;
	}

	// Calculation of the maximum distance，统计两点之间的最大距离
	maxDist = 0.;
	for (int i = 0; i <= nbClients; i++)
		for (int j = 0; j <= nbClients; j++)
			if (timeCost[i][j] > maxDist) maxDist = timeCost[i][j];

	// Calculation of the correlated vertices for each customer (for the granular restriction)，计算每个客户的相关订单，在细粒度约束下
	correlatedVertices = std::vector<std::vector<int> >(nbClients + 1);

	std::vector<std::set<int> > setCorrelatedVertices = std::vector<std::set<int> >(nbClients + 1);
	std::vector<std::pair<double, int> > orderProximity;
	//这个变量是用来保存订单的相似程度的，也就是和当前点的距离远近情况
	for (int i = 1; i <= nbClients; i++)
	{
		orderProximity.clear();//清空容器，可以移除所有的值，将容器的大小（size）设置为0，但是容器的容量不会改变
		for (int j = 1; j <= nbClients; j++)
			if (i != j) orderProximity.emplace_back(timeCost[i][j], j);
		std::sort(orderProximity.begin(), orderProximity.end());

		for (int j = 0; j < std::min<int>(ap.nbGranular, nbClients - 1); j++)
		{
			// If i is correlated with j, then j should be correlated with i
			setCorrelatedVertices[i].insert(orderProximity[j].second);
			setCorrelatedVertices[orderProximity[j].second].insert(i);
		}
	}

	// Filling the vector of correlated vertices
	//这个for循环主要是为了数据转换；set会自动对数据类型进行排序
	for (int i = 1; i <= nbClients; i++)
		for (int x : setCorrelatedVertices[i])	//这个相当于遍历setCorrelatedVertices[i]集合中的每一个元素
			correlatedVertices[i].push_back(x);	//已经有确定对应元素可以插入

	//要专门搞一个set出来是为了防止顶点重复

	// Safeguards to avoid possible numerical instability in case of instances containing arbitrarily small or large numerical values
	if (maxDist < 0.1 || maxDist > 1000000)
		throw std::string(
			"The distances are of very small or large scale. This could impact numerical stability. Please rescale the dataset and run again.");
	//这个地方的上下限设置可能会产生问题的
	if (maxDemand < 0.1 || maxDemand > 1000000)
		throw std::string(
			"The demand quantities are of very small or large scale. This could impact numerical stability. Please rescale the dataset and run again.");		//这种throw相当于是抛出错误
	if (nbVehicles < std::ceil(totalDemand / vehicleCapacity))
		throw std::string("Fleet size is insufficient to service the considered clients.");

	// A reasonable scale for the initial values of the penalties
	penaltyEarly = 0;		//10  20 50，因为初赛要尽可能距离短，可以考虑车辆多点，所以早到从1到0
	penaltyLate = 5;	//早到感觉要比迟到更加严重一点
	penaltyCapacity = 5;
	penDis = 1;			//一开始部分的时候都等于1
	routepen = 1000;		//这个是要动态调整的,500
	cpencap = penaltyCapacity;
	cpeneary = penaltyEarly;
	cpenlate = penaltyLate;
	cpendis = penDis;
	dclusterNum=std::ceil(totalDemand / vehicleCapacity)+nbClients/100;		//车辆的聚类数量
	// dclusterNum=16;		//车辆的聚类数量
	if (verbose)
		std::cout << "----- INSTANCE SUCCESSFULLY LOADED WITH " << nbClients << " CLIENTS AND " << nbVehicles << " VEHICLES" << std::endl;
}