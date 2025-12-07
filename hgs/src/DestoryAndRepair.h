#ifndef DESTORYANDREPAIR_H
#define DESTORYANDREPAIR_H

#include "Params.h"
#include "LocalSearch.h"
#include "ALNSInitial.h"
#include "ChooseType.h"


class DestoryAndRepair
{
public:
    //基础的系统参数
    Params& params;

    //相关路径参数
    std::vector < int > orderNodes;     //节点编号
    std::vector < int > orderRoutes;    //路径编号
    std::set < int > emptyRoutes;
    std::vector < NodeInitial > clients;
    std::vector < NodeInitial > depots;
    std::vector < NodeInitial > depotsEnd;
    std::vector < RouteInitial > routes;
    std::vector < NodeInitial > solution;
    double penaltyCapacityLS, penaltyEDurationLS, penaltyLDurationLS, penalthRouteLS;

    //随机相关参数
    std::vector< PerturbationType > perturbationTypes = {PerturbationType::Sequential, PerturbationType::Concentric};
    PerturbationType selectedperturbationType;
    std::vector<InsertionHeuristic> insertionHeuristics = {InsertionHeuristic::Distance, InsertionHeuristic::Cost};
    InsertionHeuristic selectedInsertionHeuristic;

    //当前解有关的参数
    int omega=params.ap.dMax;  //控制扰动的种群规模,暂时先等于这个
    int countCandidates;  //候选解的数量情况
    double bestcost=1e10;
    double bestdist=1e10;
    double cost=0.0;
    double costprev=0.0;    //考虑在前后位置插入的情况
    NodeInitial* bestNode=NULL;
    std::vector<int> candidates=std::vector<int>(params.nbClients);  //保存需要删除的解的情况，保存的就是所有客户点

    
    //启发式插入参数
    int indexHeuristic;

    //基础的功能函数
    void setSolution(Individual& indiv);
    void setOrder();
    int getBestKNN(int node,int limit);
    void addCandidates();
    int getNode(int node);
    void loadIndiv(Individual& indiv);
    void updataRouteData(RouteInitial* myRoute);
    double getCost(vvi chromRLS);
    //注意惩罚要初始化
    inline double penaltyExcessDuration(double myEexcess, double myLexcess) { return myEexcess * penaltyEDurationLS + myLexcess * penaltyLDurationLS; }
    inline double penaltyExcessLoad(double myLoad) { return std::max<double>(0., myLoad - params.vehicleCapacity) * penaltyCapacityLS; }
    
    //破坏函数
    void sequentialDestory(Individual& indiv);   //顺序扰动
    void concentricDestory(Individual& indiv);  //同心扰动
    void exportindiv(Individual& indiv);

    //参数更新函数
    int getIndexHeuristic() 
	{
		return indexHeuristic;
	}

    PerturbationType getPerturbationType() 
    {
		return selectedperturbationType;
	}

public:
    void run(Individual& indiv);

    DestoryAndRepair(Params& params):params(params)
    {}
};

#endif