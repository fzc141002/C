#ifndef ALNSINITIAL_H
#define ALNSINITIAL_H

#include "Params.h"
#include "Individual.h"
#include "RouteAndNode.h"

class ALNSInital
{
private:
    Params & params;
    Individual & indiv;

    NodeInitial * nodeU ;
	NodeInitial * nodeX ;			//x和y分别是u和v在路径route上的后继节点
    NodeInitial * nodeV ;
	NodeInitial * nodeY ;
	RouteInitial * routeU ;		//route就是包含这个点的路径
	RouteInitial * routeV ;
	int nodeUPrevIndex, nodeUIndex, nodeXIndex, nodeXNextIndex ;	
	int nodeVPrevIndex, nodeVIndex, nodeYIndex, nodeYNextIndex ;	
	double loadU, loadX, loadV, loadY;
	
    //初始化所需的外部参数
    std::unordered_set<std::pair<int, int>, PairHash>& infeasibleEdge = params.deleteEdges;
    int countNotInserted = params.nbClients;
    std::vector<int> unsignedOrder = std::vector<int>(params.nbClients);
    int minRoute=params.mincar;


    std::vector < int > orderNodes;     //节点编号
    std::vector < int > orderRoutes;    //路径编号
    std::set < int > emptyRoutes;
    std::vector < NodeInitial > clients;
    std::vector < NodeInitial > depots;
    std::vector < NodeInitial > depotsEnd;
    std::vector < RouteInitial > routes;
    double penaltyCapacityLS, penaltyEDurationLS, penaltyLDurationLS, penalthRouteLS;

    void updataRouteData(RouteInitial* myRoute);
    inline double penaltyExcessDuration(double myEexcess, double myLexcess) { return myEexcess * penaltyEDurationLS + myLexcess * penaltyLDurationLS; }
    inline double penaltyExcessLoad(double myLoad) { return std::max<double>(0., myLoad - params.vehicleCapacity) * penaltyCapacityLS; }

    void insertNode(NodeInitial * U, NodeInitial * V);
    void insertIn(NodeInitial * U, NodeInitial * V);    //仅仅只是把U插入到V之后
    void swapNode(NodeInitial * U, NodeInitial * V);
    void getCost(RouteInitial* myRoute);
    void setLocalVariablesRouteU();
    void setLocalVariablesRouteV();

public:
    void loadindiv(Individual& indiv);
    void exportindiv(Individual& indiv);
    void getSolution();
    int findBestPosition(const int& Node,const int& Route);
    void getBestRoutes(int& Node);
    void run();

    ALNSInital(Params& params,Individual& indiv)
    :params(params),indiv(indiv)
    {
        for (int i = 0; i < params.nbClients; i++) unsignedOrder[i]=i+1;
    };
};



#endif