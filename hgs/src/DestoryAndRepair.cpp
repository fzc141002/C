#include "DestoryAndRepair.h"

void DestoryAndRepair::updataRouteData(RouteInitial* myRoute)
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
    //客户点更新
    myRoute->nbCustomers = client_count;	//客户点的数量
	
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

void DestoryAndRepair::loadIndiv(Individual& indiv)     //需要在初始化里面把归属加上
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
            routes[r].chromRLS = indiv.chromR[r];		//这个需要保证正确
			//clients本来就是一个向量，然后相当于获取第一个node的地址给myClient
			//每次的myClient都不一样，保存的第一个点是第一个客户点
			NodeInitial * myClient = &clients[indiv.chromR[r][0]];
            myClient->nodeBelong = true;    //需要给路径赋初值
			myClient->route = myRoute;
			myClient->prev = myDepot;//针对第一个点
			myDepot->next = myClient;
			orderNodes.push_back(indiv.chromR[r][0]);	//第0个点没放进去
			for (int i = 1; i < (int)indiv.chromR[r].size(); i++)
			{
				//把orderNode重新赋值
				orderNodes.push_back(indiv.chromR[r][i]);			//注意加进去的点
				//把所有的前向节点定义了，顺便把前向点的后向点也定义了
				NodeInitial * myClientPred = myClient;
				myClient = &clients[indiv.chromR[r][i]]; //放的是客户点的编号
                myClient->nodeBelong = true;    //需要给路径赋初值
				myClient->prev = myClientPred;
				myClientPred->next = myClient;
				myClient->route = myRoute;
			}
			myClient->next = myDepotFin;//最后一个点也保存了
			myDepotFin->prev = myClient;
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


void DestoryAndRepair::setOrder()
{
    int inner;  //辅助变量
    for (int i=0;i<countCandidates;i++)
    {
        std::uniform_int_distribution<> dis(0,countCandidates-1);
        int indexA=dis(params.ran);
        int indexB=dis(params.ran);

        inner=candidates[indexA];
        candidates[indexA]=candidates[indexB];
        candidates[indexB]=inner;
    }
}

//相当于是解被初始化了
void DestoryAndRepair::setSolution(Individual& indiv)
{
    for (int i=0;i<indiv.eval.nbRoutes;i++)     //？？？？
    {
        routes[i].modified=false;
        routes[i].depot->modified=false;
    }

    //获得恢复相关的参数
    std::uniform_int_distribution<> dis(0, insertionHeuristics.size() - 1);
    indexHeuristic = dis(params.ran); // indexHeuristic 就是一个随机下标
    selectedInsertionHeuristic = insertionHeuristics[indexHeuristic];

    //获得扰动相关的参数，邻域的大小范围需要动态调整
    //注意如果对象中是包含引用的话,不能直接复制
    //如果对于一个复杂的类对象需要拷贝的时候
    //暂时先定为一个确定的数
    countCandidates=0;
}

int DestoryAndRepair::getNode(int node)
{
    switch (selectedInsertionHeuristic)
    {
    case InsertionHeuristic::Distance: return getBestKNN(node,1);
    case InsertionHeuristic::Cost: return getBestKNN(node,params.ap.varphi);   //是一个动态调整的值
    }
    return -1;
}

double DestoryAndRepair::getCost(vvi chromRLS)
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

//确定不同的邻近插入
int DestoryAndRepair::getBestKNN(int node,int limit)
{
    bool flag=false;
    int count=0;
    bestcost=1e10;
    bestNode=NULL;  //初始化一下
    int routeBest=-1;

    for (int i=0;i<(int)params.correlatedVertices[node].size() && count<limit;i++)
    {
        if (params.correlatedVertices[node][i]==0)
        {
            //就需要遍历每一条路径的头部的位置插进去
            for (int j=0;j<params.nbVehicles;j++)
            {
                if (routes[j].nbCustomers!=0)
                {
                    NodeInitial* inner=routes[j].depot;
                    flag=true;
                    vvi chrominner=routes[j].chromRLS;
                    chrominner.insert(chrominner.begin(),node);  //把点放在头部
                    cost=getCost(chrominner); //更新成本
                    cost+=params.timeCost[0][node]+params.timeCost[node][inner->next->cour]-params.timeCost[0][inner->next->cour];
                    if (cost<bestcost)
                    {
                        bestcost=cost;
                        bestNode=inner;
                        routeBest=j;
                    }
                }
                if (flag) count++;  //用来限制寻找次数
            }
        }
        else
        {
            //因为是从这里获取的
            NodeInitial* inner=&clients[params.correlatedVertices[node][i]];
            if (inner->nodeBelong)   //如果说在路径之中
            {
                count++;
                vvi chrominner=inner->route->chromRLS;
                chrominner.insert(chrominner.begin()+inner->position,node);  //把点放在头部
                cost=getCost(chrominner); //更新成本
                cost+=params.timeCost[inner->cour][node]+params.timeCost[node][inner->next->cour]-params.timeCost[inner->cour][inner->next->cour];
                if (cost<bestcost)
                {
                    bestcost=cost;
                    bestNode=inner;
                }
            }
        }
    }
    if (bestNode==NULL) //就需要遍历所有可能的点，寻找插入位置
    {
        for (int i=0;i<params.nbClients;i++)
        {
            NodeInitial* inner=&clients[i+1]; //这边就不考虑depot了
            if (inner->nodeBelong)
            {
                vvi chrominner=inner->route->chromRLS;
                chrominner.insert(chrominner.begin()+inner->position,node);  //把点放在头部
                cost=getCost(chrominner); //更新成本
                cost+=params.timeCost[inner->cour][node]+params.timeCost[node][inner->next->cour]-params.timeCost[inner->cour][inner->next->cour];
                if (cost<bestcost)
                {
                    bestcost=cost;
                    bestNode=&clients[i+1];
                }
            }
        }
    }
    vvi innerChromR= bestNode->route->chromRLS;
    innerChromR.insert(innerChromR.begin()+bestNode->position,node);  //把点放在头部
    cost=getCost(innerChromR); //更新成本
    cost+=params.timeCost[bestNode->cour][node]+params.timeCost[node][bestNode->next->cour]-params.timeCost[bestNode->cour][bestNode->next->cour];
    if (routeBest==-1)  //如果是depot的话
    {
        innerChromR = bestNode->route->chromRLS;
        innerChromR.insert(innerChromR.begin()+bestNode->prev->position,node);  //把点放在头部
        costprev=getCost(innerChromR); //更新成本
        costprev+=bestNode->route->penalty+params.timeCost[bestNode->prev->cour][node]+params.timeCost[node][bestNode->cour]-params.timeCost[bestNode->prev->cour][bestNode->cour];
    }
    else
        costprev=1e10;  //就不可能放到之前
    if (cost<=costprev)     //这边包含一个两个价值相当的时候，插在bestNode后面
        return routeBest!=-1?routeBest+10000:bestNode->cour;  //返回对应的编号，10000为了说明是depot
    else
    {
        if (bestNode->prev->cour==0)
            return bestNode->route->cour+10000;    //说明是depot的情况
        else 
            return bestNode->prev->cour;    //这边也会存在depot的情况
    }
}

void DestoryAndRepair::addCandidates()
{
    for (int i=0;i<countCandidates;i++)
    {
        int inner=candidates[i];
        //确确实实需要插入这个点
        int bestNodeP=getNode(inner); //获取最优的插入位置，如果是depot的话，不知道是哪条路径
        if (bestNodeP>=10000)
        {
            bestNodeP-=10000; //说明是depot,对应那一条路径
            //需要插到这条路径的第一个点
            NodeInitial* innerDepot=routes[bestNodeP].depot->next;
            clients[inner].insertBefore(innerDepot);
        }
        else
            clients[inner].insertIn(&clients[bestNodeP]);
        updataRouteData(&routes[clients[inner].route->cour]);	//fitness自己内部就更新进去了
    }
}

void DestoryAndRepair::sequentialDestory(Individual& indiv)
{
    setSolution(indiv);

    int contSizeString=0;
    double sizeString=0.;
    NodeInitial* initialNode;

    while (countCandidates<(int)omega)      //控制的是删除集合的大小
    {
        sizeString=std::min(std::max(1,params.nbClients),(int)omega-countCandidates);

        std::uniform_int_distribution<> dis(1,params.nbClients);
        int nextNode = dis(params.ran); // 随机选择一个客户点
        NodeInitial* inner=&clients[nextNode];
        while(!inner->nodeBelong) // 确保选择的点是属于某个路径的
        {
            nextNode = dis(params.ran);
            inner = &clients[nextNode];
        }
        initialNode=inner; // 保存初始节点信息

        contSizeString=0;
        do
        {
            contSizeString++;
            NodeInitial * myNode = inner->chromNext;
            inner=myNode;

            candidates[countCandidates++]=inner->cour;   //这里面保存的是点的编号

            inner->prevOld=inner->prev;
            inner->nextOld=inner->next;

            RouteInitial* myRoute = inner->route;    //只需要这样就可以了
            inner->remove(); //移除这个点
            updataRouteData(myRoute);
        } 
        while (initialNode->cour != inner->cour && contSizeString < sizeString);
    }
    setOrder();
    addCandidates();
}

void DestoryAndRepair::concentricDestory(Individual& indiv)
{
    setSolution(indiv);
    //每次进来调用的时候都是恢复好了的
    std::uniform_int_distribution<> dis(1,params.nbClients);    //因为clients里面也包含depot
    int nextNode = dis(params.ran); // 随机选择一个客户点
    NodeInitial* reference=&clients[nextNode];
    for (int i=0;i<omega && i<(int)params.correlatedVertices[reference->cour].size() && countCandidates<omega;i++)
    {
        if (params.correlatedVertices[reference->cour][i]!=0)    //如果不是depot的话
        {
            NodeInitial* inner=&clients[params.correlatedVertices[reference->cour][i]];
            candidates[countCandidates++]=inner->cour;
            inner->prevOld=inner->prev;
            inner->nextOld=inner->next;

            RouteInitial* myRoute = inner->route;    //只需要这样就可以了
            inner->remove(); //移除这个点
            updataRouteData(myRoute);
        }
    }
    setOrder();
    addCandidates();    //扰动之后顺带更新完了
}

void DestoryAndRepair::exportindiv(Individual& indiv)
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

void DestoryAndRepair::run(Individual& indiv)
{
    //随机选择一种破坏方式
    loadIndiv(indiv);  //加载个体
    std::uniform_int_distribution<> dis(0, perturbationTypes.size() - 1);
    int indexPerturbation = dis(params.ran); // indexHeuristic 就是一个随机下标
    selectedperturbationType = perturbationTypes[indexPerturbation];

    //需要进入赋值
    this->penaltyCapacityLS = params.penaltyCapacity;
	this->penaltyEDurationLS = params.penaltyEarly;
	this->penaltyLDurationLS = params.penaltyLate;

    switch (selectedperturbationType)
    {
    case PerturbationType::Sequential:
        sequentialDestory(indiv);
        break;
    case PerturbationType::Concentric:
        concentricDestory(indiv);
        break;
    }
    exportindiv(indiv);  //导出个体
}