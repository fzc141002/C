#ifndef GLOBALSEARCH_H
#define GLOBALSEARCH_H

#include "Params.h"
#include "Individual.h"
#include "RouteAndNode.h"

class GlobalSearch
{
public:
    Params & params;				// Problem parameters

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

    //还是需要初始化一个弹出池，提高效率
    std::vector<int> ejPool; //用来存储弹出池的点
    std::vector<std::pair<int,int>> initialPos; //保存弹出来的的位置,第一个位置保存路径，第二个位置保存下标
    int sucessCount = 0; //成功的次数

    void loadIndiv(Individual& indiv);
    void updataRouteData(RouteInitial* myRoute);
    
    double getCost(vvi chromRLS);
    void findBest();
    void insertIn();
    void repair();      //暂时不定义修复的函数，还是先定义一个弹出的池子再说
    void exportindiv(Individual& indiv);

    //注意相关的惩罚需要初始化
    inline double penaltyExcessDuration(double myEexcess, double myLexcess) { return myEexcess * penaltyEDurationLS + myLexcess * penaltyLDurationLS; }
    inline double penaltyExcessLoad(double myLoad) { return std::max<double>(0., myLoad - params.vehicleCapacity) * penaltyCapacityLS; }

    // Constructor
    GlobalSearch(Params & params) : params(params) {}


    bool run(Individual& indiv);


};


#endif