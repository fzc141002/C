#include "Population.h"

void Population::generateALNSPopulation()
{
	if (params.verbose) std::cout << "----- BUILDING INITIAL POPULATION" << std::endl;	
	int i=0;
	Individual randomIndiv(params);	//如果用聚类可能会导致LCS太长了？
	ALNSInital bestfirst(params,randomIndiv);
	bestfirst.run();
	do
	{
		std::cout<<"第"<<i<<"个个体"<<std::endl;
		//不断通过扰动产生新的个体加进去
		destoryAndRepair.run(randomIndiv);
		addIndividual(randomIndiv, true);
		std::vector<int> ejectionOuter;	//在初始化的时候定义一个空的弹出池
		if (!randomIndiv.eval.isFeasible && params.ran() % 2 == 0)  // Repair half of the solutions in case of infeasibility
		{
			//这个惩罚最后应该是多少
			localSearch.run(randomIndiv, params.penaltyCapacity * 10., params.penaltyEarly * 1., params.penaltyLate * 10.,params.routepen,0,ejectionOuter);
			std::cout<<"修复之后可不可行:"<<randomIndiv.eval.isFeasible<<std::endl;
			// if (randomIndiv.eval.isFeasible && randomIndiv.eval.nbRoutes>params.mincar) 
			if (randomIndiv.eval.isFeasible)
			{
				ges.run(randomIndiv,15);	//进行车辆缩减一下
				addIndividual(randomIndiv, false);
			}
				
		}
		if (randomIndiv.eval.isFeasible)		//看看引入什么样的解效果更好
		{
			for (int i=0;i<randomIndiv.eval.nbRoutes;i++)
			{
				params.newChromGather.push_back(randomIndiv.chromR[i]);
				params.newCostGather.push_back(randomIndiv.chromRCost[i]);
				if (params.newChromGather.size()>params.boolNum)
				{
					params.newChromGather.erase(params.newChromGather.begin());
					params.newCostGather.erase(params.newCostGather.begin());
				}
			}
		}
		i++;
	}
	while (i< 4*params.ap.mu && (i == 0 || params.ap.timeLimit == 0 || (double)(clock() - params.startTime) / (double)CLOCKS_PER_SEC < params.ap.timeLimit));
}


//注意后面改的时候要查看所有的和早晚惩罚有关的地方
void Population::generatePopulation()
{
	//先生成空间和时间聚类之后的个体
	// initial.run();	//不用聚类，不用看了
	// 4*params.ap.mu
	if (params.verbose) std::cout << "----- BUILDING INITIAL POPULATION" << std::endl;
	for (int i = 0; i < 4*params.ap.mu && (i == 0 || params.ap.timeLimit == 0 || (double)(clock() - params.startTime) / (double)CLOCKS_PER_SEC < params.ap.timeLimit) ; i++)
	{
		std::cout<<"第"<<i<<"个个体"<<std::endl;
		Individual randomIndiv(params);
		//在时空聚类了之后，就会采用分组聚类的方式
		// if (i<params.ap.mu)
		// 	randomIndiv.IndividualThree(params,initial,0);
		// else if (i>=params.ap.mu && i<3*params.ap.mu)
		// 	randomIndiv.IndividualThree(params,initial,1);
		// else 
		// 	randomIndiv.IndividualThree(params,initial,2);	//随机初始化
		split.generalSplit(randomIndiv, params.nbVehicles);
		//第一步也要局部搜索
		std::vector<int> ejectionOuter;	//在初始化的时候定义一个空的弹出池
		std::cout<<"可不可行:"<<randomIndiv.eval.isFeasible<<std::endl;
		addIndividual(randomIndiv, true);
		if (!randomIndiv.eval.isFeasible && params.ran() % 2 == 0)  // Repair half of the solutions in case of infeasibility
		{
			//这个惩罚最后应该是多少
			localSearch.run(randomIndiv, params.penaltyCapacity * 10., params.penaltyEarly * 1., params.penaltyLate * 10.,params.routepen,0,ejectionOuter);
			std::cout<<"修复之后可不可行:"<<randomIndiv.eval.isFeasible<<std::endl;
			if (randomIndiv.eval.isFeasible)
				addIndividual(randomIndiv, false);
		}
		if (randomIndiv.eval.isFeasible)		//看看引入什么样的解效果更好
		{
			for (int i=0;i<randomIndiv.eval.nbRoutes;i++)
			{
				params.newChromGather.push_back(randomIndiv.chromR[i]);
				params.newCostGather.push_back(randomIndiv.chromRCost[i]);
				if (params.newChromGather.size()>params.boolNum)
				{
					params.newChromGather.erase(params.newChromGather.begin());
					params.newCostGather.erase(params.newCostGather.begin());
				}
			}
		}
	}
}

//将新的个体加入到种群中
bool Population::addIndividual(const Individual & indiv, bool updateFeasible)
{	//最后一个参数是是否更新可行性记录，简单来说就是当前种群达到规模了就要更新
	if (updateFeasible)	//这个后续可以用来调整惩罚系数
	{
		//用来调整惩罚系数,可以理解为是在某一个方面可行
		listFeasibilityLoad.push_back(indiv.eval.capacityExcess < MY_EPSILON);
		listFeasibilityDuration.push_back(indiv.eval.timeEarlyExcess + indiv.eval.timeLateExcess < MY_EPSILON);
		listFeasibilityLoad.pop_front();	//移除头部的第一个元素
		listFeasibilityDuration.pop_front();
	}

	// Find the adequate subpopulation in relation to the individual feasibility
	// 根据个体的适应性情况，加入
	SubPopulation & subpop = (indiv.eval.isFeasible) ? feasibleSubpop : infeasibleSubpop;		//对于子种群进行判断

	// Create a copy of the individual and updade the proximity structures calculating inter-individual distances
	// 创建一个副本
	Individual * myIndividual = new Individual(indiv);
	for (Individual * myIndividual2 : subpop)	//计算新个体与子种群中每个个体的距离，会遍历subpop中所有的个体
	{
		double myDistance = brokenPairsDistance(*myIndividual,*myIndividual2);
		myIndividual2->indivsPerProximity.insert({ myDistance, myIndividual });
		myIndividual->indivsPerProximity.insert({ myDistance, myIndividual2 });	  //这个也是对称的
	}

	// Identify the correct location in the subpopulation and insert the individual
	// 确定插入的位置，从小往大排序的，根据penalizedCost来进行排序，在插入的时候就排好序了
	int place = (int)subpop.size();
	while (place > 0 && subpop[place - 1]->eval.penalizedCost > indiv.eval.penalizedCost - MY_EPSILON) place--;
	subpop.emplace(subpop.begin() + place, myIndividual);	//重新放，确定对应放的位置

	// Trigger a survivor selection if the maximimum subpopulation size is exceeded，超出限制之后就要开始筛选了
	if ((int)subpop.size() > params.ap.mu + params.ap.lambda)
		while ((int)subpop.size() > params.ap.mu)
			removeWorstBiasedFitness(subpop);	//移除最差的

	// Track best solution
	//eval代表的是解的状态，只有是在可行解里面才会判断
	if (indiv.eval.isFeasible && indiv.eval.penalizedCost < bestSolutionRestart.eval.penalizedCost - MY_EPSILON)
	{
		bestSolutionRestart = indiv; // Copy
		if (indiv.eval.penalizedCost < bestSolutionOverall.eval.penalizedCost - MY_EPSILON)
		{
			bestSolutionOverall = indiv;
			searchProgress.push_back({ clock() - params.startTime , bestSolutionOverall.eval.penalizedCost });
		}
		return true;	//这个true的位置应该放在里面
	}
	else
		return false;
}

//公共子串只要一个个遍历就好
void Population::findEliteLCS(const SubPopulation& pop)	//0是好的
{
	int eliteNum = std::min(params.ap.nbElite, (int)pop.size());
	if((int)pop.size() < params.ap.nbElite) return;			//我感觉不应该直接就退出了？
	std::vector<int> res_good;
	std::vector<int> res_bad;
	res_good = pop[0]->chromT;		//因为第一个已经放出来了
	for(int i=1; i<eliteNum; ++i) 
	{
		res_good = lcs_two(res_good, pop[i]->chromT);	//可能会越找越短
		if(res_good.empty()) break;
	}
	if ((int)pop.size() > params.ap.nbElite) 
	{
		res_bad = pop[pop.size()-1]->chromT;	//从后往前找
		for(int i=pop.size()-2; i>=pop.size()-eliteNum; --i) 
		{
			res_bad = lcs_two(res_bad, pop[i]->chromT);	//可能会越找越短
			if(res_bad.empty()) break;
		}
	}
		
	if (res_good==res_bad) return;	//如果说两个子串相同就返回
	if(!res_good.empty()) 
	{
		if (LCSe.size()>=params.ap.LCS_len)
			LCSe.erase(LCSe.begin());		//如果超过了长度就删除第一个
		if (std::find(LCSeHis.begin(), LCSeHis.end(), res_good) == LCSeHis.end()) 
		{
			LCSe.push_back(res_good);		//如果没有找到就加入
			LCSeHis.push_back(res_good);
		}
	}
	if(!res_bad.empty()) 
	{
		if (LCSi.size()>=params.ap.LCS_len)
			LCSi.erase(LCSi.begin());		//如果超过了长度就删除第一个
		if (std::find(LCSiHis.begin(), LCSiHis.end(), res_bad) == LCSiHis.end()) 
		{
			LCSi.push_back(res_bad);		//如果没有找到就加入
			LCSiHis.push_back(res_bad);
		}
	}
}

//更新偏置适应性，把排名和分数都更新了;适应度算的是整个种群的适应度
void Population::updateBiasedFitnesses(SubPopulation & pop)
{
	// Ranking the individuals based on their diversity contribution (decreasing order of distance)
	std::vector <std::pair <double, int> > ranking;
	for (int i = 0 ; i < (int)pop.size(); i++) 
		ranking.push_back({-averageBrokenPairsDistanceClosest(*pop[i],params.ap.nbClose),i});		//首先是多样性
	std::sort(ranking.begin(), ranking.end());

	// Updating the biased fitness values
	if (pop.size() == 1) 
		pop[0]->biasedFitness = 0;
	else
	{
		for (int i = 0; i < (int)pop.size(); i++)
		{
			double divRank = (double)i / (double)(pop.size() - 1); // Ranking from 0 to 1
			double fitRank = (double)ranking[i].second / (double)(pop.size() - 1);  //关系到其中的两个排名
			if ((int)pop.size() <= params.ap.nbElite) // Elite individuals cannot be smaller than population size
				pop[ranking[i].second]->biasedFitness = fitRank;
			else 
				pop[ranking[i].second]->biasedFitness = fitRank + (1.0 - (double)params.ap.nbElite / (double)pop.size()) * divRank;
		}
	}
}


//从子种群中移除具有最差适应性的个体
void Population::removeWorstBiasedFitness(SubPopulation & pop)
{
	updateBiasedFitnesses(pop);
	if (pop.size() <= 1) throw std::string("Eliminating the best individual: this should not occur in HGS");

	Individual * worstIndividual = NULL;
	int worstIndividualPosition = -1;
	bool isWorstIndividualClone = false;
	double worstIndividualBiasedFitness = -1.e30;
	//偏置适应度最高说明排名最差
	for (int i = 1; i < (int)pop.size(); i++)
	{
		bool isClone = (averageBrokenPairsDistanceClosest(*pop[i],1) < MY_EPSILON); // A distance equal to 0 indicates that a clone exists
		if ((isClone && !isWorstIndividualClone) || (isClone == isWorstIndividualClone && pop[i]->biasedFitness > worstIndividualBiasedFitness))
		{
			worstIndividualBiasedFitness = pop[i]->biasedFitness;
			isWorstIndividualClone = isClone;
			worstIndividualPosition = i;
			worstIndividual = pop[i];
		}
	}

	// Removing the individual from the population and freeing memory
	pop.erase(pop.begin() + worstIndividualPosition); 

	// Cleaning its distances from the other individuals in the population
	for (Individual * indiv2 : pop)
	{
		auto it = indiv2->indivsPerProximity.begin();
		while (it->second != worstIndividual) ++it;
		indiv2->indivsPerProximity.erase(it);
	}

	// Freeing memory
	delete worstIndividual; 
}

//彻底新建种群
void Population::restart()
{
	if (params.verbose) std::cout << "----- RESET: CREATING A NEW POPULATION -----" << std::endl;
	for (Individual * indiv : feasibleSubpop) delete indiv ;
	for (Individual * indiv : infeasibleSubpop) delete indiv;
	feasibleSubpop.clear();
	infeasibleSubpop.clear();
	bestSolutionRestart = Individual(params);
	generatePopulation();
	addIndividual(bestSolutionOverall,true);	//指把当前的最优解加进去
}


//对于惩罚的更新
void Population::managePenalties()
{
	// Setting some bounds [0.1,100000] to the penalty values for safety
	double fractionFeasibleLoad = (double)std::count(listFeasibilityLoad.begin(), listFeasibilityLoad.end(), true) / (double)listFeasibilityLoad.size();
	if (fractionFeasibleLoad < params.ap.targetFeasible - 0.05 && params.penaltyCapacity < 100000.)
		params.penaltyCapacity = std::min<double>(params.penaltyCapacity * params.ap.penaltyIncrease, 100000.);
	else if (fractionFeasibleLoad > params.ap.targetFeasible + 0.05 && params.penaltyCapacity > 0.1)
		params.penaltyCapacity = std::max<double>(params.penaltyCapacity * params.ap.penaltyDecrease, 0.1);

	// Setting some bounds [0.1,100000] to the penalty values for safety
	double fractionFeasibleDuration = (double)std::count(listFeasibilityDuration.begin(), listFeasibilityDuration.end(), true) / (double)listFeasibilityDuration.size();
	if (fractionFeasibleDuration < params.ap.targetFeasible - 0.05 && (params.penaltyEarly + params.penaltyLate) < 100000.)
	{
		//这边后续考虑要不要分情况讨论
		params.penaltyEarly = std::min<double>(params.penaltyEarly * params.ap.penaltyIncrease, 100000.);
		params.penaltyLate = std::min<double>(params.penaltyLate * params.ap.penaltyIncrease, 100000.);
	}
	else if (fractionFeasibleDuration > params.ap.targetFeasible + 0.05 && (params.penaltyEarly + params.penaltyLate) > 0.1)
	{
		params.penaltyEarly = std::max<double>(params.penaltyEarly * params.ap.penaltyDecrease, 0.1);
		params.penaltyLate = std::max<double>(params.penaltyLate * params.ap.penaltyDecrease, 0.1);
	}

	//如果说对距离加了一些系数进去的话，那可行解也会需要更新

	// 注意对于不可行解来说需要更新
	for (int i = 0; i < (int)infeasibleSubpop.size(); i++)
		infeasibleSubpop[i]->eval.penalizedCost = infeasibleSubpop[i]->eval.distance + infeasibleSubpop[i]->eval.nbRoutes*params.ap.routeWeight
		+ params.penaltyCapacity * infeasibleSubpop[i]->eval.capacityExcess
		+ params.penaltyEarly * infeasibleSubpop[i]->eval.timeEarlyExcess + params.penaltyLate * infeasibleSubpop[i]->eval.timeLateExcess;

	// If needed, reorder the individuals in the infeasible subpopulation since the penalty values have changed (simple bubble sort for the sake of simplicity)
	for (int i = 0; i < (int)infeasibleSubpop.size(); i++)
	{
		for (int j = 0; j < (int)infeasibleSubpop.size() - i - 1; j++)
		{
			if (infeasibleSubpop[j]->eval.penalizedCost > infeasibleSubpop[j + 1]->eval.penalizedCost + MY_EPSILON)
			{
				Individual * indiv = infeasibleSubpop[j];
				infeasibleSubpop[j] = infeasibleSubpop[j + 1];
				infeasibleSubpop[j + 1] = indiv;
			}
		}
	}
}

//通过锦标赛的方式选择两个个体
const Individual & Population::getBinaryTournament ()
{
	// Picking two individuals with uniform distribution over the union of the feasible and infeasible subpopulations
	// 构建一个均匀整数分布的对象
	std::uniform_int_distribution<> distr(0, feasibleSubpop.size() + infeasibleSubpop.size() - 1);
	int place1 = distr(params.ran);
	int place2 = distr(params.ran);
	Individual * indiv1 = (place1 >= (int)feasibleSubpop.size()) ? infeasibleSubpop[place1 - feasibleSubpop.size()] : feasibleSubpop[place1];
	Individual * indiv2 = (place2 >= (int)feasibleSubpop.size()) ? infeasibleSubpop[place2 - feasibleSubpop.size()] : feasibleSubpop[place2];
	
	// Keeping the best of the two in terms of biased fitness
	updateBiasedFitnesses(feasibleSubpop);
	updateBiasedFitnesses(infeasibleSubpop);
	if (indiv1->biasedFitness < indiv2->biasedFitness) return *indiv1 ;
	else return *indiv2 ;		
}

const SubPopulation & Population::getSubpop(int sym)
{
	return (sym == 0) ? feasibleSubpop : infeasibleSubpop;	//sym==0表示可行解，首先是从种群中获得对应的解
}

const Individual * Population::getBestFeasible ()
{
	if (!feasibleSubpop.empty()) return feasibleSubpop[0] ;
	else return NULL ;
}

const Individual * Population::getBestInfeasible ()
{
	if (!infeasibleSubpop.empty()) return infeasibleSubpop[0] ;
	else return NULL ;
}

const Individual * Population::getBestCarIndiv()
{
	int minCar=100;
	Individual minIndiv(params);
	Individual* ptr= &minIndiv;
	if (!feasibleSubpop.empty())
	{
		for (Individual* & i:feasibleSubpop)
		{
			if (i->eval.nbRoutes<minCar)
			{
				minCar=i->eval.nbRoutes;
				ptr=i;
			}
		}
		return ptr;
	}
	else return NULL;
}
const Individual * Population::getBestFound()
{
	if (bestSolutionOverall.eval.penalizedCost < 1.e29) return &bestSolutionOverall;
	else return NULL;
}



void Population::printState(int nbIter, int nbIterNoImprovement)
{
	if (params.verbose)
	{
		std::printf("It %6d %6d | T(s) %.2f", nbIter, nbIterNoImprovement, (double)(clock()-params.startTime)/(double)CLOCKS_PER_SEC);

		if (getBestFeasible() != NULL) std::printf(" | Feas %zu %.2f %.2f", feasibleSubpop.size(), getBestFeasible()->eval.penalizedCost, getAverageCost(feasibleSubpop));
		else std::printf(" | NO-FEASIBLE");

		if (getBestInfeasible() != NULL) std::printf(" | Inf %zu %.2f %.2f", infeasibleSubpop.size(), getBestInfeasible()->eval.penalizedCost, getAverageCost(infeasibleSubpop));
		else std::printf(" | NO-INFEASIBLE");

		std::printf(" | Div %.2f %.2f", getDiversity(feasibleSubpop), getDiversity(infeasibleSubpop));
		std::printf(" | Feas %.2f %.2f", (double)std::count(listFeasibilityLoad.begin(), listFeasibilityLoad.end(), true) / (double)listFeasibilityLoad.size(), (double)std::count(listFeasibilityDuration.begin(), listFeasibilityDuration.end(), true) / (double)listFeasibilityDuration.size());
		std::printf(" | Pen Capacity%.2f Early%.2f Late%.2f", params.penaltyCapacity, params.penaltyEarly, params.penaltyLate);
		if (getBestFeasible() != NULL) std::printf(" | CarNum %d",getBestFeasible()->eval.nbRoutes);
		else std::printf(" | NO-FEASIBLE");
		if (getBestCarIndiv() != NULL) std::printf(" | BestCarNum %d Feas %.2f",getBestCarIndiv()->eval.nbRoutes,getBestCarIndiv()->eval.penalizedCost);
		else std::printf(" | NO-FEASIBLE");
		std::cout << std::endl;
	}
}

//通过点对顺序不同，来计算区分度，这个到时候可以考虑换一下
double Population::brokenPairsDistance(const Individual & indiv1, const Individual & indiv2)
{
	int differences = 0;
	for (int j = 1; j <= params.nbClients; j++)
	{
		if (indiv1.successors[j] != indiv2.successors[j] && indiv1.successors[j] != indiv2.predecessors[j]) differences++;
		if (indiv1.predecessors[j] == 0 && indiv2.predecessors[j] != 0 && indiv2.successors[j] != 0) differences++;
	}
	return (double)differences / (double)params.nbClients;
}

double Population::averageBrokenPairsDistanceClosest(const Individual & indiv, int nbClosest)
{
	//考虑n个最近的邻居
	double result = 0.;
	int maxSize = std::min<int>(nbClosest, indiv.indivsPerProximity.size());
	//auto可以自动推导数据类型
	auto it = indiv.indivsPerProximity.begin();
	for (int i = 0; i < maxSize; i++)
	{
		result += it->first;		//表示pair中的第一个元素，看当前个体的平均适应度情况
		++it;
	}
	return result / (double)maxSize;
}

//获得一个种群的适应度情况
double Population::getDiversity(const SubPopulation & pop)
{
	double average = 0.;
	int size = std::min<int>(params.ap.mu, pop.size()); // Only monitoring the "mu" better solutions to avoid too much noise in the measurements
	for (int i = 0; i < size; i++) average += averageBrokenPairsDistanceClosest(*pop[i],size);
	if (size > 0) return average / (double)size;
	else return -1.0;
}

double Population::getAverageCost(const SubPopulation & pop)
{
	double average = 0.;
	int size = std::min<int>(params.ap.mu, pop.size()); // Only monitoring the "mu" better solutions to avoid too much noise in the measurements
	for (int i = 0; i < size; i++) average += pop[i]->eval.penalizedCost;
	if (size > 0) return average / (double)size;
	else return -1.0;
}

//导出有关的搜索状态
void Population::exportSearchProgress(std::string fileName, std::string instanceName)
{
	std::ofstream myfile(fileName);
	for (std::pair<clock_t, double> state : searchProgress)
		myfile << instanceName << ";" << params.ap.seed << ";" << state.second << ";" << (double)state.first / (double)CLOCKS_PER_SEC << std::endl;
}

//导出最后的结果
void Population::exportCVRPLibFormat(const Individual & indiv, std::string fileName)
{
	std::ofstream myfile(fileName);
	if (myfile.is_open())
	{
		myfile << "======================Params======================"<< std::endl;
		myfile << "Seed: " << params.ap.seed << std::endl;
		myfile << "Penalty Capacity Initial: " << params.cpencap << "\tAfter the update: "<<params.penaltyCapacity<<std::endl;
		myfile << "Penalty Early Initial: " << params.cpeneary <<"\tAfter the update: "<<params.penaltyEarly<< std::endl;
		myfile << "Penalty Late Initial: " << params.cpenlate << "\tAfter the update: "<<params.penaltyLate<<std::endl;
		myfile << "Route Weight: " << params.ap.routeWeight << "\tPenalty route: "<<params.routepen<<std::endl;
		myfile << "======================Result======================"<< std::endl;		
		double all_dis=0.0;		//总的距离和行驶时间
		double all_time=0.0;
		for (int k = 0; k < (int)indiv.chromR.size(); k++)
		{
			if (!indiv.chromR[k].empty())
			{
				double load = 0.;
				vvd timeR, etimeR;
				int isStartFeasible = 1;
				double dis_sum=0;
				GetStartTime(params, indiv.chromR[k], timeR, etimeR, isStartFeasible); // 获取时间信息
				all_time+=timeR[timeR.size()-1]; // 累加总时间
				myfile << "Route #" << k + 1 << ": 0"; // Route IDs start at 1 in the file format
				dis_sum+= params.timeCost[0][indiv.chromR[k][0]]; // 从depot到第一个客户点的距离
				for (int i=0;i<(int)indiv.chromR[k].size()-1;i++)
				{
					dis_sum+=params.timeCost[indiv.chromR[k][i]][indiv.chromR[k][i + 1]];
				}
				dis_sum+= params.timeCost[indiv.chromR[k][indiv.chromR[k].size() - 1]][0]; // 最后一个客户点回到depot的距离
				all_dis+=dis_sum; // 累加总距离
				for (int i : indiv.chromR[k]) 
				{
					myfile << "->" << i;
					load += params.cli[i].demand; // 累加每个客户点的需求
				}
				myfile <<"->0"<<std::endl;
				for (int i=1;i<(int)etimeR.size()-1;i++)
				{
					if (etimeR[i] > 0) 
					{
						myfile << "for Client" << indiv.chromR[k][i-1] << "wait:" << etimeR[i] << std::endl; // 输出早到时间
					}
				}
				myfile << "Distance:" << " " << dis_sum << std::endl; // 输出路径的总距离
				myfile << "Total Demand:"<< " " << load << std::endl; // 输出每条路径的总负载
				myfile << "Total Time" << " "<< timeR[timeR.size()-1] << std::endl; // 输出每条路径的总时间
				myfile << std::endl;
			}
		}
		myfile << "======================Total======================"<< std::endl;
		myfile << "ALL Calculate time " << (double)(clock()-params.startTime)/(double)CLOCKS_PER_SEC << std::endl;
		myfile << "timeSum " << all_time << std::endl;
		myfile << "distanceSum " << all_dis << std::endl;
		myfile << "total_Cost " << indiv.eval.penalizedCost << std::endl; // 输出总的惩罚成本
	}
	else std::cout << "----- IMPOSSIBLE TO OPEN: " << fileName << std::endl;
}

//在初始化
Population::Population(Params & params, Split & split,Initial& initial,LocalSearch & localSearch,DestoryAndRepair& destoryAndRepair,GES& ges) : params(params), split(split),initial(initial), localSearch(localSearch),destoryAndRepair(destoryAndRepair),ges(ges), bestSolutionRestart(params), bestSolutionOverall(params)
{
	listFeasibilityLoad = std::list<bool>(params.ap.nbIterPenaltyManagement, true);
	listFeasibilityDuration = std::list<bool>(params.ap.nbIterPenaltyManagement, true);
}

Population::~Population()
{
	for (int i = 0; i < (int)feasibleSubpop.size(); i++) delete feasibleSubpop[i];
	for (int i = 0; i < (int)infeasibleSubpop.size(); i++) delete infeasibleSubpop[i];
}