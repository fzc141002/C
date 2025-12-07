#ifndef GES_H
#define GES_H

#include "Params.h"
#include "Individual.h"
#include "LocalSearch.h"
#include "StartTime.h"
#include <deque>
#include <climits>
#include <algorithm>

//做一个车辆数量缩减
class GES
{
private:
    Params& params;
    LocalSearch& localSearch;
    Individual inner_indiv=Individual(params); //需要先随机初始化构造一下
    Individual copy_indiv=Individual(params);
    int insertLoseRoute=0;
    //GES部分参数
    std::vector <std::deque<double>> redundancyL;      //保存每个客户点的早到冗余度
    std::vector <std::deque<double>> culredundancyL;      //保存每个客户点的迟到冗余度
    std::vector <double> minredundancyL;
    std::vector <int> ejetionTime;      //这个后面怎么用还要具体再看一下
    std::vector <int> TfeasibleIS;

    std::vector<int> ejectionPool;	//设置弹出池
	std::vector<int> penaltyCounter;	//惩罚计数器，注意大小是n+1

    int insertNum=0;  //统计一下插入次数，也可以理解为算是ls中的移动次数
    
    //算子更新部分的参数
    std::vector<double> desScore={1,1,1};   //初始得分都是1
    std::vector<double> desWeight={1,1,1};   //初始得分都是1
    std::vector<int> desUseTime={0,0,0};    //一开始都没用
    bool insertIs=false;
    int useWhich=10;

    //破坏的方式可以不同
    void destroy1(Individual& indiv);       //随机破坏
    void destroy2(Individual& indiv);       //选择装载率最低的破坏
    void destroy3(Individual& indiv);       //客户点最少的破坏
    bool insertCustomer(int vIn,int pos);   //前面那个是值，后面那个是位置
    void updataIndiv();
    void updataRoute(int rIn);      //路径编号
    bool squeeze(int rIn,int pos);
    bool feasibleJudge(Individual& indiv);
    void eject(int rIn);   //routeID,route插入的位置以及需要插入的点
    void calALLStartTime(vvi& route,std::deque<double>& etime);   //针对每条路径计算
    double calStartTime(vvi& route,int rpos);
    void exportIndivGes(Individual& indiv);
    void deletepoint(vvi& poses,int rIn)    //是客户点序列
    {
        for (int i:poses)
        {
            inner_indiv.cliNum--;       //客户点减少
            inner_indiv.loadRS[rIn] -= params.cli[i].demand;
        }
    }
    void addpoint(vvi& poses,int rIn)
    {
        for (int i:poses)
        {
            inner_indiv.cliNum++;       //客户点减少
            inner_indiv.loadRS[rIn] += params.cli[i].demand;
        }
    }
    void initial()
    {
        desScore={1,1,1};
        desWeight={1,1,1};
        desUseTime={0,0,0};
    }
    void destoryChoose()
    {
        std::vector<double> destroyRoulette={desWeight[0],desWeight[0]+desWeight[1],desWeight[0]+desWeight[1]+desWeight[2]};
        std::uniform_real_distribution<double> dis(0.0, destroyRoulette[destroyRoulette.size()-1]);
        double ran_float=dis(params.ran);
        if (ran_float>=0 && ran_float<destroyRoulette[0])
            {useWhich=0;destroy1(inner_indiv);}
        else if (ran_float>=destroyRoulette[0] && ran_float<destroyRoulette[1])
            {useWhich=1;destroy2(inner_indiv);}
        else if (ran_float>=destroyRoulette[1] && ran_float<=destroyRoulette[2])
            {useWhich=2;destroy3(inner_indiv);}
    }
    void destroyUp()
    {
        if (insertIs) desScore[useWhich]+=1.5;
        else desScore[useWhich]+=0.6;
        desWeight[useWhich]=desWeight[useWhich]*0.5+0.5*(desScore[useWhich]/desUseTime[useWhich]);
    }

public:

    void run(Individual& indiv,int gesTime);
    GES(Params& params,LocalSearch& localSearch):params(params),localSearch(localSearch) 
    {}    
};

#endif