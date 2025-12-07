#ifndef PARAMS_H
#define PARAMS_H

#include "CircleSector.h"
#include "AlgorithmParameters.h"
#include <string>
#include <vector>
#include <list>
#include <set>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <time.h>
#include <climits>
#include <algorithm>
#include <unordered_set>
#include <random>
#include <deque>
#define MY_EPSILON 0.00001 // Precision parameter, used to avoid numerical instabilities
#define PI 3.14159265359

struct PairHash {
    template <class T1, class T2>   //调用模板之后，可以处理任何的数据类型
    //std::size_t是一种标准的整数整数类型
    //重载operator可以创建可调用对象
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto hash1 = std::hash<T1>{}(p.first);
        auto hash2 = std::hash<T2>{}(p.second);
        return hash1 ^ (hash2 << 1); //避免产生重复的哈希值,移位之后异或，组合成一个新的哈希值
    }
};

struct Client
{
	double coordX;			// Coordinate X
	double coordY;			// Coordinate Y
	double serviceDuration; // Service duration
	double demand;			// Demand
	double timeWindow[2];	// 用来记录客户的前后时间窗
	int polarAngle;			// Polar angle of the client around the depot, measured in degrees and truncated for convenience
	//客户点相对于仓库的极角判断
};

class Params
{
public:

	/* PARAMETERS OF THE GENETIC ALGORITHM */
	bool verbose;                       // Controls verbose level through the iterations
	//控制参数输出的详细程度
	AlgorithmParameters ap;	            // Main parameters of the HGS algorithm

	//可行解的路径集合
	std::deque<std::vector<int>> chromGather=std::deque<std::vector<int>>();
	std::deque<double> costGather=std::deque<double>();
	std::vector<std::vector<int>> newChromGather=std::vector<std::vector<int>>();	//用来存储新的chromGather
	std::vector<double> newCostGather=std::vector<double>();	//用来存储新的costGather
	int boolNum;
	int boolinner;

	/* ADAPTIVE PENALTY COEFFICIENTS */
	//对于超出时间和容量限制的自适应惩罚参数
	double penaltyCapacity;				// Penalty for one unit of capacity excess (adapted through the search)
	//感觉这个载重惩罚后面也要修改
	//暂时先定义两个参数表示早到和迟到
	double penaltyEarly;				// Penalty for one unit of duration excess (adapted through the search)
	double penaltyLate;		//迟到的惩罚会更加严重一点
	double penDis;
	double cpencap;
	double cpeneary;
	double cpenlate;		//惩罚系数，用来最后的输出
	double cpendis;
	double routepen;
	std::vector<std::vector<double>> pretimeWindow;
	std::vector<double> reducelimit;
	//所需车辆的数量需要动态调整一下
	int dclusterNum;		//在一开始聚类中的簇数
    int tclusterNum=10;
	/* START TIME OF THE ALGORITHM */
	clock_t startTime;                  // Start time of the optimization (set when Params is constructed)
	//这个记录的是算法的开始时间
	/* RANDOM NUMBER GENERATOR */       
	std::minstd_rand ran;               // Using the fastest and simplest LCG. The quality of random numbers is not critical for the LS, but speed is

	/* DATA OF THE PROBLEM INSTANCE */
	bool isDurationConstraint ;								// Indicates if the problem includes duration constraints
	int nbClients ;											// Number of clients (excluding the depot)
	int nbVehicles ;										// Number of vehicles
	double vehicleCapacity;									// Capacity limit
	double totalDemand ;									// Total demand required by the clients
	double maxDemand;										// Maximum demand of a client
	double maxDist;											// Maximum distance between two clients
	int mincar;
	std::vector< Client > cli ;								// Vector containing information on each client
	//这个只是定义了一下，还没初始化
	const std::vector< std::vector< double > >& timeCost;	// Distance matrix，减少数据存储量
	std::vector< std::vector< int > > correlatedVertices;	// Neighborhood restrictions: For each client, list of nearby customers，保存临近的点
	bool areCoordinatesProvided;                            // Check if valid coordinates are provided
	std::string fileName;

	//edge缩减参数
	std::unordered_set<std::pair<int, int>, PairHash> deleteEdges;
	std::vector<std::vector<int>> infeasibleEdge;
	std::vector<int> feasibleEdge;

	// Initialization from a given data set
	Params(const std::vector<double>& x_coords,
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
		const AlgorithmParameters& ap);
};
#endif

