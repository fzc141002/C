#include "StartTime.h"

void GetStartTime(const Params& params, const vvi& chromR, vvd& timeR, vvd& etimeR,int& isStartFeasible)
{
	//暂时假设车辆的数量会发生变化
	//一开始默认为1就可以了
	//注意最后还要回到仓库
	timeR.clear();
	etimeR.clear();
	//添加出发时间,就是刚刚好准时到达第一个点
	// 这个出发时间可以写成一个浮动的值
	double ini_time = std::max<double>(params.cli[chromR[0]].timeWindow[0] - params.timeCost[0][chromR[0]], 0.);   //反正目标就是不能让第一点产生等待
	// double ini_time = 0.0;   //现在的出发时间都是按照0来算的
	//如果比第一个点的晚时间窗早的话
	timeR.push_back(ini_time);
	etimeR.push_back(0);
	if (ini_time+ params.timeCost[0][chromR[0]]+MY_EPSILON< params.cli[chromR[0]].timeWindow[0])
	{
		etimeR.push_back(ini_time + params.timeCost[0][chromR[0]]-params.cli[chromR[0]].timeWindow[0]);//这个也代表的是第一个点
		timeR.push_back(params.cli[chromR[0]].timeWindow[0]);
	}
	else
	{
		if (ini_time+ params.timeCost[0][chromR[0]]+MY_EPSILON<=params.cli[chromR[0]].timeWindow[1])
		{
			etimeR.push_back(0);
		}
		else
		{
			etimeR.push_back(ini_time + params.timeCost[0][chromR[0]] - params.cli[chromR[0]].timeWindow[1]);
			isStartFeasible = 0;
		}
		timeR.push_back(ini_time+ params.timeCost[0][chromR[0]]);
	}
	
	for (int i = 1; i < (int)chromR.size(); i++)
	{
		//到达下一个点的时间=前一个点到达时间+服务时间+到达下一个点路上所需时间
		double arriveInner = timeR[i] + params.timeCost[chromR[i - 1]][chromR[i]] + params.cli[chromR[i - 1]].serviceDuration;//还需要加上服务时间
		double nextEWindow = params.cli[chromR[i]].timeWindow[0];
		double nextLWindow = params.cli[chromR[i]].timeWindow[1];
		if (arriveInner > nextLWindow + MY_EPSILON)
		{
			//早到为负，迟到为正
			isStartFeasible = 0;//表示当前解不可行
			etimeR.push_back(arriveInner - nextLWindow);	 //正的迟到了
			timeR.push_back(arriveInner);
		}
		else
		{
			if (arriveInner > nextEWindow + MY_EPSILON)
			{
				etimeR.push_back(0);//在中间刚好合适
				timeR.push_back(arriveInner);
			}
			else
			{
				etimeR.push_back(arriveInner - nextEWindow);//负的表示迟到了
				timeR.push_back(nextEWindow);//到达时间就要变得迟了
			}
		}
	}
	//最后还要回到仓库
	double arriveInner = timeR[timeR.size() - 1] + params.timeCost[chromR[chromR.size() - 1]][0] + params.cli[chromR[chromR.size() - 1]].serviceDuration;
	double depotLWindow = params.cli[0].timeWindow[1];
	if (arriveInner > depotLWindow + MY_EPSILON)
	{
		isStartFeasible = 0;
		etimeR.push_back(arriveInner - depotLWindow);
		timeR.push_back(arriveInner);
	}
	else
	{
		etimeR.push_back(0);
		timeR.push_back(arriveInner);
	}
}

//直接把整个解的惩罚都算在一起,这个再输入的时候，时间也跟着一块输入了，固定不变了
void CheckFeasible(const Params& params, const vvi& chromR, const vvd& timeR,vvd& etimR)
{
	//注意成本要考虑全面
	etimR.clear();	//把所有内容都清空
	//因为timeR里面保存了n+2个点，两个depot也在里面的
	etimR.push_back(0);
	for (int i = 0; i < (int)chromR.size(); i++)
	{
		//如果早到的话是一个可行解，但是也要惩罚
		if (timeR[i+1] < params.pretimeWindow[chromR[i]][0] - MY_EPSILON)
			etimR.push_back(timeR[i+1]-params.pretimeWindow[chromR[i]][0]);
		else if (timeR[i+1] > params.pretimeWindow[chromR[i]][1] + MY_EPSILON)
			etimR.push_back(timeR[i+1]-params.pretimeWindow[chromR[i]][1]);
		else etimR.push_back(0);
		//晚于depot的时间也要惩罚
	}
	if (timeR[timeR.size()-1] > params.pretimeWindow[0][1] + MY_EPSILON)
		etimR.push_back(timeR[timeR.size()-1]-params.pretimeWindow[0][1]);
	else etimR.push_back(0);
}