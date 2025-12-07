#ifndef SPLIT_H
#define SPLIT_H

#include "Params.h"
#include "Individual.h"
#include "StartTime.h"

//构建一个“客户点分离”的结构体
struct ClientSplit
{
	double demand;
	double serviceTime;
	double d0_x;	//0到x的距离
	double dx_0;	//x到0的距离
	double dnext;	//到下一个客户节点的距离
	double timeNow;	//记录的是到这个点的时间
	double etimeNow; //记录当前节点在当前路径中的延迟或者是提前的时间，提前为负，延迟为正
	int c_id;
	ClientSplit() : demand(0.), serviceTime(0.), d0_x(0.), dx_0(0.), dnext(0.), timeNow(0.), etimeNow(0.), c_id(0) {};
};

// Simple Deque which is used for all Linear Split algorithms
struct Trivial_Deque		//双端队列
{
	std::vector <int> myDeque; // Simply a vector structure to keep the elements of the queue
	int indexFront; // Index of the front element
	int indexBack; // Index of the back element
	//inline是一个关键字，表示函数应该被内联展开
	inline void pop_front() { indexFront++; } // Removes the front element of the queue D，移除队列的前端元素，相当于就是把指向开头的指针往后移一个，内联的方式，编译器会直接把这行代码放到对应需要的地方，而不进行常规的函数操作，适合函数体简单，并且会被频繁调用的情况
	inline void pop_back() { indexBack--; } // Removes the back element of the queue D，尾部删掉一个
	inline void push_back(int i) { indexBack++; myDeque[indexBack] = i; } // Appends a new element to the back of the queue D
	inline int get_front() { return myDeque[indexFront]; }//获得头部第一个元素
	inline int get_next_front() { return myDeque[indexFront + 1]; }
	inline int get_back() { return myDeque[indexBack]; }
	void reset(int firstNode) { myDeque[0] = firstNode; indexBack = 0; indexFront = 0; }	 //把这个队列回复初始化
	inline int size() { return indexBack - indexFront + 1; }	//获取队列大小

	Trivial_Deque(int nbElements, int firstNode)
	{
		myDeque = std::vector <int>(nbElements);
		myDeque[0] = firstNode;
		indexBack = 0;
		indexFront = 0;
	}
};

class Split
{

private:

	// Problem parameters
	const Params& params;		//定义一个常量
	int maxVehicles;

	/* Auxiliary data structures to run the Linear Split algorithm */
	std::vector < ClientSplit > cliSplit;
	std::vector < std::vector < double > > potential;  // Potential vector
	std::vector < std::vector < int > > pred;  // Indice of the predecessor in an optimal path；最优路径的前驱指引索引
	std::vector <double> sumDistance; // sumDistance[i] for i > 1 contains the sum of distances : sum_{k=1}^{i-1} d_{k,k+1}
	std::vector <double> sumLoad; // sumLoad[i] for i >= 1 contains the sum of loads : sum_{k=1}^{i} q_k
	std::vector <double> sumService; // sumService[i] for i >= 1 contains the sum of service time : sum_{k=1}^{i} s_k
	//感觉这边可能会需要额外加一些东西进去，需要有一些在时间上的惩罚
	std::vector <double> sumTimeR;	//累积时间
	std::vector <double> sumETimeR;//累计总时间惩罚
	std::vector <double> eTimeNode;//每个点各自的时间惩罚
	// To be called with i < j only
	// Computes the cost of propagating the label i until j
	//label可以理解为是0-t的最短路径成本
	inline double propagate(int i, int j, int k) //传播成本，k表示第k辆车
	{
		//sumdistance应该就是p，就是从原点出发过去的单程累计距离
		//potential可以看成是在节点i的基础上求到节点j的潜在成本
		return potential[k][i] + sumDistance[j] - sumDistance[i + 1] + cliSplit[i + 1].d0_x + cliSplit[j].dx_0
			+ params.penaltyCapacity * std::max<double>(sumLoad[j] - sumLoad[i] - params.vehicleCapacity, 0.);
	}

	// Tests if i dominates j as a predecessor for all nodes x >= j+1
	// We assume that i < j
	inline bool dominates(int i, int j, int k)		//判断是否是一个支配解
	{
		//相较于原文多了一个载重惩罚
		return potential[k][j] + cliSplit[j + 1].d0_x > potential[k][i] + cliSplit[i + 1].d0_x + sumDistance[j + 1] - sumDistance[i + 1]
			+ params.penaltyCapacity * (sumLoad[j] - sumLoad[i]);
	}

	// Tests if j dominates i as a predecessor for all nodes x >= j+1
	// We assume that i < j
	inline bool dominatesRight(int i, int j, int k)
	{
		//
		return potential[k][j] + cliSplit[j + 1].d0_x < potential[k][i] + cliSplit[i + 1].d0_x + sumDistance[j + 1] - sumDistance[i + 1] + MY_EPSILON;
	}

	// Split for unlimited fleet
	int splitSimple(Individual& indiv);

	// Split for limited fleet
	int splitLF(Individual& indiv);

public:

	// General Split function (tests the unlimited fleet, and only if it does not produce a feasible solution, runs the Split algorithm for limited fleet)，在普通的无限规模下会使用这个
	void generalSplit(Individual& indiv, int nbMaxVehicles);

	// Constructor
	Split(const Params& params);

};
#endif