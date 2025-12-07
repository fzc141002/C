#ifndef GATHER_H
#define GATHER_H

#include "Params.h"
#include "Individual.h"
#include "StartTime.h"
#include "ChooseType.h"
#include "DestoryAndRepair.h"
#include <cstdio>

class Gather
{
public:
    Params& params;
    int iteror=0; // 迭代次数
    int lastNum=0;
    int thisDelete=0;
    // 客户数量（不含depot）
    int num_customers = params.nbClients;
    // std::vector<std::deque<int>> a;
    std::vector<std::vector<int>> anew;
    std::vector<std::vector<int>> apertub;   //接受内部扰动之后的结果
    std::vector<std::vector<int>> innerChromGather=std::vector<std::vector<int>>();	//用来存储新的chromGather
	std::vector<double> innerCostGather=std::vector<double>();	//用来存储新的costGather
    DestoryAndRepair destoryAndRepair;

    Gather(Params &params,DestoryAndRepair& destoryAndRepair) : params(params),destoryAndRepair(destoryAndRepair)
    {
        anew=std::vector<std::vector<int>>(params.nbClients);
        apertub=std::vector<std::vector<int>>(params.nbClients);
    }
    // Gather data from the population and write to a file
    bool run(int minNowCar,Individual& gatherIndiv,int carLast);
    double costRoute(vvi chromR);
};


#endif
