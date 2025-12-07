#include "Genetic.h"

void Genetic::run()
{	
	/* INITIAL POPULATION */
	population.generatePopulation();			//产生个体
	// population.generateALNSPopulation();		//通过alns的方法读入个体
	int nbIter;
	int clearnb = 1;		//重置更新的次数
	int nbIterNonProd = 1;
	if (params.verbose) std::cout << "----- STARTING GENETIC ALGORITHM" << std::endl;
	for (nbIter = 0 ; nbIterNonProd <= params.ap.nbIter && (params.ap.timeLimit == 0 || (double)(clock()-params.startTime)/(double)CLOCKS_PER_SEC < params.ap.timeLimit) ; nbIter++)
	{	
		/* SELECTION AND CROSSOVER */
		crossoverOX(offspring, population.getBinaryTournament(),population.getBinaryTournament());
		/* LOCAL SEARCH */
		std::vector<int> ejectionOuter;	//在初始化的时候定义一个空的弹出池
		if (offspring.eval.isFeasible && offspring.eval.nbRoutes>params.mincar && params.ran()%2 == 0) 
			ges.run(offspring,15);	//在前期进行车辆缩减
		localSearch.run(offspring, params.penaltyCapacity, params.penaltyEarly, params.penaltyLate,params.routepen,0,ejectionOuter);
		bool isNewBest = population.addIndividual(offspring,true);
		if (!offspring.eval.isFeasible && params.ran()%2 == 0) // Repair half of the solutions in case of infeasibility
		{
			//以百分之五十的概率修复这个解(这个惩罚最后应该是多少)
			localSearch.run(offspring, params.penaltyCapacity * 10., params.penaltyEarly * 1., params.penaltyLate * 10.,params.routepen,0,ejectionOuter); //惩罚都要增加
			if (offspring.eval.isFeasible) 
			{
				if (offspring.eval.nbRoutes>params.mincar)	//也是一定的概率
					ges.run(offspring,15);	//也需要优化一下
				isNewBest = (population.addIndividual(offspring,false) || isNewBest);
			}
		}
		if (offspring.eval.isFeasible)
		{
			for (int i=0;i<offspring.eval.nbRoutes;i++)
			{
				//一个是时间可行，一个是重量可行,那么这一条路是可行
				params.newChromGather.push_back(offspring.chromR[i]); 
				params.newCostGather.push_back(offspring.chromRCost[i]);
			}
			lastbest=offspring.eval.nbRoutes;
		}
		destoryAndRepair.run(offspring);
		localSearch.run(offspring, params.penaltyCapacity*10, params.penaltyEarly, params.penaltyLate*10,params.routepen,0,ejectionOuter);
		if (offspring.eval.isFeasible)
		{
			for (int i=0;i<offspring.eval.nbRoutes;i++)
			{
				//一个是时间可行，一个是重量可行,那么这一条路是可行
				params.newChromGather.push_back(offspring.chromR[i]); 
				params.newCostGather.push_back(offspring.chromRCost[i]);
			}
			lastbest=offspring.eval.nbRoutes;
		}
		//进行最优解计算
		Individual gatherIndiv(params);
		bool getnew=gather.run(population.getBestFound()->eval.nbRoutes,gatherIndiv,lastbest);	//集合模型
		if (getnew && gatherIndiv.eval.penalizedCost+MY_EPSILON< population.getBestFound()->eval.penalizedCost)
		{
			//如果说集合模型的解更好，就更新一下
			population.addIndividual(gatherIndiv,true);
			isNewBest = true;	//更新了就算是新的最优解
		}
		else if(!gatherIndiv.eval.isFeasible)
		{
			localSearch.run(gatherIndiv, params.penaltyCapacity * 10., params.penaltyEarly * 1., params.penaltyLate * 10.,params.routepen,0,ejectionOuter); //惩罚都要增加
			if (gatherIndiv.eval.isFeasible) 
			{
				isNewBest = (population.addIndividual(gatherIndiv,false) || isNewBest);
			}
		}
		if (gatherIndiv.eval.isFeasible)
		{
			for (int i=0;i<gatherIndiv.eval.nbRoutes;i++)
			{
				params.newChromGather.push_back(gatherIndiv.chromR[i]); 
				params.newCostGather.push_back(gatherIndiv.chromRCost[i]);
			}
			lastbest=gatherIndiv.eval.nbRoutes;
		}
		destoryAndRepair.run(gatherIndiv);
		localSearch.run(gatherIndiv, params.penaltyCapacity*10, params.penaltyEarly, params.penaltyLate*10,params.routepen,0,ejectionOuter);
		if (gatherIndiv.eval.isFeasible)
		{
			for (int i=0;i<gatherIndiv.eval.nbRoutes;i++)
			{
				//一个是时间可行，一个是重量可行,那么这一条路是可行
				params.newChromGather.push_back(gatherIndiv.chromR[i]); 
				params.newCostGather.push_back(gatherIndiv.chromRCost[i]);
			}
			lastbest=gatherIndiv.eval.nbRoutes;
		}

		/* LCS,暂时先不考虑变异的情况 */
		population.findEliteLCS(population.getSubpop(0));	//前面的0表示可行种群，1表示不可行种群
		//不可行种群的暂时不要
		// population.findEliteLCS(population.getSubpop(1)); //后面的0表示LCSe，1表示LCSi，表示不好
		/* TRACKING THE NUMBER OF ITERATIONS SINCE LAST SOLUTION IMPROVEMENT */
		//如果当前的最优解更新了，他就会跟着更新
		if (isNewBest) {nbIterNonProd = 1;clearnb++;}
		else nbIterNonProd ++ ;

		/* DIVERSIFICATION, PENALTY MANAGEMENT AND TRACES */
		if (nbIter % params.ap.nbIterPenaltyManagement == 0) population.managePenalties();	   //判断惩罚是否到了更新的周期
		// if (nbIter % params.ap.nbIterTraces == 0) 	//每500代输出一个结果
		if (nbIter % 10 == 0) 	//每500代输出一个结果
		{
			population.printState(nbIter, nbIterNonProd);
			if (population.getBestFound() != NULL)
			{
				population.exportCVRPLibFormat(*population.getBestFound(), "../process/C1_2_11.TXT");
			}
			if (population.getBestCarIndiv() != NULL)
			{
				population.exportCVRPLibFormat(*population.getBestCarIndiv(), "../process/C1_2_22.TXT");
			}
		}
		// /* FOR TESTS INVOLVING SUCCESSIVE RUNS UNTIL A TIME LIMIT: WE RESET THE ALGORITHM/POPULATION EACH TIME maxIterNonProd IS ATTAINED*/
		// if (params.ap.timeLimit != 0 && nbIterNonProd == params.ap.nbIter)
		// if (nbIter % 200==0 && nbIter>0)
		// {
		// 	population.restart();
		// 	nbIterNonProd = 1;
		// }
	}
	if (params.verbose) std::cout << "----- GENETIC ALGORITHM FINISHED AFTER " << nbIter << " ITERATIONS. TIME SPENT: " << (double)(clock() - params.startTime) / (double)CLOCKS_PER_SEC << std::endl;
}


int findback(const std::vector<int>& vec, const std::vector<bool>& freq ,int num)
{
	if (freq[num]==true)
		return findback(vec,freq,vec[num]);
	else
		return num;
}

//注意这里保存的是第一个子代，每次只生成一个
void Genetic::crossoverOX(Individual & result, const Individual & parent1, const Individual & parent2)
{
	// Frequency table to track the customers which have been already inserted，标记每个客户是否被插入到了子代当中
	std::vector <bool> freqClient = std::vector <bool> (params.nbClients + 1, false);
	//这个保存的是位置
	//注意有风险！！！crossover的这个后代每次都要更新
	result.chromT=std::vector<int>(params.nbClients,0);
	// Picking the beginning and end of the crossover zone
	std::uniform_int_distribution<> distr(0, params.nbClients-1);
	int start = distr(params.ran);
	int end = distr(params.ran);
	// Avoid that start and end coincide by accident
	while (end == start) end = distr(params.ran);
	// Copy from start to end
	int j = start;
	//把1的抽一块，放到2中
	int isLCS=0;
	std::vector<std::vector<int>> lcs_inner;
	for (int pos=0;pos<population.LCSe.size();pos++)
	{
		std::vector<int> judge;
		judge=population.lcs_two(population.LCSe[pos], parent2.chromT);	//看看2里面有没有
		if (!judge.empty())
			lcs_inner.push_back(judge);	//如果有的话，就把这个LCS放到里面
	}
	//直接对比一整段就可以了
	/*
	while循环的判断逻辑
	1.首先是如果1对应的元素j在LCS模块里面，就要删除掉，放上对应的2的元素
		1.1如果2的元素之前已经用过了，就要放上loc中2对应位置保存的元素
	2.如果1对应的元素j不在LCS模块里面，就把1对应的元素放进去
	*/
	std::vector<int> loc_where(params.nbClients+1,0);	//用来记录反向位置的下表
	lcs_inner.clear();	//清空一下,如果需要恢复初始的话
	while (j % params.nbClients != (end + 1) % params.nbClients)
	{
		int sym_inner=0;
		if (!lcs_inner.empty())	//不空的话，就是2里面有LCS模块的话，才要去原位置删除
		{
			for (int pos=0;pos<lcs_inner.size();pos++)
			{
				if (std::find(lcs_inner[pos].begin(), lcs_inner[pos].end(), parent1.chromT[j % params.nbClients]) != lcs_inner[pos].end())
				{
					if (freqClient[parent2.chromT[j % params.nbClients]] == false)
					{
						result.chromT[j % params.nbClients] = parent2.chromT[j % params.nbClients];
						freqClient[result.chromT[j % params.nbClients]] = true;
						sym_inner=1;	//表示有重复
						break;
					}
					else
					{
						result.chromT[j % params.nbClients] = findback(loc_where,freqClient,loc_where[parent2.chromT[j % params.nbClients]]);
						freqClient[result.chromT[j % params.nbClients]] = true;		//放下的元素也要是true
						sym_inner=1;	//表示有重复
						break;
					}	
				}
			}
		}
		if (sym_inner==0)
		{
			if (freqClient[parent1.chromT[j % params.nbClients]] == false)
			{
				result.chromT[j % params.nbClients] = parent1.chromT[j % params.nbClients];
				freqClient[result.chromT[j % params.nbClients]] = true;		//注意这里面保存的是位置
				loc_where[result.chromT[j % params.nbClients]]=parent2.chromT[j % params.nbClients];
			}
			else
			{
				if (freqClient[parent2.chromT[j % params.nbClients]] == true)
				{
					result.chromT[j % params.nbClients] = findback(loc_where,freqClient,loc_where[parent2.chromT[j % params.nbClients]]);
					freqClient[result.chromT[j % params.nbClients]] = true;		//注意这里面保存的是位置
				}
				else
				{
					result.chromT[j % params.nbClients] = parent2.chromT[j % params.nbClients];	//这个位置不用记录
					freqClient[result.chromT[j % params.nbClients]] = true;		//注意这里面保存的是位置
				}	
			}
		}
		j++;
	}
	// Fill the remaining elements in the order given by the second parent
	for (int i = 1; i <= params.nbClients; i++)
	{
		int temp = parent2.chromT[(end + i) % params.nbClients];
		if (freqClient[temp] == false)
		{
			result.chromT[j % params.nbClients] = temp;
			j++;
		}
	}
	// Complete the individual with the Split algorithm
	//这个split感觉需要改一下
	split.generalSplit(result, parent1.eval.nbRoutes);
}

//选择的过程
void Genetic::rouletteWheelSelection(Individual& selected) 
{
    std::vector<double> fitness;
    for (const auto& indiv : population.getSubpop(0)) {
        double fit = 1.0 / (indiv->eval.penalizedCost + MY_EPSILON); // 防止除零
        fitness.push_back(fit);
    }
    double sumFitness = std::accumulate(fitness.begin(), fitness.end(), 0.0);
    std::uniform_real_distribution<> dist(0.0, sumFitness);
    double randValue = dist(params.ran);

    double acc = 0.0;
    for (int i = 0; i < (int)fitness.size(); i++) 
	{
        acc += fitness[i];
        if (randValue <= acc) 
		{
            selected = *population.getSubpop(0)[i];
            return;
        }
    }
    // 保险起见
    selected = *population.getSubpop(0).back();
}

//这个需要分多少类，暂时先自己初始化,population里面暂时不需要
Genetic::Genetic(Params & params) : 
	params(params), 
	initial(params),
	split(params),
	localSearch(params),
	destoryAndRepair(params),
	ges(params,localSearch),
	population(params,this->split,this->initial,this->localSearch,this->destoryAndRepair,this->ges),
	offspring(params),
	gather(params,this->destoryAndRepair),
	globalSearch(params),
	gatherIndiv(params),
	gsIndiv(params)
	{
		offspring.IndividualThree(params,initial,2);
	}