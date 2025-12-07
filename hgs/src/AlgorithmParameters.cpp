#include "AlgorithmParameters.h"
#include <iostream>

extern "C"
struct AlgorithmParameters default_algorithm_parameters() {
	struct AlgorithmParameters ap{};	//声明并初始化一个结构体变量

	ap.nbGranular = 10;	//如果是10的时候,一开始是20,ls的时候需要考虑的临近节点数量
	ap.mu = 25;		//25
	ap.lambda = 40;
	ap.nbElite = 4;
	ap.nbClose = 5;	//临近种群，在计算多样性的时候需要考虑
	ap.LCS_len =5 ;

	ap.GES_timeLimit = 20;	//暂时先定是20秒
	ap.ejMAX = 5;

	ap.varphi = 40;
	ap.dMax = 30;

	ap.maxOuter = 5;
	ap.gstime = 500;

	ap.nbIterPenaltyManagement = 100;	//一开始是100
	ap.targetFeasible = 0.2;		//一开始是0.2
	ap.penaltyDecrease = 0.85;
	ap.penaltyIncrease = 1.1;		//1.2
	ap.routeWeight = 10000;

	ap.seed = 0;
	ap.nbIter = 500;	//也是改成200，一开始是20000
	ap.nbIterTraces = 500;
	ap.timeLimit = 0;
	ap.useSwapStar = 1;

	return ap;
}

void print_algorithm_parameters(const AlgorithmParameters & ap)
{
	std::cout << "=========== Algorithm Parameters =================" << std::endl;
	std::cout << "---- nbGranular              is set to " << ap.nbGranular << std::endl;
	std::cout << "---- mu                      is set to " << ap.mu << std::endl;
	std::cout << "---- lambda                  is set to " << ap.lambda << std::endl;
	std::cout << "---- nbElite                 is set to " << ap.nbElite << std::endl;
	std::cout << "---- nbClose                 is set to " << ap.nbClose << std::endl;
	std::cout << "---- nbIterPenaltyManagement is set to " << ap.nbIterPenaltyManagement << std::endl;
	std::cout << "---- targetFeasible          is set to " << ap.targetFeasible << std::endl;
	std::cout << "---- penaltyDecrease         is set to " << ap.penaltyDecrease << std::endl;
	std::cout << "---- penaltyIncrease         is set to " << ap.penaltyIncrease << std::endl;
	std::cout << "---- seed                    is set to " << ap.seed << std::endl;
	std::cout << "---- nbIter                  is set to " << ap.nbIter << std::endl;
	std::cout << "---- nbIterTraces            is set to " << ap.nbIterTraces << std::endl;
	std::cout << "---- timeLimit               is set to " << ap.timeLimit << std::endl;
	std::cout << "---- useSwapStar             is set to " << ap.useSwapStar << std::endl;
	std::cout << "==================================================" << std::endl;
}
