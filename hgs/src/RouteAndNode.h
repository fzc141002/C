#ifndef ROUTEANDNODE_H
#define ROUTEANDNODE_H

#include "Params.h"
#include "Individual.h"
#include "CircleSector.h"

struct NodeInitial;

struct RouteInitial
{
    int cour;							// Route index
	int nbCustomers;					// Number of customers visited in the route
    NodeInitial * depot;						// Pointer to the associated depot
	double duration;					// Total time on the route
	double load;						// Total load on the route
	double reversalDistance;			// Difference of cost if the route is reversed
	double penalty;						// Current sum of load and duration penalties，这个惩罚感觉还需要具体看看
	double tpenalty; //仅考虑交换点之后时间有关的惩罚
	double penAndtime;//把交换前后的总时间和时间惩罚都加起来

    vvi chromRLS;
	//用来表示自己的路径
	vvd timeR;	//直接把更新之后的数据都记录下来
	vvd etimeR;
    int isStartFeasible=0;
    int isRouteFeasible=0;
    double myEtime = 0;
    double myLtime = 0;
    double polarAngleBarycenter;//排序用

    double bestCost = 0;
	bool modified=false;

	//LS有关的参数
	int whenLastModified;
	int whenLastTestedSWAPStar;
	int isfeasible;
	CircleSector sector;
};

struct NodeInitial
{
    bool isDepot;						// Tells whether this node represents a depot or not
	int cour;							// Node index
	int position;						// Position in the route
	double deltaDisD;						//表示删除这个点之后的距离变化成本
	double arriveTime;					//记录这个点的到达时间
    NodeInitial * next;						// Next node in the route order
	NodeInitial * prev;						// Previous node in the route order
	NodeInitial * nextOld;
	NodeInitial * prevOld;
	NodeInitial * chromNext;			// 定义一个序列的前后向关系
	RouteInitial * route;						// Pointer towards the associated route
	double cumulatedLoad;				// Cumulated load on this route until the customer (including itself)
	double cumulatedTime;				// Cumulated time on this route until the customer (including itself)
	double cumulatedReversalDistance;	// Difference of cost if the segment of route (0...cour) is reversed (useful for 2-opt moves with asymmetric problems)，如果将路径反向排序的话，成本会有什么样的变化
	bool nodeBelong=false;
	bool modified=false;

	//LS有关参数
	int whenLastTestedRI;
	double deltaRemoval;

	//把某一个点插到后面去，这个点实在路径中的
	void insertNode(NodeInitial * V)
	{
		//这个操作相当于把U从原来的路径中删除掉
		if (this->prev) this->prev->next = this->next;
		if (this->next) this->next->prev = this->prev;
		V->next->prev = this;
		this->prev = V;
		this->next = V->next;
		V->next = this;
		this->route = V->route;	//把U插到V的路径中去
	}

	//空的点进去，就是这个点是隔离在外面的
	void insertIn(NodeInitial * V)
	{
		V->next->prev = this;
		this->prev = V;
		this->next = V->next;
		V->next = this;
		this->route = V->route;

		this->nodeBelong=true;
		this->modified=true;
		V->modified=true;	//这个量需要修改吗
		V->next->modified=true;
		V->prev->modified=true;
		V->route->modified=true;
	}

	//插到一个点之前，空的插进去，并且是插到之前
	void insertBefore(NodeInitial * V)
	{
		V->prev->next = this;	//把U插到V的前面去,放到depot后面
		this->prev = V->prev;	//U的上一个就是V的上一个
		V->prev = this;
		this->next = V;		//U的下一个就是V
		this->route = V->route;	//把U插到V的路径中去

		this->nodeBelong=true;
		this->modified=true;
		V->modified=true;
		// V->next->modified=true;	//V的next暂时没有变化
		V->prev->modified=true;
		V->route->modified=true;
	}

	//算钱有关的不在这里算
	void remove()
	{
		this->route->modified= true;
		this->modified= true;
		this->next->modified= true;
		this->prev->modified= true;

		this->nodeBelong=false;
		this->prev->next = this->next;	//把U从原来的路径中删除掉
		this->next->prev = this->prev;

		this->route = NULL;	
		this->prev = NULL;	
		this->next = NULL;	
	}

	void swapNode(NodeInitial * V)
	{
		NodeInitial * myVPred = V->prev;
		NodeInitial * myVSuiv = V->next;
		NodeInitial * myUPred = this->prev;
		NodeInitial * myUSuiv = this->next;
		RouteInitial * thisU = this->route;
		RouteInitial * thisV = V->route;

		myUPred->next = V;		//把所有的信息完整的复制过来
		myUSuiv->prev = V;
		myVPred->next = this;
		myVSuiv->prev = this;

		this->prev = myVPred;
		this->next = myVSuiv;
		V->prev = myUPred;
		V->next = myUSuiv;

		this->route = thisV;
		V->route = thisU;
	}
};

#endif