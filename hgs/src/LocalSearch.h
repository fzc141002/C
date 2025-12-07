#ifndef LOCALSEARCH_H
#define LOCALSEARCH_H
#define MIN_F 0.001
//20的时候是1分22秒，配10邻居会卡住
//100的时候是1分07秒
#include "Individual.h"
#include "RouteAndNode.h"
#include <fstream>
#include <iostream>
#include <string>
#include <algorithm>

// Structure used in SWAP* to remember the three best insertion positions of a customer in a given route
// 寻找三个最佳的插入位置
struct ThreeBestInsert
{
	//针对对当前客户点u和目标路径V来说的
	int whenLastCalculated;	//和nbMoves有关系
	double bestCost[3];	//由小到大的记录三个成本
	NodeInitial * bestLocation[3];	//对应三个成本的最佳位置

	//和已经确定好的三个成本和位置进行对比
	void compareAndAdd(double costInsert, NodeInitial * placeInsert)
	{
		if (costInsert >= bestCost[2]) return;	//对比三种插入模式的优劣性
		else if (costInsert >= bestCost[1])
		{
			bestCost[2] = costInsert; bestLocation[2] = placeInsert;
		}
		else if (costInsert >= bestCost[0])
		{
			bestCost[2] = bestCost[1]; bestLocation[2] = bestLocation[1];
			bestCost[1] = costInsert; bestLocation[1] = placeInsert;
		}
		else
		{
			bestCost[2] = bestCost[1]; bestLocation[2] = bestLocation[1];
			bestCost[1] = bestCost[0]; bestLocation[1] = bestLocation[0];
			bestCost[0] = costInsert; bestLocation[0] = placeInsert;
		}
	}

	// Resets the structure (no insertion calculated)
	// 插入结构体重置
	void reset()	//都会重置成一个很大的数
	{
		bestCost[0] = 1.e30; bestLocation[0] = NULL;
		bestCost[1] = 1.e30; bestLocation[1] = NULL;
		bestCost[2] = 1.e30; bestLocation[2] = NULL;
	}

	ThreeBestInsert() { reset(); };		//相当于也是一个构造函数
};

// Structured used to keep track of the best SWAP* move
struct SwapStarElement
{
	//其实本质上是对路径的交换
	double moveCost = 1.e30 ;
	NodeInitial * U = NULL ;
	NodeInitial * bestPositionU = NULL;
	NodeInitial * V = NULL;
	NodeInitial * bestPositionV = NULL;
};

// Main local learch structure
class LocalSearch
{

private:
	
	Params & params ;							// Problem parameters
	bool searchCompleted;						// Tells whether all moves have been evaluated without success
	int nbMoves;								// Total number of moves (RI and SWAP*) applied during the local search. Attention: this is not only a simple counter, it is also used to avoid repeating move evaluations；对于移动次数的粗略估计
	//在这里暂时不考虑模式增强PI
	std::vector < int > orderNodes;				// Randomized order for checking the nodes in the RI local search
	//这里的order，其实是顺序的意思
	std::vector < int > orderRoutes;			// Randomized order for checking the routes in the SWAP* local search
	std::set < int > emptyRoutes;				// indices of all empty routes，代表所有空路线的下表
	int loopID;									// Current loop index
	/* THE SOLUTION IS REPRESENTED AS A LINKED LIST OF ELEMENTS */
	std::vector < NodeInitial > clients;				// Elements representing clients (clients[0] is a sentinel and should not be accessed)？？？什么意思
	std::vector < NodeInitial > depots;				// Elements representing depots，这个代表的是起点
	std::vector < NodeInitial > depotsEnd;				// Duplicate of the depots to mark the end of the routes，这个代表的是终点的那个depot
	std::vector < RouteInitial > routes;				// Elements representing routes
	std::vector < std::vector < ThreeBestInsert > > bestInsertClient;   // (SWAP*) For each route and node, storing the cheapest insertion cost 

	/* TEMPORARY VARIABLES USED IN THE LOCAL SEARCH LOOPS */
	// nodeUPrev -> nodeU -> nodeX -> nodeXNext
	// nodeVPrev -> nodeV -> nodeY -> nodeYNext
	NodeInitial * nodeU ;
	NodeInitial * nodeX ;			//x和y分别是u和v在路径route上的后继节点
    NodeInitial * nodeV ;
	NodeInitial * nodeY ;
	RouteInitial * routeU ;		//route就是包含这个点的路径
	RouteInitial * routeV ;
	int nodeUPrevIndex, nodeUIndex, nodeXIndex, nodeXNextIndex ;	
	int nodeVPrevIndex, nodeVIndex, nodeYIndex, nodeYNextIndex ;	
	double loadU, loadX, loadV, loadY;
	double serviceU, serviceX, serviceV, serviceY;
	//这是惩罚的系数
	double penaltyCapacityLS, penaltyEDurationLS, penaltyLDurationLS, penalthRouteLS;
	bool intraRouteMove ;

	//初始化函数
	void setLocalVariablesRouteU(); // Initializes some local variables and distances associated to routeU to avoid always querying the same values in the distance matrix
	void setLocalVariablesRouteV(); // Initializes some local variables and distances associated to routeV to avoid always querying the same values in the distance matrix

	//计算两个惩罚，注意对于double来说最好能有精度添加
	inline double penaltyExcessDuration(double myEexcess, double myLexcess) { return myEexcess * penaltyEDurationLS + myLexcess * penaltyLDurationLS; }
	// inline double penaltyExcessDuration(double myEexcess, double myLexcess) { return myEexcess * penaltyEDurationLS + myLexcess * penaltyLDurationLS; }
	inline double penaltyExcessLoad(double myLoad) { return std::max<double>(0., myLoad - params.vehicleCapacity) * penaltyCapacityLS; }

	/* RELOCATE MOVES */
	// (Legacy notations: move1...move9 from Prins 2004)
	bool move1(int routePen); // If U is a client node, remove U and insert it after V
	bool move2(int routePen); // If U and X are client nodes, remove them and insert (U,X) after V
	bool move3(int routePen); // If U and X are client nodes, remove them and insert (X,U) after V
	//定义了不同的移动方式

	/* SWAP MOVES */
	bool move4(int routePen); // If U and V are client nodes, swap U and V
	bool move5(int routePen); // If U, X and V are client nodes, swap (U,X) and V
	bool move6(int routePen); // If (U,X) and (V,Y) are client nodes, swap (U,X) and (V,Y) 
	 
	/* 2-OPT and 2-OPT* MOVES */
	bool move7(int routePen); // If route(U) == route(V), replace (U,X) and (V,Y) by (U,V) and (X,Y)
	bool move8(int routePen); // If route(U) != route(V), replace (U,X) and (V,Y) by (U,V) and (X,Y)
	bool move9(int routePen); // If route(U) != route(V), replace (U,X) and (V,Y) by (U,Y) and (V,X)

	/* SUB-ROUTINES FOR EFFICIENT SWAP* EVALUATIONS */
	bool swapStar(); // Calculates all SWAP* between routeU and routeV and apply the best improving move
	//这个具体怎么交换还是要考虑一下的
	double getCheapestInsertSimultRemoval(NodeInitial * U, NodeInitial * V, NodeInitial *& bestPosition); // Calculates the insertion cost and position in the route of V, where V is omitted
	void preprocessInsertions(RouteInitial * R1, RouteInitial * R2); // Preprocess all insertion costs of nodes of route R1 in route R2

	/* ROUTINES TO UPDATE THE SOLUTIONS */
	//static无需创建对象，直接调用
	vvd getPenalty(const vvi& chromInner);
	static void insertNode(NodeInitial * U, NodeInitial * V);		// Solution update: Insert U after V
	static void swapNode(NodeInitial * U, NodeInitial * V) ;		// Solution update: Swap U and V	
	//这两个交换路径可相同，也可以不同
	//后面这两个函数好像不需要
	void updateRouteData(RouteInitial * myRoute);			// Updates the preprocessed data of a route
	//获得翻转之后的路径

	public:

	// Run the local search with the specified penalty values
	//sym代表要不要扩展新路径
	void run(Individual& indiv, double penaltyCapacityLS, double penaltyEDurationLS, double penaltyLDurationLS ,double penalthRouteLS,int sym,std::vector<int>& ejectionPool);	  //带有特定的惩罚

	// Loading an initial solution into the local search
	void loadIndividual(const Individual & indiv);
	void checkIndividual(const Individual & indiv);

	// Exporting the LS solution into an individual and calculating the penalized cost according to the original penalty weights from Params
	//将localsearch的结果到处
	void exportIndividual(Individual & indiv);


	// Constructor
	LocalSearch(Params & params);
};

#endif
