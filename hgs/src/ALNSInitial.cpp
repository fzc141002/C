#include "ALNSInitial.h"

void ALNSInital::setLocalVariablesRouteU()
{
	routeU = nodeU->route;	//这是一个赋值操作
	nodeX = nodeU->next;	//定义一下nodeX是什么
	nodeXNextIndex = nodeX->next->cour;
	nodeUIndex = nodeU->cour;
	nodeUPrevIndex = nodeU->prev->cour;
	nodeXIndex = nodeX->cour;
	loadU = params.cli[nodeUIndex].demand;
	loadX = params.cli[nodeXIndex].demand;
}

void ALNSInital::setLocalVariablesRouteV()
{
	routeV = nodeV->route;
	nodeY = nodeV->next;	//无法保证nodeY不是depot
	nodeYNextIndex = nodeY->next->cour;
	nodeVIndex = nodeV->cour;
	nodeVPrevIndex = nodeV->prev->cour;
	nodeYIndex = nodeY->cour;
	loadV = params.cli[nodeVIndex].demand;
	loadY = params.cli[nodeYIndex].demand;
}


void ALNSInital::updataRouteData(RouteInitial* myRoute)
{
    int myplace = 0;
	double myload = 0.;
	double mytime = 0.;
	double myReversalDistance = 0.;	//翻转过来的路径成本
	vvd timeR, etimeR;
	int isStartFeasible = 1;

	myRoute->chromRLS.clear();
	int client_count = 0;
	for (NodeInitial* UI = myRoute->depot->next; !UI->isDepot; UI = UI->next)
	{
		//把顺序更新一下
		client_count++;
		myRoute->chromRLS.push_back(UI->cour);	//放的是点对应的编号
	}
	
	if (client_count != 0)	//如果是空的话
	{
		GetStartTime(params, myRoute->chromRLS, timeR, etimeR, isStartFeasible);	 //获取基本参数信息
		mytime = timeR[timeR.size() - 1];//这个已经保存所有信息了
		myRoute->timeR = timeR;
		myRoute->etimeR = etimeR;
	
		NodeInitial * mynode = myRoute->depot;//首先把depot赋值过去,depot上面全部都是0
		mynode->position = 0;
		mynode->cumulatedLoad = 0.;
		mynode->cumulatedTime = 0.;
		mynode->cumulatedReversalDistance = 0.;
		double myEtime = 0;
		double myLtime = 0;
		bool firstIt = true;	//如果是第一个点的话
		while (!mynode->isDepot || firstIt)	//这个里面进来循环的只有客户点
		{
			//如果不是depot或者是第一个点的话
			if (etimeR[mynode->position] > 0)
				myLtime += etimeR[mynode->position];
			else myEtime -= etimeR[mynode->position];
			mynode = mynode->next;	//这个next和prev相当于会自己切换，更改指针指向
			myplace++;	//1就是代表第一个客户点
			mynode->position = myplace;
			//position是路径中的编号，cour是所有客户点中的编号
			myload += params.cli[mynode->cour].demand;
			//相当于是把两个点反过来距离会有什么变化，这个要不要改变有待商榷
			myReversalDistance += params.timeCost[mynode->cour][mynode->prev->cour] - params.timeCost[mynode->prev->cour][mynode->cour] ;
			mynode->cumulatedLoad = myload;
			mynode->cumulatedTime = timeR[mynode->position];
			mynode->cumulatedReversalDistance = myReversalDistance;
			firstIt = false;	//经历过之后就变成了false
		}

		myLtime += etimeR[etimeR.size() - 1];//最后还要加上一个回去depot可能的延迟
		myRoute->duration = mytime;
		myRoute->load = myload;
		//只要把路径输入之后就会更新
		myRoute->tpenalty = penaltyExcessDuration(myEtime, myLtime);	 //仅仅保存和时间有关的变化 
		myRoute->penalty = penaltyExcessDuration(myEtime, myLtime) + penaltyExcessLoad(myload);
		myRoute->penAndtime = myRoute->tpenalty + myRoute->duration;
		myRoute->nbCustomers = myplace-1;	//说明不到最后一个点
		myRoute->reversalDistance = myReversalDistance;
	}
	else
	{
		myRoute->duration = 0.;
		myRoute->load = 0.;
		myRoute->tpenalty = 0.;
		myRoute->penalty = 0.;
		myRoute->penAndtime = 0.;
		myRoute->nbCustomers = 0;
		myRoute->reversalDistance = 0.;
	}
	//如果是空的话
	//如果是空路径的话，就插入到empty里面去
	if (myRoute->nbCustomers == 0)
		emptyRoutes.insert(myRoute->cour);
	else
	    emptyRoutes.erase(myRoute->cour);		//移除某一个元素
}

void ALNSInital::loadindiv(Individual& indiv)
{
    clients = std::vector< NodeInitial > (params.nbClients+1);
    routes = std::vector< RouteInitial > (params.nbVehicles);
    depots = std::vector < NodeInitial > (params.nbVehicles);
	depotsEnd = std::vector < NodeInitial > (params.nbVehicles);

    for (int i = 0; i <= params.nbClients; i++) 
	{
		clients[i].cour = i; 
		clients[i].isDepot = false; 	//false代表不是，也就是0
	}
	for (int i = 0; i < params.nbVehicles; i++)
	{
		//这边在初始化每一条路径的depot
		routes[i].cour = i;
		routes[i].depot = &depots[i];
		depots[i].cour = 0;		//确实就是用来存储depot的，一条路径的起点
		depots[i].isDepot = true;
		depots[i].route = &routes[i];//.route对应的是一个指针
		depotsEnd[i].cour = 0;		//这个用来存储一条路径的终点
		depotsEnd[i].isDepot = true;
		depotsEnd[i].route = &routes[i];
	}
	//注意这边的编号，客户是从1开始，路径是从0开始，前面提到orderNodes的第一个点是不可以访问的，就如此定义
	for (int i = 1 ; i <= params.nbClients ; i++) orderNodes.push_back(i);
	for (int r = 0 ; r < params.nbVehicles ; r++) orderRoutes.push_back(r);

    for (int r = 0; r < params.nbVehicles; r++)
    {
        NodeInitial * myDepot = &depots[r];//向量，取对应的地址，这些不管空不空都有的
		NodeInitial * myDepotFin = &depotsEnd[r];
		RouteInitial * myRoute = &routes[r];//某一条route
		myDepot->prev = myDepotFin;	//相当于是建立了一个双向链表
		myDepotFin->next = myDepot;//为节点建立前后关系
        routes[r].chromRLS.clear();//先把空间清除
        
        //进来的时候要重新写,首先把第一个元素放进去
        if (r < params.mincar)
        {
            std::uniform_int_distribution<> dis(0,countNotInserted-1);
            int pushIn=dis(params.ran);
            int inner = unsignedOrder[pushIn];
            routes[r].chromRLS.push_back(inner);
            NodeInitial * myClient=&clients[inner];
            myClient->route = myRoute;
			myClient->prev = myDepot;//针对第一个点
			myDepot->next = myClient;
            myClient->next = myDepotFin;//最后一个点也保存了
			myDepotFin->prev = myClient;
            orderNodes.push_back(inner);	//注意orderNodes里面到底要放什么

            unsignedOrder[pushIn]=unsignedOrder[countNotInserted-1];
            unsignedOrder[--countNotInserted]=inner;
        }
        else
        {
            myDepot->next = myDepotFin;
			myDepotFin->prev = myDepot;
        }
        updataRouteData(&routes[r]);
    }
    std::sort(orderNodes.begin(), orderNodes.end());
}

void ALNSInital::exportindiv(Individual& indiv)
{
    //第一个表示客户点,第二个表示路径
    std::vector<std::pair<int,int>> sortOrder;
    for (int r=0;r<params.nbVehicles;r++)
        sortOrder.emplace_back(-routes[r].nbCustomers,r);
    std::sort(sortOrder.begin(),sortOrder.end());

    int pos = 0;	//记录当前一共过了多少个点了
	for (int r = 0; r < params.nbVehicles; r++)
	{
		indiv.chromR[r].clear();
		NodeInitial * node = depots[sortOrder[r].second].next;
		while (!node->isDepot)//当下一个点不是depot的时候
		{
			indiv.chromT[pos] = node->cour;
			indiv.chromR[r].push_back(node->cour);
			node = node->next;	//下一个点
			pos++;
		}
	}//这边相当于是把在LS解释之后的结果，再输出回去
	indiv.evaluateCompleteCost(params);	//对成本还要重新估计
}

void ALNSInital::insertNode(NodeInitial * U, NodeInitial * V)
{
	//这个操作相当于把U从原来的路径中删除掉
	if (U->prev) U->prev->next = U->next;
	if (U->next) U->next->prev = U->prev;
	V->next->prev = U;
	U->prev = V;
	U->next = V->next;
	V->next = U;
	U->route = V->route;	//把U插到V的路径中去
}

void ALNSInital::insertIn(NodeInitial * U, NodeInitial * V)
{
    V->next->prev = U;
    U->prev = V;
    U->next = V->next;
    V->next = U;
    U->route = V->route;
}

//如果是swap的话，就再swap回去
void ALNSInital::swapNode(NodeInitial * U, NodeInitial * V)
{
	NodeInitial * myVPred = V->prev;
	NodeInitial * myVSuiv = V->next;
	NodeInitial * myUPred = U->prev;
	NodeInitial * myUSuiv = U->next;
	RouteInitial * myRouteU = U->route;
	RouteInitial * myRouteV = V->route;

	myUPred->next = V;		//把所有的信息完整的复制过来
	myUSuiv->prev = V;
	myVPred->next = U;
	myVSuiv->prev = U;

	U->prev = myVPred;
	U->next = myVSuiv;
	V->prev = myUPred;
	V->next = myUSuiv;

	U->route = myRouteV;
	V->route = myRouteU;
}

void ALNSInital::getSolution()
{
    //变成特定的路径结构,在load的同时就初始化了
    loadindiv(indiv); 

    while (countNotInserted>0)
    {
        std::uniform_int_distribution<> dis(0,countNotInserted-1);
        int pushIn = dis(params.ran);
        int inner = unsignedOrder[pushIn];
        getBestRoutes(inner);   //插入的点的ID,内部已经完成所有操作
        //把点的顺序更新了
		unsignedOrder[pushIn]=unsignedOrder[countNotInserted-1];
		unsignedOrder[--countNotInserted]=inner;
    }
}

void ALNSInital::run()
{
    this->penaltyCapacityLS = params.penaltyCapacity;
	this->penaltyEDurationLS = params.penaltyEarly;
	this->penaltyLDurationLS = params.penaltyLate;
    getSolution();  //把一个解构造好
    exportindiv(indiv);
}

void ALNSInital::getCost(RouteInitial* myRoute)
{
    GetStartTime(params,myRoute->chromRLS,myRoute->timeR,myRoute->etimeR,myRoute->isStartFeasible);
    if (!myRoute->chromRLS.empty())
    {
        double weight=0;
        for (int i=0;i<(int)myRoute->chromRLS.size();i++)
        {
            weight+=params.cli[myRoute->chromRLS[i]].demand;    //重量要加上去
            if (myRoute->etimeR[i+1]>0)
                myRoute->myLtime+=myRoute->etimeR[i+1];
            else myRoute->myEtime-=myRoute->etimeR[i+1];
        }
        //还需要对最后一个点进行判断
        myRoute->myLtime+=myRoute->etimeR[myRoute->etimeR.size()-1];
        myRoute->duration=myRoute->timeR[myRoute->timeR.size()-1];
        myRoute->load = weight;
        myRoute->tpenalty=penaltyExcessDuration(myRoute->myEtime, myRoute->myLtime);
        myRoute->penalty = penaltyExcessDuration(myRoute->myEtime, myRoute->myLtime) + penaltyExcessLoad(myRoute->load);
		myRoute->penAndtime = myRoute->tpenalty + myRoute->duration;
		myRoute->nbCustomers = myRoute->chromRLS.size();	//说明不到最后一个点
        myRoute->isRouteFeasible=(myRoute->penalty < MY_EPSILON);
    }
    else std::cout<<"客户序列输入错误"<<std::endl;
}

//返回的是点的ID
int ALNSInital::findBestPosition(const int& Node,const int& Route)
{
    int innerID=0;
    double lowestcost=0;
    double cost=0;
    RouteInitial innerRoute = routes[Route];
    vvi innerChromR=innerRoute.chromRLS;
    NodeInitial* innerFirst = innerRoute.depot;
    bool firstIt = true;

    //第一个段也有可能可以插进去
    innerRoute.chromRLS.insert(innerRoute.chromRLS.begin(),Node);
    getCost(&innerRoute);
    innerRoute.chromRLS = innerChromR;    //赋值回去
    lowestcost=innerRoute.penalty+params.timeCost[0][Node]+params.timeCost[Node][innerFirst->next->cour]-params.timeCost[0][innerFirst->next->cour];  //不能这么写,只需要看penalty就好了，还要加上插入的成本
    innerID=0;
    innerFirst=innerFirst->next;
    while (!innerFirst->isDepot)
    {
        innerRoute.chromRLS.insert(innerRoute.chromRLS.begin()+innerFirst->position,Node);
        getCost(&innerRoute);
        cost=innerRoute.penalty+params.timeCost[innerFirst->cour][Node]+params.timeCost[Node][innerFirst->next->cour]-params.timeCost[innerFirst->cour][innerFirst->next->cour];
        innerRoute.chromRLS = innerChromR;    //赋值回去
        if (cost+MY_EPSILON<lowestcost)
        {
            lowestcost=cost;
            innerID=innerFirst->cour;
        }
        innerFirst=innerFirst->next;
    }
    routes[Route].bestCost=lowestcost;

    return innerID;
}

//返回的是点的ID,输入的第一个元素是需要插入的点的ID
void ALNSInital::getBestRoutes(int& Node)
{
    double bestCost=1e10;
    int innerID=0;
    int bestID=0;
    int bestRoute=0;

    for (int i=0;i<minRoute;i++)
    {
        innerID=findBestPosition(Node,i);
        if (routes[i].bestCost+MY_EPSILON<bestCost)
        {
            bestCost=routes[i].bestCost;
            bestID=innerID;
            bestRoute=i;
        }
    }
    //在就把相应的更新了
    nodeU=&clients[Node];
    if (bestID!=0)
        nodeV=&clients[bestID]; //如果是0的话，就代表是depot
    else
        nodeV=&depots[bestRoute];
    setLocalVariablesRouteV();
    insertIn(nodeU,nodeV);  //因为U是空白点进来的
    updataRouteData(&routes[bestRoute]);
}