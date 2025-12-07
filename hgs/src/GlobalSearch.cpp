#include "GlobalSearch.h"


bool GlobalSearch::run(Individual& indiv)
{
    //先把惩罚给赋值了
    this->penaltyCapacityLS = params.penaltyCapacity;
	this->penaltyEDurationLS = params.penaltyEarly;
	this->penaltyLDurationLS = params.penaltyLate;

    //需要有一个失败修复还原操作
    loadIndiv(indiv);  //加载个体
    std::cout<<"这里"<<std::endl;
    findBest();
    std::cout<<"这里1"<<std::endl;
    insertIn();
    std::cout<<"这里2"<<std::endl;
    if (sucessCount>0)  //有一个成功就可以了
    {
        exportindiv(indiv);  //导出个体
        return true;
    }
    else
    {
        return false;
        std::cout<<"恢复失败"<<std::endl;
    }
}

void GlobalSearch::loadIndiv(Individual& indiv)
{
    clients = std::vector< NodeInitial > (params.nbClients+1);  //把这个也变化了
    routes = std::vector< RouteInitial > (params.nbVehicles);
    depots = std::vector < NodeInitial > (params.nbVehicles);
	depotsEnd = std::vector < NodeInitial > (params.nbVehicles);
    solution = std::vector< NodeInitial > (params.nbClients+1);

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

    NodeInitial * first = &clients[indiv.chromT[0]];
    for (int i=0;i<(int)indiv.chromT.size();i++)
    {
        NodeInitial * myClient = &clients[indiv.chromT[i]];
        if (i==(int)indiv.chromT.size()-1)
            myClient->chromNext=first;
        else myClient->chromNext = &clients[indiv.chromT[i+1]];
    }

    for (int r = 0; r < params.nbVehicles; r++)
    {
        NodeInitial * myDepot = &depots[r];//向量，取对应的地址，这些不管空不空都有的
		NodeInitial * myDepotFin = &depotsEnd[r];
		RouteInitial * myRoute = &routes[r];//某一条route
		myDepot->prev = myDepotFin;	//相当于是建立了一个双向链表
		myDepotFin->next = myDepot;//为节点建立前后关系
        routes[r].chromRLS.clear();//先把空间清除
        
        //进来的时候要重新写,首先把第一个元素放进去    
        if (!indiv.chromR[r].empty())
        {
            myDepot->arriveTime=indiv.timeRS[r][0];	//depot的出发时间
            myDepotFin->arriveTime=indiv.timeRS[r][indiv.timeRS[r].size()-1];	//depot的到达时间

            routes[r].chromRLS = indiv.chromR[r];		//这个需要保证正确
			//clients本来就是一个向量，然后相当于获取第一个node的地址给myClient
			//每次的myClient都不一样，保存的第一个点是第一个客户点
			NodeInitial * myClient = &clients[indiv.chromR[r][0]];
            myClient->arriveTime=indiv.timeRS[r][1];	//第一个点的到达时间
            myClient->nodeBelong = true;    //需要给路径赋初值
			myClient->route = myRoute;
			myClient->prev = myDepot;//针对第一个点
			myDepot->next = myClient;
			orderNodes.push_back(indiv.chromR[r][0]);	//第0个点没放进去
            std::cout<<params.timeCost[0][myClient->cour]<<"\t";
			for (int i = 1; i < (int)indiv.chromR[r].size(); i++)
			{
				//把orderNode重新赋值
				orderNodes.push_back(indiv.chromR[r][i]);			//注意加进去的点
				//把所有的前向节点定义了，顺便把前向点的后向点也定义了
				NodeInitial * myClientPred = myClient;
				myClient = &clients[indiv.chromR[r][i]]; //放的是客户点的编号
                myClient->arriveTime=indiv.timeRS[r][i+1];
                myClient->nodeBelong = true;    //需要给路径赋初值
				myClient->prev = myClientPred;
				myClientPred->next = myClient;
				myClient->route = myRoute;
                std::cout<<params.timeCost[myClient->prev->cour][myClient->cour]<<"\t";
			}
			myClient->next = myDepotFin;//最后一个点也保存了
			myDepotFin->prev = myClient;
            std::cout<<params.timeCost[myClient->cour][myClient->next->cour]<<"\t";
            std::cout<<"========================="<<std::endl;
            //用来保存距离减少的值，因为next节点是逐级定义是
            myClient=&clients[indiv.chromR[r][0]];
            myClient->deltaDisD=params.timeCost[0][myClient->cour] + params.timeCost[myClient->cour][myClient->next->cour] - params.timeCost[0][myClient->next->cour];
            for (int i = 1; i < (int)indiv.chromR[r].size(); i++)
            {
                myClient = &clients[indiv.chromR[r][i]];
                myClient->deltaDisD = params.timeCost[myClient->prev->cour][myClient->cour] + params.timeCost[myClient->cour][myClient->next->cour] - params.timeCost[myClient->prev->cour][myClient->next->cour];
            }
		}
		else
		{
			//空路径就直接前后自己赋值
			myDepot->next = myDepotFin;
			myDepotFin->prev = myDepot;
		}
        updataRouteData(&routes[r]);
    }
    std::sort(orderNodes.begin(), orderNodes.end());
}

void GlobalSearch::updataRouteData(RouteInitial* myRoute){
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
    //客户点更新
    myRoute->nbCustomers = client_count;	//客户点的数量
	
	if (client_count != 0)	//如果是空的话
	{
		GetStartTime(params, myRoute->chromRLS, timeR, etimeR, isStartFeasible);	 //获取基本参数信息
		mytime = timeR[timeR.size() - 1];//这个已经保存所有信息了
		myRoute->timeR = timeR;
		myRoute->etimeR = etimeR;
	
		NodeInitial * mynode = myRoute->depot;//首先把depot赋值过去,depot上面全部都是0
        mynode->arriveTime = timeR[0];      //保存一下到达时间
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
            mynode->arriveTime = timeR[mynode->position];	//更新到达时间
			//position是路径中的编号，cour是所有客户点中的编号
			myload += params.cli[mynode->cour].demand;
			//相当于是把两个点反过来距离会有什么变化，这个要不要改变有待商榷
			myReversalDistance += params.timeCost[mynode->cour][mynode->prev->cour] - params.timeCost[mynode->prev->cour][mynode->cour] ;
			mynode->cumulatedLoad = myload;
			mynode->cumulatedTime = timeR[mynode->position];
			mynode->cumulatedReversalDistance = myReversalDistance;
			firstIt = false;	//经历过之后就变成了false
		}
        mynode->arriveTime = timeR[timeR.size()-1];	//最后一个点的到达时间
        myLtime += etimeR[etimeR.size() - 1];//最后还要加上一个回去depot可能的延迟

        //更新一下移除情况
        for (NodeInitial* mynode = myRoute->depot->next; !mynode->isDepot; mynode = mynode->next)
        {
            mynode->deltaDisD = params.timeCost[mynode->prev->cour][mynode->cour]+params.timeCost[mynode->cour][mynode->next->cour]-params.timeCost[mynode->prev->cour][mynode->next->cour];//保存自己被移除的时间
        }
		
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

void GlobalSearch::exportindiv(Individual& indiv)
{
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

double GlobalSearch::getCost(vvi chromRLS)
{
    vvd timeR, etimeR;
    int isStartFeasible = 1;
    double myEtime = 0;
    double myLtime = 0;
    double load = 0;
    double penalty = 0;
    GetStartTime(params,chromRLS,timeR,etimeR,isStartFeasible);
    if (!chromRLS.empty())
    {
        double weight=0;
        for (int i=0;i<(int)chromRLS.size();i++)
        {
            weight+=params.cli[chromRLS[i]].demand;    //重量要加上去
            if (etimeR[i+1]>0)
                myLtime+=etimeR[i+1];
            else myEtime-=etimeR[i+1];
        }
        //还需要对最后一个点进行判断
        myLtime+=etimeR[etimeR.size()-1];
        load = weight;
        penalty = penaltyExcessDuration(myEtime, myLtime) + penaltyExcessLoad(load);
        return penalty;
    }
    else
    {
        return 0.0;
    }
}

void GlobalSearch::findBest()
{
    ejPool.clear();
    std::vector<std::pair<double,int>> ejInner;
    for (int i=0;i<routes.size();i++)
    {
        if (routes[i].nbCustomers>0)
        {
            for (NodeInitial* UI = routes[i].depot->next; !UI->isDepot; UI = UI->next)
            {
                ejInner.emplace_back(UI->deltaDisD, UI->cour); //保存每个点的增量和编号
            }
        }
    }
    std::sort(ejInner.begin(), ejInner.end(),
    [](const std::pair<double, int>& a, const std::pair<double, int>& b) {
        return a.first > b.first; // 从大到小
    });
    for (int i=0;i<params.ap.maxOuter;i++)
    {
        ejPool.push_back(ejInner[i].second); //只取前面maxOuter个点
        initialPos.emplace_back(clients[ejInner[i].second].route->cour,clients[ejInner[i].second].position); //保存初始位置
    }
}

void GlobalSearch::insertIn()
{
    int insertPoint = 0;
    sucessCount = 0; //成功插入的次数
    for (int star=0;star<params.ap.maxOuter;star++)
    {
        insertPoint = ejPool[star]; //从大到小的增量中取出
        double deltaInerst = 0.0; //插入的增量
        double beforeT = 0.0;
        double afterT = 0.0;
        for (int i=0;i<routes.size();i++)
        {
            NodeInitial* UI = routes[i].depot->next;
            if (routes[i].nbCustomers>0 && routes[i].load+params.cli[insertPoint].demand<=params.vehicleCapacity) //如果是空路径就跳过
            {
                while(!UI->isDepot && UI->cour!=insertPoint && UI->next != &clients[insertPoint]) //遍历每一条路径
                {
                    if (UI->position==1)    //如果是当前路径中第一个客户点的话，+10000放进去
                    {
                        deltaInerst = params.timeCost[0][insertPoint] + params.timeCost[insertPoint][UI->cour] - params.timeCost[0][UI->cour];
                        beforeT=UI->prev->arriveTime;
                        afterT=UI->arriveTime;  //这里保存的是自己和前一个的时间，因为第一个点比较特殊
                        if (beforeT+MY_EPSILON<params.cli[insertPoint].timeWindow[0] && 
                            params.cli[insertPoint].timeWindow[1]+MY_EPSILON<afterT && 
                            deltaInerst<clients[insertPoint].deltaDisD)
                        {
                            vvi roadInner=routes[i].chromRLS;
                            roadInner.insert(roadInner.begin(),insertPoint);  //把点放在头部
                            vvd timeR, etimeR;
                            int isStartFeasible = 1;
                            GetStartTime(params, roadInner, timeR, etimeR, isStartFeasible); //更新时间
                            if (isStartFeasible==1)
                            {
                                int routeID=clients[insertPoint].route->cour; //获取对应的路径编号
                                clients[insertPoint].remove();
                                clients[insertPoint].insertBefore(UI->prev);
                                updataRouteData(&routes[i]);	//fitness自己内部就更新进去了
                                updataRouteData(&routes[routeID]);	//更新对应的路径
                                sucessCount++;
                            }
                        }
                    }
                    deltaInerst = params.timeCost[UI->cour][insertPoint] + params.timeCost[insertPoint][UI->next->cour] - params.timeCost[UI->cour][UI->next->cour];
                    beforeT=UI->arriveTime;
                    afterT=UI->next->arriveTime;
                    if (beforeT+MY_EPSILON<params.cli[insertPoint].timeWindow[0] && 
                        params.cli[insertPoint].timeWindow[1]+MY_EPSILON<afterT && 
                        deltaInerst<clients[insertPoint].deltaDisD)
                    {
                        vvi roadInner=routes[i].chromRLS;
                        roadInner.insert(roadInner.begin()+UI->position,insertPoint);  //放到UI之后
                        vvd timeR, etimeR;
                        int isStartFeasible = 1;
                        GetStartTime(params, roadInner, timeR, etimeR, isStartFeasible); //更新时间
                        if (isStartFeasible==1)
                        {
                            int routeID=clients[insertPoint].route->cour; //获取对应的路径编号
                            clients[insertPoint].remove();
                            clients[insertPoint].insertIn(UI->prev);
                            updataRouteData(&routes[i]);	//fitness自己内部就更新进去了
                            updataRouteData(&routes[routeID]);	//更新对应的路径
                            sucessCount++;
                        }
                    }
                    UI = UI->next; //下一个点
                }
            }
        }
    }  
}