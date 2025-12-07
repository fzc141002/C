#ifndef INITIAL_H
#define INITIAL_H

#include "Params.h"
#include <vector>

class Initial
{
public:
    int dMaxIter=20;
    int tMaxIter=20;
    Params& params;
    std::vector<std::vector<int>> disCluted;
    std::vector<std::vector<int>> timeCluted;
    std::vector<int> discluster;
    std::vector<int> timecluster;
    std::vector<std::pair<double, double>> dcenter; //保存中心点的坐标
    std::vector<double> tcenter; //保存时间中心点

    void run();
    void DisClusterInitial();
    void TimeClusterInitial();
     
    Initial(Params& params);
};


#endif // INITIAL_H