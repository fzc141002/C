#include "Preprocess.h"

void Preprocess::preprocessTimeWindows()
{
    //时间窗首先根据三角不等式缩减
    for (int i=1 ;i <= params.nbClients; i++)
    {
        params.cli[i].timeWindow[0] = std::max(params.cli[0].timeWindow[0]+params.timeCost[0][i],params.cli[i].timeWindow[0]);
        params.cli[i].timeWindow[1] = std::min(params.cli[0].timeWindow[1]-params.timeCost[i][0],params.cli[i].timeWindow[1]);
        double e_arrive=0.0;
        double l_arrive=0.0;
        double e_leave=0.0;
        double l_leave=0.0;
        //注意j和i的顺序是相反的
    }

    while(true)
    {
        changed=false;
        Rule1();
        Rule2();
        Rule3();
        Rule4();
        if (!changed) break;
    }
}

void Preprocess::Rule1()
{
    for (int j=1;j <= params.nbClients;j++)
    {
        double preT=my_MAX;
        int bestNode=0;
        for (int i=1;i <= params.nbClients;i++)
        {
            //让前序和后序彻底一点
            // if (params.cli[i].timeWindow[0] + params.cli[i].serviceDuration < params.cli[j].timeWindow[1])
            // {
                if (i == j) continue;
                double inner=params.cli[i].timeWindow[0]+params.timeCost[i][j]+params.cli[i].serviceDuration;
                if (inner<preT-MY_EPSILON) {preT=inner;bestNode=i;}
            // }
        }
        preT=std::max(params.cli[j].timeWindow[0],std::min(params.cli[j].timeWindow[1],preT));
        if (preT - MY_EPSILON > params.cli[j].timeWindow[0])
        {
            if (params.cli[j].timeWindow[1]-preT > params.reducelimit[j]+MY_EPSILON)
            {
                params.cli[j].timeWindow[0] = preT;
                keypoints[j-1][0] = bestNode; // 前前点
                changed = true;
            }
           
        }
    }
}

void Preprocess::Rule2()
{
    for (int j=1;j <= params.nbClients;j++)
    {
        double preT=my_MAX;
        int bestNode=0;
        for (int i=1;i <= params.nbClients;i++)
        {
            // if (params.cli[j].serviceDuration+params.cli[j].timeWindow[0] > params.cli[i].timeWindow[1])
            // {
                if (i == j) continue;
                double inner=params.cli[i].timeWindow[0]-params.timeCost[j][i]-params.cli[j].serviceDuration;
                if (inner<preT-MY_EPSILON) {preT=inner; bestNode=i;}
            // }  
        }
        preT=std::max(params.cli[j].timeWindow[0],std::min(params.cli[j].timeWindow[1],preT));
        if (preT-MY_EPSILON > params.cli[j].timeWindow[0])
        {
            if (params.cli[j].timeWindow[1]-preT > params.reducelimit[j]+MY_EPSILON)
            {
                params.cli[j].timeWindow[0] = preT;
                keypoints[j-1][1] = bestNode; // 前后点
                changed=true;
            }
                
        }
    }
}

void Preprocess::Rule3()
{
    for (int j=1;j <= params.nbClients;j++)
    {
        double preT=-my_MAX;
        int bestNode=0;
        for (int i=1;i <= params.nbClients;i++)
        {
            // if (params.cli[i].timeWindow[1] + params.cli[i].serviceDuration < params.cli[j].timeWindow[1])
            // {
                if (i == j) continue;
                double inner=params.cli[i].timeWindow[1]+params.timeCost[i][j]+params.cli[i].serviceDuration;
                if (inner>preT+MY_EPSILON) {preT=inner;bestNode=i;}
            // }
        }
        preT=std::min(params.cli[j].timeWindow[1],std::max(params.cli[j].timeWindow[0],preT));
        if (preT + MY_EPSILON < params.cli[j].timeWindow[1])
        {
            if (preT - params.cli[j].timeWindow[0] > params.reducelimit[j]+MY_EPSILON)
            {
                params.cli[j].timeWindow[1] = preT;
                keypoints[j-1][2] = bestNode; // 后前点
                changed=true;
            }
                
        }
    }
}

void Preprocess::Rule4()
{
    for (int j=1;j <= params.nbClients;j++)
    {
        double preT=-my_MAX;
        int bestNode=0;
        for (int i=1;i <= params.nbClients;i++)
        {
            // if (params.cli[j].serviceDuration+params.cli[j].timeWindow[1] > params.cli[i].timeWindow[1])
            // {
                if (i == j) continue;
                double inner=params.cli[i].timeWindow[1]-params.timeCost[j][i]-params.cli[j].serviceDuration;
                if (inner>preT+MY_EPSILON) {preT=inner;bestNode=i;}
            // }
        }
        preT=std::min(params.cli[j].timeWindow[1],std::max(params.cli[j].timeWindow[0],preT)); 
        if (preT + MY_EPSILON < params.cli[j].timeWindow[1])
        {
            if (preT - params.cli[j].timeWindow[0] > params.reducelimit[j]+MY_EPSILON)
            {
                params.cli[j].timeWindow[1] = preT;
                keypoints[j-1][3] = bestNode; // 后后点
                changed = true;
            }
        }
    }
}

void Preprocess::edgedelete()
{
    //需要从时间窗的角度考虑，每一条路径都要判断
    for (int i=0;i<=(int)params.nbClients;i++)
    {
        for (int j=0;j<=(int)params.nbClients;j++)
        {
            if (params.cli[i].timeWindow[0]+params.cli[i].serviceDuration+params.timeCost[i][j]>params.cli[j].timeWindow[1]+MY_EPSILON)
            {
                directedEdges.insert({i,j});    //通过edge.count({i,j})可以查询元素
            }
        }
    }
}

void Preprocess::infeasibleEdgeGather()
{
    for (int i=0;i<=(int)params.nbClients;i++)
    {
        for (int j=0;j<=(int)params.nbClients;j++)
        {
            if (params.cli[i].timeWindow[0]+params.cli[i].serviceDuration+params.timeCost[i][j]>params.cli[j].timeWindow[1]+MY_EPSILON)
            {
                params.infeasibleEdge[i].push_back(j);
            }
        }
    }
}

void Preprocess::feasibleEdgeGather()
{
    for (int i=0;i<(int)params.nbClients;i++)
    {
        for (int j=0;j<(int)params.nbClients;j++)
        {
            if (params.cli[i].timeWindow[0]+params.cli[i].serviceDuration+params.timeCost[i][j]<=params.cli[j].timeWindow[1]+MY_EPSILON)
            {
                if (i!=j)
                    params.feasibleEdge.push_back(i*200+j);
            }
        }
    }
}

Preprocess::Preprocess(Params& params) : params(params) 
{
    //把关键节点初始化，保存的顺序为前前、前后、后前、后后
    keypoints = std::vector<std::vector<int>>(params.nbClients, std::vector<int>(4, 0));
    changed = false;
}