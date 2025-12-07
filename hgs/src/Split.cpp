#include "Split.h" 

void Split::generalSplit(Individual & indiv, int nbMaxVehicles)
{
	// Do not apply Split with fewer vehicles than the trivial (LP) bin packing bound
	maxVehicles = std::max<int>(nbMaxVehicles, std::ceil(params.totalDemand/params.vehicleCapacity));	//车辆数量足够
	// Initialization of the data structures for the linear split algorithms
	// Direct application of the code located at https://github.com/vidalt/Split-Library
	for (int i = 1; i <= params.nbClients; i++)
	{
		//为什么都是到前一个点，这个chromT里面没有depot
		cliSplit[i].demand = params.cli[indiv.chromT[i - 1]].demand;
		cliSplit[i].serviceTime = params.cli[indiv.chromT[i - 1]].serviceDuration;
		cliSplit[i].d0_x = params.timeCost[0][indiv.chromT[i - 1]];
		cliSplit[i].dx_0 = params.timeCost[indiv.chromT[i - 1]][0];
		cliSplit[i].c_id = indiv.chromT[i - 1];	//第一个点应该是第0个点
		if (i < params.nbClients) cliSplit[i].dnext = params.timeCost[indiv.chromT[i - 1]][indiv.chromT[i]];//到下一个点的距离
		else cliSplit[i].dnext = -1.e30;
		//初始化基本参数
		sumLoad[i] = sumLoad[i - 1] + cliSplit[i].demand;//
		sumService[i] = sumService[i - 1] + cliSplit[i].serviceTime;//服务时间没有用到
		sumDistance[i] = sumDistance[i - 1] + cliSplit[i - 1].dnext;
		
	}

	// We first try the simple split, and then the Split with limited fleet if this is not successful
	if (splitSimple(indiv) == 0)
		splitLF(indiv);

	// Build up the rest of the Individual structure
	indiv.evaluateCompleteCost(params);
}

//这个函数相当于是做bellman分割
int Split::splitSimple(Individual & indiv)
{
	// Reinitialize the potential structures，Bellman
	potential[0][0] = 0;//不需要分很多车，因为车足够
	for (int i = 1; i <= params.nbClients; i++)
		potential[0][i] = 1.e30;

	// MAIN ALGORITHM -- Simple Split using Bellman's algorithm in topological order
	// This code has been maintained as it is very simple and can be easily adapted to a variety of constraints, whereas the O(n) Split has a more restricted application scope
	if (params.isDurationConstraint)	//如果有时间限制的话
	{
		//需要考虑前后点的关系
		for (int i = 0; i < params.nbClients; i++)
		{
			double load = 0.;
			double distance = 0.;	//所以distance是可以在一次循环中累积的
			double serviceDuration = 0.;
			//当客户点j加入到从节点i出发的路径中时；cliSplit中顺序每次都是不一样的
			double etimeSum = 0.;//分别记录早到时间和迟到时间
			double ltimeSum = 0.;
			//所有的循环都是从depot出发的
			//倾向于分配到下一个点的原因是：一开始重新从仓库出发的话，惩罚成本就归零了
			double timeSum = std::max<double>(params.cli[cliSplit[i + 1].c_id].timeWindow[0] - params.timeCost[0][cliSplit[i + 1].c_id], 0);	  //定义一个从depot出发的时间
			for (int j = i + 1; j <= params.nbClients && load <= 1.5 * params.vehicleCapacity ; j++)	 //这个1.5感觉也是一个超参
			{
				load += cliSplit[j].demand;
				serviceDuration += cliSplit[j].serviceTime;
				if (j == i + 1)
				{
					timeSum += cliSplit[j].d0_x;
					distance += cliSplit[j].d0_x;
				}
				else
				{
					timeSum += cliSplit[j - 1].dnext;
					distance += cliSplit[j - 1].dnext;
				}
				if (timeSum + MY_EPSILON < params.cli[cliSplit[j].c_id].timeWindow[0])  //如果早于时间窗
				{
					etimeSum += params.cli[cliSplit[j].c_id].timeWindow[0] - timeSum;
					timeSum = params.cli[cliSplit[j].c_id].timeWindow[0] + params.cli[cliSplit[j].c_id].serviceDuration;
				}
				else
				{
					if (timeSum + MY_EPSILON < params.cli[cliSplit[j].c_id].timeWindow[1])
					{
						timeSum += params.cli[cliSplit[j].c_id].serviceDuration;
					}
					else
					{
						ltimeSum += timeSum - params.cli[cliSplit[j].c_id].timeWindow[1];
						timeSum += params.cli[cliSplit[j].c_id].serviceDuration;
					}
				}
				//这个参数暂时先设置成这样子
				double cost = distance + cliSplit[j].dx_0
					+ params.penaltyCapacity * std::max<double>(load - params.vehicleCapacity, 0.)
					+ params.penaltyEarly*0.001 * etimeSum + params.penaltyLate*0.001 * ltimeSum;
				if (potential[0][i] + cost < potential[0][j])
				{
					potential[0][j] = potential[0][i] + cost;
					pred[0][j] = i;		//算法1
				}
			}
		}
	}
	else
	{	//在没有时间限制的情况下可以使用
		Trivial_Deque queue = Trivial_Deque(params.nbClients + 1, 0);
		for (int i = 1; i <= params.nbClients; i++)
		{
			// The front is the best predecessor for i
			potential[0][i] = propagate(queue.get_front(), i, 0);
			pred[0][i] = queue.get_front();

			if (i < params.nbClients)
			{
				// If i is not dominated by the last of the pile
				if (!dominates(queue.get_back(), i, 0))
				{
					// then i will be inserted, need to remove whoever is dominated by i.移除被i支配的那一项
					while (queue.size() > 0 && dominatesRight(queue.get_back(), i, 0))	//当尾部的那一项被i支配的时候，就需要被移除，移除完之后把i放进去
						queue.pop_back();
					queue.push_back(i);
				}
				// Check iteratively if front is dominated by the next front
				//成本也可以作为支配的标准
				while (queue.size() > 1 && propagate(queue.get_front(), i + 1, 0) > propagate(queue.get_next_front(), i + 1, 0) - MY_EPSILON)
					queue.pop_front();
			}
		}
	}
	if (potential[0][params.nbClients] > 1.e29)
		throw std::string("ERROR : no Split solution has been propagated until the last node"); //没有一个解的方案可以被传递到最后

	// Filling the chromR structure
	for (int k = params.nbVehicles - 1; k >= maxVehicles; k--)
		indiv.chromR[k].clear();

	int end = params.nbClients;
	
	for (int k = 0; k<maxVehicles; k++)	//遍历每一辆车
	{
		indiv.chromR[k].clear();		//把第k辆车清空
		int begin = pred[0][end];		//从最后一个客户的前序节点开始，因为是逆序查找的
		//pred只要保存在一条上就可以
		for (int ii = begin; ii < end; ii++)
			indiv.chromR[k].push_back(indiv.chromT[ii]);	//注意这些点放进去的时候都是连续的
		end = begin;
	}
	// Return OK in case the Split algorithm reached the beginning of the routes
	return (end == 0);		//回到了depot相当于就全部找完了
}

// Split for problems with limited fleet
int Split::splitLF(Individual & indiv)
{
	// Initialize the potential structures，对于车的容量产生限制
	potential[0][0] = 0;
	for (int k = 0; k <= maxVehicles; k++)
		for (int i = 1; i <= params.nbClients; i++)
			potential[k][i] = 1.e30;

	// MAIN ALGORITHM -- Simple Split using Bellman's algorithm in topological order
	// This code has been maintained as it is very simple and can be easily adapted to a variety of constraints, whereas the O(n) Split has a more restricted application scope
	if (params.isDurationConstraint) 
	{
		for (int k = 0; k < maxVehicles; k++)
		{	//k是车辆数，相当于意思是一辆车上至少有一个点
			for (int i = k; i < params.nbClients && potential[k][i] < 1.e29 ; i++)
			{
				double load = 0.;
				double serviceDuration = 0.;
				double distance = 0.;
				double etimeSum = 0.;//分别记录早到时间和迟到时间
				double ltimeSum = 0.;
				//所有的循环都是从depot出发的
				double timeSum = std::max<double>(params.cli[cliSplit[i + 1].c_id].timeWindow[0] - params.timeCost[0][cliSplit[i + 1].c_id], 0);	  //定义一个从depot出发的时间
				for (int j = i + 1; j <= params.nbClients && load <= 1.5 * params.vehicleCapacity ; j++) // Setting a maximum limit on load infeasibility to accelerate the algorithm
				{
					load += cliSplit[j].demand;
					serviceDuration += cliSplit[j].serviceTime;
					if (j == i + 1)
					{
						timeSum += cliSplit[j].d0_x;
						distance += cliSplit[j].d0_x;
					}
					else
					{
						timeSum += cliSplit[j - 1].dnext;
						distance += cliSplit[j - 1].dnext;
					}

					if (timeSum + MY_EPSILON < params.cli[cliSplit[j].c_id].timeWindow[0])  //如果早于时间窗
					{
						etimeSum += params.cli[cliSplit[j].c_id].timeWindow[0] - timeSum;
						timeSum = params.cli[cliSplit[j].c_id].timeWindow[0] + params.cli[cliSplit[j].c_id].serviceDuration;
					}
					else
					{
						if (timeSum + MY_EPSILON < params.cli[cliSplit[j].c_id].timeWindow[1])
						{
							timeSum += params.cli[cliSplit[j].c_id].serviceDuration;
						}
						else
						{
							ltimeSum += timeSum - params.cli[cliSplit[j].c_id].timeWindow[1];
							timeSum += params.cli[cliSplit[j].c_id].serviceDuration;
						}
					}

					double cost = distance + cliSplit[j].dx_0
						+ params.penaltyCapacity * std::max<double>(load - params.vehicleCapacity, 0.)
						+ params.penaltyEarly * etimeSum*0.001 + params.penaltyLate * ltimeSum*0.001;
					//这个地方感觉有点问题
					if (potential[k][i] + cost < potential[k + 1][j])
					{
						potential[k + 1][j] = potential[k][i] + cost;
						pred[k + 1][j] = i;		//注意i和j是两辆车上的
					}
				}
			}
		}
	}
	else // MAIN ALGORITHM -- Without duration constraints in O(n), from "Vidal, T. (2016). Split algorithm in O(n) for the capacitated vehicle routing problem. C&OR"
	{
		Trivial_Deque queue = Trivial_Deque(params.nbClients + 1, 0);
		for (int k = 0; k < maxVehicles; k++)
		{
			// in the Split problem there is always one feasible solution with k routes that reaches the index k in the tour.
			queue.reset(k);

			// The range of potentials < 1.29 is always an interval.
			// The size of the queue will stay >= 1 until we reach the end of this interval.
			for (int i = k + 1; i <= params.nbClients && queue.size() > 0; i++)
			{
				// The front is the best predecessor for i
				potential[k + 1][i] = propagate(queue.get_front(), i, k);
				pred[k + 1][i] = queue.get_front();

				if (i < params.nbClients)
				{
					// If i is not dominated by the last of the pile 
					if (!dominates(queue.get_back(), i, k))
					{
						// then i will be inserted, need to remove whoever he dominates
						while (queue.size() > 0 && dominatesRight(queue.get_back(), i, k))
							queue.pop_back();
						queue.push_back(i);
					}

					// Check iteratively if front is dominated by the next front
					while (queue.size() > 1 && propagate(queue.get_front(), i + 1, k) > propagate(queue.get_next_front(), i + 1, k) - MY_EPSILON)
						queue.pop_front();
				}
			}
		}
	}

	if (potential[maxVehicles][params.nbClients] > 1.e29)
		throw std::string("ERROR : no Split solution has been propagated until the last node");

	// It could be cheaper to use a smaller number of vehicles
	double minCost = potential[maxVehicles][params.nbClients];
	int nbRoutes = maxVehicles;
	for (int k = 1; k < maxVehicles; k++)
		if (potential[k][params.nbClients] < minCost)
			{minCost = potential[k][params.nbClients]; nbRoutes = k;}

	// Filling the chromR structure
	// 这是两个独立的部分
	for (int k = params.nbVehicles-1; k >= nbRoutes ; k--)
		indiv.chromR[k].clear();

	int end = params.nbClients;
	for (int k = 0; k <nbRoutes; k++)
	{
		indiv.chromR[k].clear();
		int begin = pred[k+1][end];
		for (int ii = begin; ii < end; ii++)
			indiv.chromR[k].push_back(indiv.chromT[ii]);	
		end = begin;
	}

	// Return OK in case the Split algorithm reached the beginning of the routes
	return (end == 0);
}

Split::Split(const Params & params): params(params)
{
	// Structures of the linear Split
	// 都把depot考虑进去了
	cliSplit = std::vector <ClientSplit>(params.nbClients + 1);
	sumDistance = std::vector <double>(params.nbClients + 1,0.);
	sumLoad = std::vector <double>(params.nbClients + 1,0.);
	sumService = std::vector <double>(params.nbClients + 1, 0.);
	sumTimeR = std::vector<double>(params.nbClients + 1, 0.);
	sumETimeR = std::vector<double>(params.nbClients + 1, 0.);
	eTimeNode = std::vector<double>(params.nbClients + 1, 0.);
	potential = std::vector < std::vector <double> >(params.nbVehicles + 1, std::vector <double>(params.nbClients + 1,1.e30));
	pred = std::vector < std::vector <int> >(params.nbVehicles + 1, std::vector <int>(params.nbClients + 1,0));
}
