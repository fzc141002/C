#ifndef INDIVIDUAL_H
#define INDIVIDUAL_H

#include "Params.h"
#include "StartTime.h"
#include "Initial.h"

//解的状态结构体,保存的是所有的解
struct EvalIndiv
{
	double penalizedCost = 0.;		// Penalized cost of the solution
	int nbRoutes = 0;				// Number of routes
	double distance = 0.;			// Total distance
	double capacityExcess = 0.;		// Sum of excess load in all routes
	double timeEarlyExcess = 0.;		// 早到的时间在所有路径中的总和
	double timeLateExcess = 0.;			// 晚到的时间在所有路径中的总和
	bool isFeasible = false;		// Feasibility status of the individual
	void eval_clear()
	{
		penalizedCost = 0.;		// Penalized cost of the solution
		nbRoutes = 0;				// Number of routes
		distance = 0.;			// Total distance
		capacityExcess = 0.;		// Sum of excess load in all routes
		timeEarlyExcess = 0.;		// 早到的时间在所有路径中的总和
		timeLateExcess = 0.;			// 晚到的时间在所有路径中的总和
		isFeasible = false;		// Feasibility status of the individual
	}

};

class Individual
{
public:

	EvalIndiv eval;															// Solution cost parameters
	std::vector < int > chromT ;			//表示一个编码模式					// Giant tour representing the individual，这可以理解为是铺平了
	std::vector < std::vector <int> > chromR ;								// For each vehicle, the associated sequence of deliveries (complete solution)，相关交付序列，这个是每辆车单独一个解
	std::vector < std::vector <double> > timeRS ;								// For each vehicle, the associated sequence of times (complete solution)
	std::vector < std::vector <double> > etimeRS ;								// For each vehicle, the associated sequence of earliest times (complete solution)
	std::vector < double > loadRS ;											// For each vehicle, the associated load (complete solution)
	std::vector <double> chromRCost;										//保存当前解中每一条路径的成本
	std::vector <int> chromRFeasible;
	std::vector < int > successors ;											// For each node, the successor in the solution (can be the depot 0)，注意这些都是对于每一个点来说的
	std::vector < int > predecessors ;										// For each node, the predecessor in the solution (can be the depot 0)
	std::multiset < std::pair < double, Individual* > > indivsPerProximity ;	// The other individuals in the population, ordered by increasing proximity (the set container follows a natural ordering based on the first value of the pair)
	//允许存储多个具有相同键值的元素，允许存储键值相同的元素，就比如说第一个键相同，不能放到set中，但是可以放到multiset中；可以找到与当前个体最接近的个体情况，double中保存的是接近程度
	int cliNum;
	void ind_clear()
	{
		timeRS.clear() ;
		etimeRS.clear() ;
		loadRS.clear() ;
		chromRCost.clear();
		chromRFeasible.clear();
	}

	double biasedFitness;														// Biased fitness of the solution，偏置适应度，是同时考虑了传统适应度和解的多样性

	// Measuring cost and feasibility of an Individual from the information of chromR (needs chromR filled and access to Params)
	void evaluateCompleteCost(const Params & params);

	// Constructor of a random individual containing only a giant tour with a shuffled visit order
	Individual(Params & params);
	// 根据地理聚类的结果，生成一个个体
	void IndividualThree(Params & params,Initial & initial,int Istime);
	void check() const;  //只要客户点数量对就好
	void copy(const Individual & indiv);

	// Constructor of an individual from a file in CVRPLib solution format as produced by the algorithm (useful if a user wishes to input an initial solution)
	//这个可以用来输入一个初始解
	Individual(Params & params, std::string fileName);
};
#endif
