#ifndef STARTTIME_H
#define STARTTIME_H
#include "Params.h"
#include "Individual.h"
typedef std::vector<double> vvd;//记录到达每一个点的时间
typedef std::vector<int> vvi;	//还是得每一条路线单独分开来
typedef std::vector<std::vector<double>> vdouble;
typedef std::vector<std::vector<int>> vint;

void GetStartTime(const Params& params, const vvi& chromR, vvd& timeR, vvd& etimeR, int& isStartFeasible);
//注意这个params应该是一个被缩减过之后的时间窗
void CheckFeasible(const Params& params, const vvi& chromR, const vvd& timeR,vvd& etimR);


#endif //STARTTIME_H