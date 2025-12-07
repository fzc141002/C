#include "Initial.h"
#include <iostream>

void Initial::run()
{
    //先生成基本的类
    DisClusterInitial();
    TimeClusterInitial();
    for (int i=0;i<params.nbClients;i++)
    {
        disCluted[discluster[i]].push_back(i+1);
        timeCluted[timecluster[i]].push_back(i+1);
    }

    std::ofstream outFile("cluster.txt");
    if (outFile.is_open())
    {
        outFile << "空间聚类中心点：" << std::endl;
        for (const auto& i:disCluted)
        {
            for (const auto& j : i)
            {
                outFile << j << " ";
            }
            outFile << std::endl;
        }
        outFile << "时间聚类中心点：" << std::endl;
        for (const auto& i : timeCluted)
        {
            for (const auto& j : i)
            {
                outFile << j << " ";
            }
            outFile << std::endl;
        }
        outFile.close();
    }
    else
    {
        std::cerr << "无法打开文件进行写入！" << std::endl;
    }

    std::cout<<"空间聚类结果："<<std::endl;
    for (int i=0;i<params.dclusterNum;i++)
    {
        std::cout<<"第"<<i<<"个空间聚类的点为："<<dcenter[i].first<<","<<dcenter[i].second<<"** ";
        for (int j=0;j<disCluted[i].size();j++)
        {
            std::cout<<disCluted[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
    std::cout<<"时间聚类结果："<<std::endl;
    for (int i=0;i<params.tclusterNum;i++)
    {
        std::cout<<"第"<<i<<"个时间聚类的点为："<<tcenter[i]<<"** ";
        for (int j=0;j<timeCluted[i].size();j++)
        {
            std::cout<<timeCluted[i][j]<<" ";
        }
        std::cout<<std::endl;
    }
}

//这个聚类稍微有点问题
void Initial::DisClusterInitial()
{
    //确定初始聚类的中心
    std::vector<std::pair<double,double>> center;
    //确定当前的坐标边界

    //用kmeans++获取中心
    int cout_center=0;
    int ini_point=1; //从第一个点开始
    std::vector<int> inner_points;
    inner_points.push_back(ini_point); //将初始点加入到内点中
    center.push_back(std::pair<double,double>(params.cli[ini_point].coordX,params.cli[ini_point].coordY));
    while (true)
    {
        if (cout_center>=params.dclusterNum) break; //如果中心点已经够了，就退出
        std::vector<double> disCenter(params.nbClients, 0.0); //记录每个点到中心点的距离
        for (int i=1;i<params.nbClients;i++)
        {
            double min_dis= 1e10; //记录当前点到中心点的最小距离
            for (int j=0;j<(int)inner_points.size();j++)
            {
                double dist=params.timeCost[inner_points[j]][i]*params.timeCost[inner_points[j]][i];
                if (dist<min_dis) min_dis = dist; //更新最小距离
            }
            disCenter[i-1] = min_dis; //更新当前点到中心点的距离
        }
        std::discrete_distribution<> dis(disCenter.begin(), disCenter.end()); //使用离散分布来选择下一个点
        int new_point=dis(params.ran);
        inner_points.push_back(new_point); //将当前的点加入到内点中
        center.push_back(std::pair<double,double>(params.cli[new_point].coordX,params.cli[new_point].coordY));
        cout_center++;
    }
    //对于太集中的center需要去重
    std::set<std::pair<double,double>> unique_center(center.begin(), center.end());
    center.assign(unique_center.begin(), unique_center.end()); //去重后的中心点
    params.dclusterNum = center.size(); //更新时间聚类的数量

    for (int i=0;i<params.dclusterNum;i++)
    {
        std::cout<<"第"<<i<<"个空间聚类的中心点为："<<center[i].first<<"  "<<center[i].second<<std::endl;
    }
    //计算每个点到中心点的距离
    for (int i=0;i<dMaxIter;i++)
    {
        for (int j=1;j<=params.nbClients;j++)
        {
            double min_dist = 1e10;
            int min_index = 0;
            for (int k=0;k<params.dclusterNum;k++)
            {
                double dist = (params.cli[j].coordX - center[k].first) * (params.cli[j].coordX - center[k].first)
                            + (params.cli[j].coordY - center[k].second) * (params.cli[j].coordY - center[k].second);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    min_index = k;
                }
            }
            discluster[j-1] = min_index;
        }

        std::vector<std::pair<double,double>> new_center(params.dclusterNum,std::pair<double,double>(0.0,0.0));
        std::vector<int> count(params.dclusterNum,0);
        for (int j=1;j<=params.nbClients;j++)
        {
            new_center[discluster[j-1]].first += params.cli[j].coordX;
            new_center[discluster[j-1]].second += params.cli[j].coordY;
            count[discluster[j-1]]++;
        }
        for (int k=0;k<params.dclusterNum;k++)
        {
            if (count[k]>0)
            {
                new_center[k].first /= count[k];
                new_center[k].second /= count[k];
            }
            else continue; //如果没有点分配到这个中心点，则不更新
        }
        center=new_center;
    }
    dcenter = center; //保存中心点的坐标
}

void Initial::TimeClusterInitial()
{
    std::vector<double> center;    //对于时间的聚类一般会少一点
    int cout_center=0;
    int ini_point=1; //从第一个点开始
    std::vector<int> inner_points;
    inner_points.push_back(ini_point); //将初始点加入到内点中
    center.push_back(params.cli[ini_point].timeWindow[0]); //将初始点的时间窗口加入到中心点中
    while (true)
    {
        if (cout_center>=params.tclusterNum) break; //如果中心点已经够了，就退出
        std::vector<double> disCenter(params.nbClients, 0.0); //记录每个点到中心点的距离
        for (int i=1;i<params.nbClients;i++)
        {
            double min_dis= 1e20; //记录当前点到中心点的最小距离
            for (int j=0;j<(int)inner_points.size();j++)
            {
                double dist=(params.cli[inner_points[j]].timeWindow[0] - params.cli[i].timeWindow[0])*
                (params.cli[inner_points[j]].timeWindow[0] - params.cli[i].timeWindow[0]);
                if (dist<min_dis) min_dis = dist; //更新最小距离
            }
            disCenter[i-1] = min_dis; //更新当前点到中心点的距离
        }
        std::discrete_distribution<> dis(disCenter.begin(), disCenter.end()); //使用离散分布来选择下一个点
        int new_point=dis(params.ran);
        inner_points.push_back(new_point); //将当前的点加入到内点中
        center.push_back(params.cli[new_point].timeWindow[0]);
        cout_center++;
    }
    //对于太集中的center需要去重
    std::set<double> unique_center(center.begin(), center.end());
    center.assign(unique_center.begin(), unique_center.end()); //去重后的中心点
    params.tclusterNum = center.size(); //更新时间聚类的数量

    for (int i=0;i<params.tclusterNum;i++)
    {
        std::cout<<"第"<<i<<"个时间聚类的中心点为："<<center[i]<<std::endl;
    }
    //计算每个点到中心点的距离
    for (int i=0;i<tMaxIter;i++)
    {
        for (int j=1;j<=params.nbClients;j++)
        {
            double min_dist = 1e20;
            int min_index = 0;
            for (int k=0;k<params.tclusterNum;k++)
            {
                double dist = (params.cli[j].timeWindow[0] - center[k])*(params.cli[j].timeWindow[0] - center[k]);
                if (dist < min_dist)
                {
                    min_dist = dist;
                    min_index = k;
                }
            }
            timecluster[j-1] = min_index;
        }

        std::vector<double> new_center(params.tclusterNum,0.0);
        std::vector<int> count(params.tclusterNum,0);
        for (int j=1;j<=params.nbClients;j++)
        {
            new_center[timecluster[j-1]] += params.cli[j].timeWindow[0];
            count[timecluster[j-1]]++;
        }
        for (int k=0;k<params.tclusterNum;k++)
        {
            if (count[k]>0)
                new_center[k] /= count[k];
            else continue; //如果没有点分配到这个中心点，则不更新
        }
        center=new_center;
    }
    tcenter=center;
}

Initial::Initial(Params& params) : params(params)
{
    //注意第一个括号里面，需要有对应的数据类型
    disCluted = std::vector<std::vector<int>>(params.nbClients,std::vector<int>());
    timeCluted = std::vector<std::vector<int>>(params.nbClients,std::vector<int>());
    discluster = std::vector<int>(params.nbClients, 0);
    timecluster = std::vector<int>(params.nbClients, 0);
}