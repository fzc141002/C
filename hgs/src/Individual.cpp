#include "Individual.h" 
#include "StartTime.h"

//对于价值的估计要好好考虑一下
void Individual::evaluateCompleteCost(const Params& params)
{
	eval = EvalIndiv();	//用了结构体的默认构造函数进行初始化
	//这个地方直接粗线条的判断可能会有问题
	vvd timeR, etimeR;	//注意这个地方把depot也保存下来了
	int isStartFeasible = 1;	//只要有一个不可行就是不可行
	//一开始默认都是1
	double sumTime = 0;		//把实际的时间都保存下来
	double sumDis = 0;		//把实际的距离都保存下来
	double sumETime = 0;	//把早的时间都保存下来
	double sumLTime = 0;	//把迟的时间都保存下来
	int countNum =0 ;
	ind_clear();	//把内部元素都初始化了
	eval.eval_clear();
	for (int r = 0; r < params.nbVehicles; r++)
	{
		
		if (!chromR[r].empty())
		{
			countNum+=(int)chromR[r].size();
			int inner_feasible = 1;		//判断这个解是否可行
			eval.nbRoutes++;
			double load = params.cli[chromR[r][0]].demand;	//load是对于每一辆车来说的
			double etimeInner=0.0;
			double ltimeInner=0.0;
			double roadCost=params.timeCost[0][chromR[r][0]];
			sumDis += params.timeCost[0][chromR[r][0]];	//从depot到第一个客户点的距离
			predecessors[chromR[r][0]] = 0;
			GetStartTime(params, chromR[r], timeR, etimeR, isStartFeasible);
			// CheckFeasible(params, chromR[r], timeR, etimeR);
			timeRS.push_back(timeR);
			etimeRS.push_back(etimeR);
			// if (etimeR[1]<0) sumETime -= etimeR[1];	//其实第一个点一定是大于等于0的，所以不用考虑小于0的情况
			if (etimeR[1]>0) {sumLTime += etimeR[1];inner_feasible=0;ltimeInner+= etimeR[1];} 
			//注意timeR中一共保存了客户点+2depot
			for (int i = 1; i < chromR[r].size(); i++)
			{
				sumDis += params.timeCost[chromR[r][i - 1]][chromR[r][i]];
				roadCost += params.timeCost[chromR[r][i - 1]][chromR[r][i]];
				load += params.cli[chromR[r][i]].demand;
				if (etimeR[i + 1] < 0)//小于0早到
				{
					sumETime -= etimeR[i + 1];
					etimeInner -= etimeR[i + 1];	//早到的时间
				}	
				else if (etimeR[i + 1] > 0)
				{
					sumLTime += etimeR[i + 1];
					ltimeInner += etimeR[i + 1];	//迟到的时间
					inner_feasible=0;	//迟到了就不可行了
				}
				predecessors[chromR[r][i]] = chromR[r][i - 1];
				successors[chromR[r][i - 1]] = chromR[r][i];
			}
			loadRS.push_back(load);	//把载重也放进去
			sumDis += params.timeCost[chromR[r][chromR[r].size() - 1]][0];	//最后一个客户点回到depot的距离
			roadCost += params.timeCost[chromR[r][chromR[r].size() - 1]][0];	//最后一个客户点回到depot的距离
			//最后回去depot可能有，也可能没有，不可能早到
			sumLTime += etimeR[etimeR.size() - 1];
			ltimeInner += etimeR[etimeR.size() - 1];	//最后一个点的时间
			if (etimeR[etimeR.size() - 1] > 0) inner_feasible = 0;	//不考虑重量了
			chromRFeasible.push_back(inner_feasible);	//把可行性更新进去,保存的也是时间可行性
			double loadExcess = load - params.vehicleCapacity;	//计算载重超出多少
			double loadcost=0;
			if (loadExcess > 0)	//如果载重超出的话
				loadcost = loadExcess * params.penaltyCapacity;	//计算载重超出惩罚
			else loadcost = 0.0;
			chromRCost.push_back(roadCost+loadcost+etimeInner*params.penaltyEarly+ltimeInner*params.penaltyLate);	//把可行性更新进去,保存的也是时间可行性
			if (load > params.vehicleCapacity)
				eval.capacityExcess += load - params.vehicleCapacity;
			successors[chromR[r][chromR[r].size() - 1]] = 0;
			//一个路径的总时间就是最后一个点的时间，因为最后要回到depot的
			sumTime += timeR[timeR.size() - 1];
		}
	}
	cliNum=countNum;		//需要把客户数更新一下
	// eval.distance = sumTime;		//这个distance记录的时间，把这个改成真实距离
	eval.distance = sumDis;		//这个distance记录的时间，把这个改成真实距离
	eval.timeEarlyExcess = sumETime;
	eval.timeLateExcess = sumLTime;
	eval.isFeasible = (eval.capacityExcess < MY_EPSILON && eval.timeLateExcess < MY_EPSILON && eval.nbRoutes<=params.nbVehicles);
	// eval.penalizedCost = eval.distance + eval.capacityExcess * params.penaltyCapacity + eval.timeEarlyExcess * params.penaltyEarly + eval.timeLateExcess * params.penaltyLate;
	//首先惩罚要不要改变一下
	eval.penalizedCost = eval.distance + eval.nbRoutes*params.ap.routeWeight+
	eval.capacityExcess * params.penaltyCapacity + eval.timeEarlyExcess * params.penaltyEarly + eval.timeLateExcess * params.penaltyLate;
}

//这个就是随机初始化
Individual::Individual(Params & params)
{
	successors = std::vector <int>(params.nbClients + 1);
	predecessors = std::vector <int>(params.nbClients + 1);
	chromR = std::vector < std::vector <int> >(params.nbVehicles);
	chromT = std::vector <int>(params.nbClients);
	cliNum = params.nbClients;	//客户点的数量，可能遇到需要更新的情况
	for (int i = 0; i < params.nbClients; i++) chromT[i] = i + 1;
	std::shuffle(chromT.begin(), chromT.end(), params.ran);
	//shuffle是algorithm中的一个函数，随机打乱元素的个数，将chromT中的元素随机打乱
	//但是用了固定的随机数种子，所以每次的结果都是一样的
	eval.penalizedCost = 1.e30;	//定义一个很大的值
}

//这个是根据clusters来初始化的
void Individual::IndividualThree(Params & params,Initial & initial,int Istime)
{
	chromT.clear(); //先清空
	if (Istime == 0)
	{
		for (int i=0;i<params.dclusterNum;i++)
		{
			if (initial.disCluted[i].empty()) continue;	//如果这个集群是空的就跳过
			std::vector<int> inner_cluster = initial.disCluted[i];
			std::shuffle(inner_cluster.begin(), inner_cluster.end(), params.ran);
			chromT.insert(chromT.end(),inner_cluster.begin(),inner_cluster.end());
		}
	}
	else if (Istime == 1)
	{
		std::vector<std::vector<int>> temp_clusters = initial.timeCluted;
		int nbcus=params.nbClients;
		while (true)
		{
			for (int i = 0; i < params.tclusterNum; i++)
			{
				if (!temp_clusters[i].empty())	//如果这个非空
				{
					std::uniform_int_distribution<> distr(0,temp_clusters[i].size()-1);
					int ran_pos= distr(params.ran);	//随机选择一个位置
					chromT.push_back(temp_clusters[i][ran_pos]);	//把这个点放进去
					temp_clusters[i].erase(temp_clusters[i].begin() + ran_pos);	//把这个点从原来的集群中删除
					nbcus--;
				}
			}
			if (nbcus == 0) break;
		}
	}
	else	//最后一种是随机初始化
	{
		//就只有random的时候才需要一开始就有
		chromT = std::vector <int>(params.nbClients);
		for (int i = 0; i < params.nbClients; i++) chromT[i] = i + 1;
		std::shuffle(chromT.begin(), chromT.end(), params.ran);
	}
}

Individual::Individual(Params & params, std::string fileName) : Individual(params)
{
    chromT.clear();
    chromR = std::vector<std::vector<int>>(params.nbVehicles);
    std::ifstream inputFile(fileName);
    if (!inputFile.is_open())
        throw std::string("Impossible to open solution file provided in input in : " + fileName);

    std::string line;
    // 跳到Result部分
    while (std::getline(inputFile, line)) {
        if (line.find("======================Result======================") != std::string::npos)
            break;
    }

    int routeIdx = 0;
    while (std::getline(inputFile, line)) {
        if (line.find("======================Total======================") != std::string::npos)
            break;
        if (line.find("Route #") == 0) {
            // 解析客户点
            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;
            std::string customersStr = line.substr(colonPos + 1);
            std::vector<int> route;
            std::stringstream ss(customersStr);
            std::string token;
            while (std::getline(ss, token, '-')) {
                // 跳过 ">" 和空格
                size_t numStart = token.find_first_of("0123456789");
                if (numStart != std::string::npos) {
                    int customer = std::stoi(token.substr(numStart));
                    if (customer != 0) { // depot不加入
                        route.push_back(customer);
                        chromT.push_back(customer);
                    }
                }
            }
            if (!route.empty() && routeIdx < chromR.size())
                chromR[routeIdx++] = route;
        }
    }

    // 校验
    evaluateCompleteCost(params);
    if ((int)chromT.size() != params.nbClients)
        throw std::string("Input solution does not contain the correct number of clients");
    if (!eval.isFeasible)
        throw std::string("Input solution is infeasible");
    if (params.verbose)
        std::cout << "----- INPUT SOLUTION HAS BEEN SUCCESSFULLY READ (TXT FORMAT) WITH COST " << eval.penalizedCost << std::endl;
}

void Individual::copy(const Individual& indiv)
{
	this->eval = indiv.eval;
	this->chromT = indiv.chromT;
	this->chromR = indiv.chromR;
	this->timeRS = indiv.timeRS;
	this->etimeRS = indiv.etimeRS;
	this->loadRS = indiv.loadRS;
	this->chromRFeasible = indiv.chromRFeasible;
	this->successors = indiv.successors;
	this->predecessors = indiv.predecessors;
	this->indivsPerProximity = indiv.indivsPerProximity;
	this->cliNum = indiv.cliNum;
	this->biasedFitness = indiv.biasedFitness;
}

void Individual::check() const
{
	std::vector<int> cusRecord;
	for (int i=0;i<(int)chromR.size();i++)
	{
		if (!chromR[i].empty())
		{
			for (int i:chromR[i])
			{
				if (!cusRecord.empty() && std::find(cusRecord.begin(),cusRecord.end(),i)==cusRecord.end())
				{
					cusRecord.push_back(i);	//把i元素放进去
				}
				if (cusRecord.empty()) cusRecord.push_back(i);
			}
		}
	}
	if ((int)cusRecord.size()!=cliNum) 
	std::cout<<"客户点数量不一致"<<std::endl;
	else
	std::cout<<"客户点的数量为"<<cliNum<<std::endl;
}
