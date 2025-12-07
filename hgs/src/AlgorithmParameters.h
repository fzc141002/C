#ifndef ALGORITHMPARAMETERS_H
#define ALGORITHMPARAMETERS_H

struct AlgorithmParameters {
	//这个参数主要是用来构建邻域的
	int nbGranular;			// Granular search parameter, limits the number of moves in the RI local search，路径增强的一个阈值
	int mu;					// Minimum population size
	int lambda;				// Number of solutions created before reaching the maximum population size (i.e., generation size)，相当于一个可以活动的范围空间
	int nbElite;			// Number of elite individuals
	int nbClose;			// Number of closest solutions/individuals considered when calculating diversity contribution，表示和某一个解最为接近的解的数量
	int LCS_len;

	double GES_timeLimit;	//GES操作的总时间限制
	int ejMAX;				//单路径弹出最大值，如果增加限制可以提高缩减效果，但是会增加计算量

	//随机扰动参数
	int dMax;				//扰动操作中衡量多样性的一个指标
	int varphi;				//控制随机扰动的强度

	//路径全局搜索参数，下面两个参数后来没有用
	int maxOuter;
	int gstime;

	int nbIterPenaltyManagement;  // Number of iterations between penalty updates，表示惩罚更新之间的迭代次数，再另一篇文章中是100
	double targetFeasible;	      // Reference proportion for the number of feasible individuals, used for the adaptation of the penalty parameters，表示期望的自然生成可行解的占比
	//这个考虑后面要不要分
	double penaltyDecrease;	      // Multiplier used to decrease penalty parameters if there are sufficient feasible individuals，惩罚变化系数
	double penaltyIncrease;	      // Multiplier used to increase penalty parameters if there are insufficient feasible individuals
	double routeWeight;

	int seed;				// Random seed. Default value: 0
	int nbIter;				// Nb iterations without improvement until termination (or restart if a time limit is specified). Default value: 20,000 iterations，在没有优化的情况下，经过20000代迭代结束
	int nbIterTraces;       // Number of iterations between traces display during HGS execution,相当于是过多少代之后会展示结果
	double timeLimit;		// CPU time limit until termination in seconds. Default value: 0 (i.e., inactive)
	int useSwapStar;		// Use SWAP* local search or not. Default value: 1. Only available when coordinates are provided.
};

//这部分代码主要是环境兼容性
#ifdef __cplusplus		//编译指令，如果是c++编译器编译的话
extern "C"		//就保留c的特性
#endif
struct AlgorithmParameters default_algorithm_parameters();
//定义一个结构体函数，默认一个初始化参数的函数

#ifdef __cplusplus
//一个引用为对应的参数
void print_algorithm_parameters(const AlgorithmParameters & ap);
#endif

#endif //ALGORITHMPARAMETERS_H

//在头文件中写声明，在源文件中写函数的实现
//const修饰指针——常量指针 const int*，指针的指向可以修改，但是指针指向的值不可以修改
//指针常量 int* const p，指针的指向不可以改，但是指针指向的值可以修改
