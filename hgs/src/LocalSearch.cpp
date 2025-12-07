#include "LocalSearch.h" 


//相当于一个局部搜索的开始函数
void LocalSearch::run(Individual& indiv, double penaltyCapacityLS, double penaltyEDurationLS, double penaltyLDurationLS,double penalthRouteLS,int sym,std::vector<int>& ejectionPool)
{
	//对于一个个体进行局部搜索
	//惩罚从外面放进来的
	this->penaltyCapacityLS = penaltyCapacityLS;
	this->penaltyEDurationLS = penaltyEDurationLS;
	this->penaltyLDurationLS = penaltyLDurationLS;
	this->penalthRouteLS = penalthRouteLS;
	loadIndividual(indiv);
	// Shuffling the order of the nodes explored by the LS to allow for more diversity in the search
	//对于节点的顺序重新探索
	std::shuffle(orderNodes.begin(), orderNodes.end(), params.ran);
	std::shuffle(orderRoutes.begin(), orderRoutes.end(), params.ran);		//这个对于swap*来说的
	for (int i = 1; i <= params.nbClients; i++)
		if (params.ran() % params.ap.nbGranular == 0)  // O(n/nbGranular) calls to the inner function on average, to achieve linear-time complexity overall,临近点的顺序也可以打乱
			std::shuffle(params.correlatedVertices[i].begin(), params.correlatedVertices[i].end(), params.ran);

	searchCompleted = false;
	//写一个记录变化的案例
	for (loopID = 0; !searchCompleted; loopID++)
	{
		if (loopID > 1) // Allows at least two loops since some moves involving empty routes are not checked at the first loop
			searchCompleted = true;
		if (loopID > 50)
		{
			std::cout<<"Loop ID: " << loopID << "超过50次，退出循环"<<std::endl;
			break;
		}

		/* CLASSICAL ROUTE IMPROVEMENT (RI) MOVES SUBJECT TO A PROXIMITY RESTRICTION */
		//所有客户点
		for (int posU = 0; posU < indiv.cliNum; posU++)
		{
			//相当于对于每个点来说的
			nodeU = &clients[orderNodes[posU]];	//对应客户点的地址,oderNodes里面保存的是需要被ls的客户点
			//保存上一个路径的移动次数
			int lastTestRINodeU = nodeU->whenLastTestedRI;
			nodeU->whenLastTestedRI = nbMoves;
			//这个邻居的规模要不要考虑缩减一点？？？		
			for (int posV = 0; posV < (int)params.correlatedVertices[nodeU->cour].size(); posV++)
			{
				nodeV = &clients[params.correlatedVertices[nodeU->cour][posV]];
				//这个要遍历的有点多，这样子遍历的少一点
				if (std::find(ejectionPool.begin(), ejectionPool.end(), nodeV->cour) == ejectionPool.end()) // nodeV在当前的路径里面
				{
					if (loopID == 0 || std::max<int>(nodeU->route->whenLastModified, nodeV->route->whenLastModified) > lastTestRINodeU) // only evaluate moves involving routes that have been modified since last move evaluations for nodeU，如果说发生了修改，就执行移动评价
					{
						// Randomizing the order of the neighborhoods within this loop does not matter much as we are already randomizing the order of the node pairs (and it's not very common to find improving moves of different types for the same node pair)
						setLocalVariablesRouteU();//把前后节点初始化
						setLocalVariablesRouteV();
						if (move1(0)) {continue;} // RELOCATE			
						if (move2(0)) {continue;} // RELOCATE		
						if (move3(0)) {continue;} // RELOCATE			
						if (nodeUIndex <= nodeVIndex && move4(0)) {continue;} // SWAP			
						if (move5(0)) {continue;} // SWAP		
						if (nodeUIndex <= nodeVIndex && move6(0)) {continue;} // SWAP		
						if (intraRouteMove && move7(0)) {continue;} // 2-OPT			
						if (!intraRouteMove && move8(0)) {continue;} // 2-OPT*	
						if (!intraRouteMove && move9(0)) {continue;} // 2-OPT*	
						// Trying moves that insert nodeU directly after the depot
						if (nodeV->prev->isDepot)
						{
							nodeV = nodeV->prev;		//这个用完之后就会丢掉的
							setLocalVariablesRouteV();	//把V重新定位一下
							setLocalVariablesRouteV();	//把V重新定位一下
							if (move1(0)) {continue;} // RELOCATE
							if (move2(0)) {continue;} // RELOCATE
							if (move3(0)) {continue;} // RELOCATE
							if (!intraRouteMove && move8(0)) {continue;} // 2-OPT*
							if (!intraRouteMove && move9(0)) {continue;} // 2-OPT*
						}
					}
				}
			}
			//可以和空路径进行交换,这个在后面感觉不需要，因为要尽可能减少车辆数量
			/* 不一定是可行解的时候，不过拓展新路径需要考虑增加的成本 */
			if (loopID > 0 && !emptyRoutes.empty() && sym==0)	//还有空路径
			{
				nodeV = routes[*emptyRoutes.begin()].depot;//如果V是空路径的起点的话
				setLocalVariablesRouteU();
				setLocalVariablesRouteV();
				if (move1(penalthRouteLS)) {continue;} // RELOCATE
				if (move2(penalthRouteLS)) {continue;} // RELOCATE
				if (move3(penalthRouteLS)) {continue;} // RELOCATE
				if (move9(penalthRouteLS)) {continue;} // 2-OPT*
			}
		}
		if (params.ap.useSwapStar == 1 && params.areCoordinatesProvided)
		{
			/* (SWAP*) MOVES LIMITED TO ROUTE PAIRS WHOSE CIRCLE SECTORS OVERLAP */
			//只对路径在弧形范围内有交叉的进行检查
			for (int rU = 0; rU < params.nbVehicles; rU++)
			{
				routeU = &routes[orderRoutes[rU]];
				int lastTestSWAPStarRouteU = routeU->whenLastTestedSWAPStar;
				routeU->whenLastTestedSWAPStar = nbMoves;
				for (int rV = 0; rV < params.nbVehicles; rV++)
				{
					routeV = &routes[orderRoutes[rV]];
					if (routeU->nbCustomers > 0 && routeV->nbCustomers > 0 && routeU->cour < routeV->cour
						&& (loopID == 0 || std::max<int>(routeU->whenLastModified, routeV->whenLastModified)
							> lastTestSWAPStarRouteU))
						if (CircleSector::overlap(routeU->sector, routeV->sector))
						{
							swapStar();
						}
							
				}
			}
		}
	}
	// Register the solution produced by the LS in the individual
	exportIndividual(indiv);
}

//下面这两个函数是在为路线设置局部变量
void LocalSearch::setLocalVariablesRouteU()
{
	routeU = nodeU->route;	//这是一个赋值操作
	nodeX = nodeU->next;	//定义一下nodeX是什么
	nodeXNextIndex = nodeX->next->cour;
	nodeUIndex = nodeU->cour;
	nodeUPrevIndex = nodeU->prev->cour;
	nodeXIndex = nodeX->cour;
	loadU    = params.cli[nodeUIndex].demand;
	serviceU = params.cli[nodeUIndex].serviceDuration;
	loadX	 = params.cli[nodeXIndex].demand;
	serviceX = params.cli[nodeXIndex].serviceDuration;
}

void LocalSearch::setLocalVariablesRouteV()
{
	routeV = nodeV->route;
	nodeY = nodeV->next;	//无法保证nodeY不是depot
	nodeYNextIndex = nodeY->next->cour;
	nodeVIndex = nodeV->cour;
	nodeVPrevIndex = nodeV->prev->cour;
	nodeYIndex = nodeY->cour;
	loadV    = params.cli[nodeVIndex].demand;
	serviceV = params.cli[nodeVIndex].serviceDuration;
	loadY	 = params.cli[nodeYIndex].demand;
	serviceY = params.cli[nodeYIndex].serviceDuration;
	intraRouteMove = (routeU == routeV);//是否为同一路径
}

void printroute(vvi& r)
{
	if (r.empty()) {std::cout<<"空路径"<<std::endl; return;}
	else
	{
		for (int i=0;i<(int)r.size();i++)
		{
			std::cout<<r[i]<<"  ";
		}
	}
}


bool LocalSearch::move1(int routePen)
{
	if (nodeUIndex == nodeYIndex) return false;
	// if (params.deleteEdges.count({nodeV->cour,nodeU->cour}) || params.deleteEdges.count({nodeU->cour,nodeY->cour})) return false;
	//分别表示U前面点的ID，U后面点的ID（X），表示移除U的成本
	double costSuppU = params.timeCost[nodeUPrevIndex][nodeXIndex] - params.timeCost[nodeUPrevIndex][nodeUIndex] - params.timeCost[nodeUIndex][nodeXIndex];
	double costSuppV = params.timeCost[nodeVIndex][nodeUIndex] + params.timeCost[nodeUIndex][nodeYIndex] - params.timeCost[nodeVIndex][nodeYIndex]+routePen;

	//如果是跨路径的话，需要考虑额外的成本
	//其实不管是不是一条路径，好像都需要考虑时间窗的变化
	// Early move pruning to save CPU time. Guarantees that this move cannot improve without checking additional (load, duration...) constraints
	//如果光交换的成本就大于原来的惩罚成本了，就不可行了
	//删除U插入路径V中,V之后
	int uNode = nodeU->position-1;
	int vNode = nodeV->position-1;
	int	uNumber = nodeU->cour; 
	vvi	uroute = routeU->chromRLS;
	vvi vroute = routeV->chromRLS;
	vvd upen,vpen;
	uroute.erase(uroute.begin() + uNode);	//注意是位置，而不是对应的元素
	if (routeU->cour==routeV->cour)	//如果是同一条路径
	{
		if (vNode < uNode) uroute.insert(uroute.begin() + vNode + 1, uNumber);
		else uroute.insert(uroute.begin() + vNode, uNumber);
		upen=getPenalty(uroute);
		vpen = upen; // Same penalty for both routes
		//因为一条路径，所以最终直选哦考虑
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load)- routeU->penalty;
	}
	else
	{
		if (costSuppU + costSuppV >= routeU->penalty + routeV->penalty) return false;
		vroute.insert(vroute.begin() + vNode + 1, uNumber);
		if (uroute.size()>0)
			upen = getPenalty(uroute);
		else
		{
			costSuppU -= penalthRouteLS; //空路径就要减少一定的成本
			upen = vvd(1,(0,0));	//修改的原因是防止出现空路径
		}
		vpen = getPenalty(vroute);
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load - loadU) - routeU->penalty;
		costSuppV += vpen[0] + penaltyExcessLoad(routeV->load + loadU) - routeV->penalty;
	}
	//恢复回去,nodeX->prev就是原本nodeU的prev
	//这个相当于是交换之后成本高了，就不交换
	if (costSuppU + costSuppV > -MIN_F)  return false;
	//如果说U和Y是同一个节点，本身U就位于V的后面，交换无意义;
	insertNode(nodeU, nodeV);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (!intraRouteMove) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move2(int routePen)
{
	if (nodeU == nodeY || nodeV == nodeX || nodeX->isDepot) return false;
	// if (params.deleteEdges.count({nodeV->cour,nodeU->cour}) || params.deleteEdges.count({nodeX->cour,nodeY->cour})) return false;
	//同时把U和X都移走了，但是U和X之间距离还是客观存在的
	double costSuppU = params.timeCost[nodeUPrevIndex][nodeXNextIndex] - params.timeCost[nodeUPrevIndex][nodeUIndex] - params.timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params.timeCost[nodeVIndex][nodeUIndex] + params.timeCost[nodeXIndex][nodeYIndex] - params.timeCost[nodeVIndex][nodeYIndex]+routePen;
	//两个元素怎么处理
	int uNode = nodeU->position-1;
	int xNode = nodeX->position-1;
	int vNode = nodeV->position-1;
	int	uNumber = nodeU->cour;
	int xNumber = nodeX->cour;
	vvi	uroute = routeU->chromRLS;
	vvi vroute = routeV->chromRLS;
	vvd upen, vpen;
	uroute.erase(uroute.begin() + xNode);
	uroute.erase(uroute.begin() + uNode);
	if (routeU->cour==routeV->cour)
	{
		if (xNode<vNode) 
		{
			uroute.insert(uroute.begin()+vNode-1, uNumber);
			uroute.insert(uroute.begin()+vNode, xNumber);
		}
		else
		{
			uroute.insert(uroute.begin()+vNode+1, uNumber);
			uroute.insert(uroute.begin()+vNode+2, xNumber);
		}
		upen = getPenalty(uroute);
		vpen = upen;
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load) - routeU->penalty;
	}
	else
	{
		if (costSuppU + costSuppV >= routeU->penalty + routeV->penalty) return false;
		vroute.insert(vroute.begin() + vNode + 1, uNumber);
		vroute.insert(vroute.begin() + vNode + 2, xNumber);
		if (uroute.size() > 0)
			upen = getPenalty(uroute);
		else
		{
			costSuppU -= penalthRouteLS;
			upen = vvd(1,(0,0));
		}
		vpen = getPenalty(vroute);
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load - loadU - loadX) - routeU->penalty;
		costSuppV += vpen[0] + penaltyExcessLoad(routeV->load + loadU + loadX) - routeV->penalty;
	}
	//恢复
	if (costSuppU + costSuppV > -MIN_F) return false;
	insertNode(nodeU, nodeV);
	insertNode(nodeX, nodeU);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (!intraRouteMove) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move3(int routePen)
{
	if (nodeU == nodeY || nodeX == nodeV || nodeX->isDepot) return false;
	// if (params.deleteEdges.count({nodeV->cour,nodeX->cour}) || params.deleteEdges.count({nodeU->cour,nodeY->cour})) return false;
	//U和X交换位置插入到V之后
	double costSuppU = params.timeCost[nodeUPrevIndex][nodeXNextIndex] - params.timeCost[nodeUPrevIndex][nodeUIndex] - params.timeCost[nodeUIndex][nodeXIndex] - params.timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params.timeCost[nodeVIndex][nodeXIndex] + params.timeCost[nodeXIndex][nodeUIndex] + params.timeCost[nodeUIndex][nodeYIndex] - params.timeCost[nodeVIndex][nodeYIndex]+routePen;

	// Early move pruning to save CPU time. Guarantees that this move cannot improve without checking additional (load, duration...) constraints
	int uNode = nodeU->position-1;
	int xNode = nodeX->position-1;
	int vNode = nodeV->position-1;
	int	uNumber = nodeU->cour;
	int xNumber = nodeX->cour;
	vvi	uroute = routeU->chromRLS;
	vvi vroute = routeV->chromRLS;
	//删了一个之后，后面往前移，所以还是uNode的位置
	vvd upen,vpen;
	uroute.erase(uroute.begin() + xNode);
	uroute.erase(uroute.begin() + uNode);
	if (routeU->cour == routeV->cour)
	{
		if (xNode < vNode)
		{
			uroute.insert(uroute.begin() + vNode - 1, xNumber);
			uroute.insert(uroute.begin() + vNode, uNumber);
		}
		else
		{
			uroute.insert(uroute.begin() + vNode + 1, xNumber);
			uroute.insert(uroute.begin() + vNode + 2, uNumber);
		}
		upen = getPenalty(uroute);
		vpen = upen;
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load) - routeU->penalty;
	}
	else
	{
		if (costSuppU + costSuppV >= routeU->penalty + routeV->penalty) return false;
		vroute.insert(vroute.begin() + vNode + 1, xNumber);
		vroute.insert(vroute.begin() + vNode + 2, uNumber);
		if (uroute.size() > 0)
			upen = getPenalty(uroute);
		else
		{
			costSuppU -= penalthRouteLS;
			upen = vvd(1,(0,0));
		}
		vpen = getPenalty(vroute);
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load - loadU - loadX) - routeU->penalty;
		costSuppV += vpen[0] + penaltyExcessLoad(routeV->load + loadU + loadX) - routeV->penalty;
	}

	if (costSuppU + costSuppV > -MIN_F) return false;

	insertNode(nodeX, nodeV);
	insertNode(nodeU, nodeX);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (!intraRouteMove) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move4(int routePen)
{
	// if (params.deleteEdges.count({nodeU->prev->cour,nodeV->cour}) || params.deleteEdges.count({nodeV->prev->cour,nodeU->cour})) return false;
	// if (params.deleteEdges.count({nodeV->cour,nodeU->next->cour}) || params.deleteEdges.count({nodeU->cour,nodeV->next->cour})) return false;
	//把u和v互换位置
	double costSuppU = params.timeCost[nodeUPrevIndex][nodeVIndex] + params.timeCost[nodeVIndex][nodeXIndex] - params.timeCost[nodeUPrevIndex][nodeUIndex] - params.timeCost[nodeUIndex][nodeXIndex];
	double costSuppV = params.timeCost[nodeVPrevIndex][nodeUIndex] + params.timeCost[nodeUIndex][nodeYIndex] - params.timeCost[nodeVPrevIndex][nodeVIndex] - params.timeCost[nodeVIndex][nodeYIndex];

	// Early move pruning to save CPU time. Guarantees that this move cannot improve without checking additional (load, duration...) constraints
	int uNode = nodeU->position-1;
	int vNode = nodeV->position-1;
	vvi uroute = routeU->chromRLS;
	vvi vroute = routeV->chromRLS;
	vvd upen, vpen;
	if (routeU->cour==routeV->cour)
	{
		std::swap(uroute[uNode],uroute[vNode]);
		upen = getPenalty(uroute);
		vpen = upen;
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load) - routeU->penalty;
	}
	else
	{
		if (costSuppU + costSuppV >= routeU->penalty + routeV->penalty) return false;
		std::swap(uroute[uNode], vroute[vNode]);
		upen = getPenalty(uroute);
		vpen = getPenalty(vroute);
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load + loadV - loadU) - routeU->penalty;
		costSuppV += vpen[0] + penaltyExcessLoad(routeV->load + loadU - loadV) - routeV->penalty;
	}
	
	if (costSuppU + costSuppV > -MIN_F) return false;
	if (nodeUIndex == nodeVPrevIndex || nodeUIndex == nodeYIndex) return false;
	swapNode(nodeU, nodeV);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (!intraRouteMove) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move5(int routePen)
{
	// if (params.deleteEdges.count({nodeU->prev->cour,nodeV->cour}) || params.deleteEdges.count({nodeV->cour,nodeX->next->cour})) return false;
	// if (params.deleteEdges.count({nodeV->prev->cour,nodeU->cour}) || params.deleteEdges.count({nodeX->cour,nodeY->cour})) return false;
	if (nodeU == nodeV->prev || nodeX == nodeV->prev || nodeU == nodeY || nodeX->isDepot)
		return false;
	//相当于是把V一个点，换了U和X的两个点
	double costSuppU = params.timeCost[nodeUPrevIndex][nodeVIndex] + params.timeCost[nodeVIndex][nodeXNextIndex] - params.timeCost[nodeUPrevIndex][nodeUIndex] - params.timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params.timeCost[nodeVPrevIndex][nodeUIndex] + params.timeCost[nodeXIndex][nodeYIndex] - params.timeCost[nodeVPrevIndex][nodeVIndex] - params.timeCost[nodeVIndex][nodeYIndex];

	// Early move pruning to save CPU time. Guarantees that this move cannot improve without checking additional (load, duration...) constraints
	
	int uNode = nodeU->position-1;
	int xNode = nodeX->position-1;
	int vNode = nodeV->position-1;
	int xNumber = nodeX->cour;
	vvi	uroute = routeU->chromRLS;
	vvi vroute = routeV->chromRLS;
	vvd upen, vpen;
	if (routeU->cour==routeV->cour)
	{
		std::swap(uroute[uNode], uroute[vNode]);
		uroute.erase(uroute.begin() + xNode);
		if (xNode < vNode) uroute.insert(uroute.begin() + vNode, xNumber);
		else uroute.insert(uroute.begin() + vNode+1, xNumber);
		upen = getPenalty(uroute);
		vpen = upen; // Same penalty for both routes
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load) - routeU->penalty;
	}
	else
	{
		if (costSuppU + costSuppV >= routeU->penalty + routeV->penalty) return false;
		std::swap(uroute[uNode], vroute[vNode]);
		uroute.erase(uroute.begin() + xNode);
		vroute.insert(vroute.begin() + vNode + 1, xNumber);
		vvd upen = getPenalty(uroute);
		vvd vpen = getPenalty(vroute);
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load + loadV - loadU - loadX) - routeU->penalty;
		costSuppV += vpen[0] + penaltyExcessLoad(routeV->load + loadU + loadX - loadV) - routeV->penalty;
	}
	if (costSuppU + costSuppV > -MIN_F) return false;

	swapNode(nodeU, nodeV);
	insertNode(nodeX, nodeU);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (!intraRouteMove) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move6(int routePen)
{
	if (nodeX->isDepot || nodeY->isDepot || nodeY == nodeU->prev || nodeU == nodeY || nodeX == nodeV || nodeV == nodeX->next)
		return false;
	//ux和vy四个点顺序不换的交换位置
	double costSuppU = params.timeCost[nodeUPrevIndex][nodeVIndex] + params.timeCost[nodeYIndex][nodeXNextIndex] - params.timeCost[nodeUPrevIndex][nodeUIndex] - params.timeCost[nodeXIndex][nodeXNextIndex];
	double costSuppV = params.timeCost[nodeVPrevIndex][nodeUIndex] + params.timeCost[nodeXIndex][nodeYNextIndex] - params.timeCost[nodeVPrevIndex][nodeVIndex] - params.timeCost[nodeYIndex][nodeYNextIndex];

	// Early move pruning to save CPU time. Guarantees that this move cannot improve without checking additional (load, duration...) constraints
	
	int uNode = nodeU->position-1;
	int xNode = nodeX->position-1;
	int vNode = nodeV->position-1;
	int yNode = nodeY->position-1;
	vvi	uroute = routeU->chromRLS;
	vvi vroute = routeV->chromRLS;
	vvd upen,vpen;
	if (routeU->cour==routeV->cour)
	{
		std::swap(uroute[uNode], uroute[vNode]);
		std::swap(uroute[xNode], uroute[yNode]);
		upen = getPenalty(uroute);
		vpen = upen;
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load) - routeU->penalty;
	}
	else
	{
		if (costSuppU + costSuppV >= routeU->penalty + routeV->penalty) return false;
		std::swap(uroute[uNode], vroute[vNode]);
		std::swap(uroute[xNode], vroute[yNode]);
		upen = getPenalty(uroute);
		vpen = getPenalty(vroute);
		costSuppU += upen[0] + penaltyExcessLoad(routeU->load + loadV + loadY - loadU - loadX) - routeU->penalty;
		costSuppV += vpen[0] + penaltyExcessLoad(routeV->load + loadU + loadX - loadV - loadY) - routeV->penalty;
	}
	
	if (costSuppU + costSuppV > -MIN_F) return false;

	swapNode(nodeU, nodeV);
	swapNode(nodeX, nodeY);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	if (!intraRouteMove) updateRouteData(routeV);
	return true;
}

bool LocalSearch::move7(int routePen)
{
	//因为后续是按照顺序检查的，for循环，如果说不加这个if后面会重复一遍,取2的原因是相隔两个点的话情况和进交换U和V的情况相同
	if (nodeU->position > nodeV->position || nodeV->position-nodeX->position <= 2) return false;
	//就是把两点互换之后的距离变化，互换的距离相当于是从这个点开始到depot的距离增加量，然后相减的话，就是最后的增加变化情况，一般是多的减少的
	//X和V中间的每一个点都会变的

	double penALL = routeU->penAndtime;	//变换之前的总成本
	vvi routeOLD = routeU->chromRLS;
	int changeX = nodeX->position-1;
	int changeV = nodeV->position-1;
	vvi routeNew;
	int routeSize = (int)routeOLD.size();
	for (int i = 0; i < routeSize; i++)
	{
		if (i >= changeX && i <= changeV)
		{
			//逆序放入
			routeNew.push_back(routeOLD[changeV-i+changeX]);
		}
		else routeNew.push_back(routeOLD[i]);
	}
	vvd upen = getPenalty(routeNew);
	double cost = upen[0] + upen[1] - penALL;	//move7情况下的load是不会变的
	if (cost > -MIN_F) return false;

	NodeInitial * nodeNum = nodeX->next;
	nodeX->prev = nodeNum;
	nodeX->next = nodeY;

	while (nodeNum != nodeV)
	{
		//这个相当于是在两个点互换
		NodeInitial * temp = nodeNum->next;
		nodeNum->next = nodeNum->prev;
		nodeNum->prev = temp;
		nodeNum = temp;	//这里面一直在地址传递
	}

	nodeV->next = nodeV->prev;
	nodeV->prev = nodeU;
	nodeU->next = nodeV;
	nodeY->prev = nodeX;

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	return true;
}

bool LocalSearch::move8(int routePen)
{
	if(nodeX->isDepot || nodeY->isDepot) return false;
	//路径变化成本确实是这样的
	double cost = params.timeCost[nodeUIndex][nodeVIndex] + params.timeCost[nodeXIndex][nodeYIndex] - params.timeCost[nodeUIndex][nodeXIndex] - params.timeCost[nodeVIndex][nodeYIndex]
		+ nodeV->cumulatedReversalDistance + routeU->reversalDistance - nodeX->cumulatedReversalDistance
		- routeU->penalty - routeV->penalty;

	// Early move pruning to save CPU time. Guarantees that this move cannot improve without checking additional (load, duration...) constraints
	if (cost >= 0) return false;
	vvi urouteOLD = routeU->chromRLS;		//但是这个里面是没有保存depot的
	vvi vrouteOLD = routeV->chromRLS;
	int changeX = nodeX->position-1;	//注意position里面是从depot开始计数的
	int changeV = nodeV->position-1;
	vvi urouteNew(urouteOLD.begin(), urouteOLD.begin() + changeX);  //注意是左闭右开的
	vvi vrouteNew(vrouteOLD.begin() + changeV + 1, vrouteOLD.end());//end本身就是指向下一个元素
	vvi innerU, innerV;	//临时获取的
	int sizeU = (int)urouteOLD.size();
	int sizeV = (int)vrouteOLD.size();
	for (int i = sizeU - 1; i >= changeX; i--)
	{
		innerU.push_back(urouteOLD[i]);//指U变换出去的
	}
	for (int i = changeV; i >= 0; i--)
	{
		innerV.push_back(vrouteOLD[i]);
	}
	urouteNew.insert(urouteNew.end(), innerV.begin(), innerV.end());
	innerU.insert(innerU.end(), vrouteNew.begin(), vrouteNew.end());
	vvd upen = getPenalty(urouteNew);
	vvd vpen = getPenalty(innerU);

	cost += upen[0] + vpen[0] + penaltyExcessLoad(nodeU->cumulatedLoad + nodeV->cumulatedLoad) + penaltyExcessLoad(routeU->load + routeV->load - nodeU->cumulatedLoad - nodeV->cumulatedLoad);
	if (cost > -MIN_F) return false;

	NodeInitial * depotU = routeU->depot;
	NodeInitial * depotV = routeV->depot;
	NodeInitial * depotUFin = routeU->depot->prev;
	NodeInitial * depotVFin = routeV->depot->prev;
	NodeInitial * depotVSuiv = depotV->next;

	NodeInitial * temp;
	NodeInitial * xx = nodeX;
	NodeInitial * vv = nodeV;

	while (!xx->isDepot)
	{
		temp = xx->next;
		xx->next = xx->prev;
		xx->prev = temp;
		xx->route = routeV;
		xx = temp;
	}

	while (!vv->isDepot)
	{
		temp = vv->prev;
		vv->prev = vv->next;
		vv->next = temp;
		vv->route = routeU;
		vv = temp;
	}

	nodeU->next = nodeV;
	nodeV->prev = nodeU;
	nodeX->next = nodeY;
	nodeY->prev = nodeX;

	if (nodeX->isDepot)
	{
		depotUFin->next = depotU;
		depotUFin->prev = depotVSuiv;
		depotUFin->prev->next = depotUFin;
		depotV->next = nodeY;
		nodeY->prev = depotV;
	}
	else if (nodeV->isDepot)
	{
		depotV->next = depotUFin->prev;
		depotV->next->prev = depotV;
		depotV->prev = depotVFin;
		depotUFin->prev = nodeU;
		nodeU->next = depotUFin;
	}
	else
	{
		depotV->next = depotUFin->prev;
		depotV->next->prev = depotV;
		depotUFin->prev = depotVSuiv;
		depotUFin->prev->next = depotUFin;
	}

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	updateRouteData(routeV);
	return true;
}

bool LocalSearch::move9(int routePen)
{
	//相当于是把x和y后面的所有点顺序都交换了，上下交换
	if (nodeY->isDepot || nodeX->isDepot) return false;
	double cost = params.timeCost[nodeUIndex][nodeYIndex] + params.timeCost[nodeVIndex][nodeXIndex] - params.timeCost[nodeUIndex][nodeXIndex] - params.timeCost[nodeVIndex][nodeYIndex]
		        - routeU->penalty - routeV->penalty + routePen;

	// Early move pruning to save CPU time. Guarantees that this move cannot improve without checking additional (load, duration...) constraints
	if (cost >= 0) return false;

	vvi vrouteOLD = routeV->chromRLS;		//注意这个是V
	vvi urouteOLD = routeU->chromRLS;
	int changeX = nodeX->position-1;
	int changeY = nodeY->position-1;
	vvi urouteNew(urouteOLD.begin(), urouteOLD.begin() + changeX);
	vvi vrouteNew(vrouteOLD.begin(), vrouteOLD.begin() + changeY);
	vvi innerU, innerV;	//临时获取的
	int sizeU = (int)urouteOLD.size();
	int sizeV = (int)vrouteOLD.size();
	for (int i = changeX; i < sizeU; i++)
	{
		innerU.push_back(urouteOLD[i]);
	}
	for (int i = changeY; i < sizeV; i++)
	{
		innerV.push_back(vrouteOLD[i]);
	}
	urouteNew.insert(urouteNew.end(), innerV.begin(), innerV.end());
	vrouteNew.insert(vrouteNew.end(), innerU.begin(), innerU.end());
	vvd upen = getPenalty(urouteNew);
	vvd vpen = getPenalty(vrouteNew);

	cost += upen[0] + vpen[0] + penaltyExcessLoad(nodeU->cumulatedLoad + routeV->load - nodeV->cumulatedLoad)
		+ penaltyExcessLoad(nodeV->cumulatedLoad + routeU->load - nodeU->cumulatedLoad);
	if (cost > -MIN_F) return false;

	NodeInitial * depotU = routeU->depot;
	NodeInitial * depotV = routeV->depot;
	NodeInitial * depotUFin = depotU->prev;
	NodeInitial * depotVFin = depotV->prev;
	NodeInitial * depotUpred = depotUFin->prev;

	NodeInitial * count = nodeY;
	while (!count->isDepot)
	{
		count->route = routeU;
		count = count->next;
	}

	count = nodeX;
	while (!count->isDepot)
	{
		count->route = routeV;
		count = count->next;
	}

	nodeU->next = nodeY;
	nodeY->prev = nodeU;
	nodeV->next = nodeX;
	nodeX->prev = nodeV;

	if (nodeX->isDepot)
	{
		depotUFin->prev = depotVFin->prev;
		depotUFin->prev->next = depotUFin;
		nodeV->next = depotVFin;
		depotVFin->prev = nodeV;
	}
	else
	{
		depotUFin->prev = depotVFin->prev;
		depotUFin->prev->next = depotUFin;
		depotVFin->prev = depotUpred;
		depotVFin->prev->next = depotVFin;
	}

	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	updateRouteData(routeV);
	return true;
}

//没看懂，具体关注一下怎么用吧
bool LocalSearch::swapStar()
{
	SwapStarElement myBestSwapStar;	//存储交换的信息
	// Preprocessing insertion costs
	//[routeV][U]，哪个在后面，前面就是哪个
	preprocessInsertions(routeU, routeV);
	preprocessInsertions(routeV, routeU);
	// Evaluating the moves
	//这里是将两条路径上的点逐个交换
	//这边的UXVY专门是给swap*用的
	for (nodeU = routeU->depot->next; !nodeU->isDepot; nodeU = nodeU->next)
	{
		for (nodeV = routeV->depot->next; !nodeV->isDepot; nodeV = nodeV->next)
		{
			//这个里面已经减去了原先的路径惩罚了
			double deltaPenRouteU = penaltyExcessLoad(routeU->load + params.cli[nodeV->cour].demand - params.cli[nodeU->cour].demand) - routeU->penalty;	 //交换后的和当前的惩罚做差
			double deltaPenRouteV = penaltyExcessLoad(routeV->load + params.cli[nodeU->cour].demand - params.cli[nodeV->cour].demand) - routeV->penalty;

			// Quick filter: possibly early elimination of many SWAP* due to the capacity constraints/penalties and bounds on insertion costs
			//这边的if判断里面把所有的都包括进去了
			if (deltaPenRouteU + nodeU->deltaRemoval + deltaPenRouteV + nodeV->deltaRemoval <= 0)
			{
				//构建一个内部临时的变量
				SwapStarElement mySwapStar;
				mySwapStar.U = nodeU;
				mySwapStar.V = nodeV;

				// Evaluate best reinsertion cost of U in the route of V where V has been removed
				//评估在已移除节点 V 的路径中，节点 U 最佳重新插入的成本，会给这个位置赋值
				double extraV = getCheapestInsertSimultRemoval(nodeU, nodeV, mySwapStar.bestPositionU);

				// Evaluate best reinsertion cost of V in the route of U where U has been removed
				double extraU = getCheapestInsertSimultRemoval(nodeV, nodeU, mySwapStar.bestPositionV);

				// Evaluating final cost，计算交换后的总成本
				//这个成本包括deltaPenRouteU装载量U和V交换，deltaRemoval移除U的成本，extraU就是把V插入U的额外成本，时间成本都在外面计算了

				//这个是之前的成本，保存的是之前的时间惩罚
				int uNode = nodeU->position-1;
				int vNode = nodeV->position-1;
				int uNumber = nodeU->cour;
				int vNumber = nodeV->cour;
				vvi uroute = routeU->chromRLS;
				vvi vroute = routeV->chromRLS;
				vroute.insert(vroute.begin() + mySwapStar.bestPositionU->position, uNumber);	//在V的路径中插入U
				uroute.insert(uroute.begin() + mySwapStar.bestPositionV->position, vNumber);	//在U的路径中插入V
				uroute.erase(std::remove(uroute.begin(),uroute.end(),uNumber));	//删除U
				vroute.erase(std::remove(vroute.begin(),vroute.end(),vNumber));	//删除V
				vvd upen = getPenalty(uroute);
				vvd vpen = getPenalty(vroute);
				
				//应该是不用减去之间的时间惩罚了，因为上面已经在总的惩罚里面减掉了
				//载重变化情况，第二个是删除一个点的成本，第三个是插入一个点的成本
				mySwapStar.moveCost = deltaPenRouteU + nodeU->deltaRemoval + extraU + deltaPenRouteV + nodeV->deltaRemoval + extraV + upen[0]+vpen[0];

				if (mySwapStar.moveCost < myBestSwapStar.moveCost)
					myBestSwapStar = mySwapStar;
			}
		}
	}
	//U对于V的重定向，U需要插入到V里面
	// Including RELOCATE from nodeU towards routeV (costs nothing to include in the evaluation at this step since we already have the best insertion location)
	// Moreover, since the granularity criterion粒度不同 is different, this can lead to different improving moves
	for (nodeU = routeU->depot->next; !nodeU->isDepot; nodeU = nodeU->next)
	{
		SwapStarElement mySwapStar;
		mySwapStar.U = nodeU;
		mySwapStar.bestPositionU = bestInsertClient[routeV->cour][nodeU->cour].bestLocation[0];
		//这个是路径U删除节点U之后的成本变化
		double deltaDistRouteU = params.timeCost[nodeU->prev->cour][nodeU->next->cour] - params.timeCost[nodeU->prev->cour][nodeU->cour] - params.timeCost[nodeU->cour][nodeU->next->cour];
		//这个成本是U插入路径V的最小成本
		double deltaDistRouteV = bestInsertClient[routeV->cour][nodeU->cour].bestCost[0];

		int uNode = nodeU->position-1;
		int uNumber = nodeU->cour;
		vvi uroute = routeU->chromRLS;
		vvi vroute = routeV->chromRLS;
		uroute.erase(uroute.begin() + uNode);	//删除
		vroute.insert(vroute.begin() + mySwapStar.bestPositionU->position, uNumber);
		vvd upen;
		if (uroute.size() > 0)
			upen = getPenalty(uroute);
		else
			upen = vvd(1,(0,0));
		vvd vpen = getPenalty(vroute);
		//deltaDistRouteU为U移除之后的路径变化成本
		//后面第二部分为U移除之后装载量变化成本
		
		mySwapStar.moveCost = deltaDistRouteU + deltaDistRouteV
			+ penaltyExcessLoad(routeU->load - params.cli[nodeU->cour].demand) - routeU->penalty
			+ penaltyExcessLoad(routeV->load + params.cli[nodeU->cour].demand) - routeV->penalty
			+ upen[0] + vpen[0];
			//这边也不用减去之间了，因为已经在总的penalty里减掉了

		if (mySwapStar.moveCost < myBestSwapStar.moveCost)
			myBestSwapStar = mySwapStar;
	}
	// Including RELOCATE from nodeV towards routeU
	for (nodeV = routeV->depot->next; !nodeV->isDepot; nodeV = nodeV->next)
	{
		SwapStarElement mySwapStar;
		mySwapStar.V = nodeV;
		mySwapStar.bestPositionV = bestInsertClient[routeU->cour][nodeV->cour].bestLocation[0];
		double deltaDistRouteU = bestInsertClient[routeU->cour][nodeV->cour].bestCost[0];
		double deltaDistRouteV = params.timeCost[nodeV->prev->cour][nodeV->next->cour] - params.timeCost[nodeV->prev->cour][nodeV->cour] - params.timeCost[nodeV->cour][nodeV->next->cour];

		//删除和恢复
		int vNode = nodeV->position-1;
		int vNumber = nodeV->cour;
		vvi uroute = routeU->chromRLS;
		vvi vroute = routeV->chromRLS;
		vroute.erase(vroute.begin() + vNode);	//删除
		uroute.insert(uroute.begin() + mySwapStar.bestPositionV->position, vNumber);
		vvd upen = getPenalty(uroute);
		vvd vpen;
		if (vroute.size() > 0)
			vpen = getPenalty(vroute);
		else
			vpen = vvd(1,(0,0));

		mySwapStar.moveCost = deltaDistRouteU + deltaDistRouteV
			+ penaltyExcessLoad(routeU->load + params.cli[nodeV->cour].demand) - routeU->penalty
			+ penaltyExcessLoad(routeV->load - params.cli[nodeV->cour].demand) - routeV->penalty
			+ upen[0] + vpen[0];
		
		if (mySwapStar.moveCost < myBestSwapStar.moveCost)
			myBestSwapStar = mySwapStar;
	}
	if (myBestSwapStar.moveCost > -MIN_F) return false;
	if (myBestSwapStar.bestPositionU != NULL) insertNode(myBestSwapStar.U, myBestSwapStar.bestPositionU);
	if (myBestSwapStar.bestPositionV != NULL) insertNode(myBestSwapStar.V, myBestSwapStar.bestPositionV);
	nbMoves++; // Increment move counter before updating route data
	searchCompleted = false;
	updateRouteData(routeU);
	updateRouteData(routeV);
	return true;
}

//这几个成本函数后面再来看，U在V中的成本
double LocalSearch::getCheapestInsertSimultRemoval(NodeInitial * U, NodeInitial * V, NodeInitial *& bestPosition)
{
	//获得是移除节点 V 的情况下，找出将节点 U 插入到节点 V 所在路线的最便宜（成本最低）的插入位置，并返回对应的插入成本
	//这边是获取最优的插入信息,[路径][需要插入的点]
	ThreeBestInsert * myBestInsert = &bestInsertClient[V->route->cour][U->cour];
	bool found = false;//found用来检查插入位置是否是V或者是V的前一个节点

	// Find best insertion in the route such that V is not next or pred (can only belong to the top three locations)
	bestPosition = myBestInsert->bestLocation[0];
	double bestCost = myBestInsert->bestCost[0];
	found = (bestPosition != V && bestPosition->next != V);
	if (!found && myBestInsert->bestLocation[1] != NULL)
	{
		bestPosition = myBestInsert->bestLocation[1];
		bestCost = myBestInsert->bestCost[1];
		found = (bestPosition != V && bestPosition->next != V);
		if (!found && myBestInsert->bestLocation[2] != NULL)
		{
			bestPosition = myBestInsert->bestLocation[2];
			bestCost = myBestInsert->bestCost[2];
			found = true;
		}
	}//一层层往里找合适地位置

	// Compute insertion in the place of V，拆在V
	//delta是把V删掉之后插入U的成本减去仅删掉V不插入U的成本
	double deltaCost = params.timeCost[V->prev->cour][U->cour] + params.timeCost[U->cour][V->next->cour] - params.timeCost[V->prev->cour][V->next->cour];
	if (!found || deltaCost < bestCost)
	{
		bestPosition = V->prev;
		bestCost = deltaCost;
	}
	return bestCost;
}

//这个函数看不懂，假装要插进去，看一下成本变化情况
//只需要考虑路径变化前后的时间成本，装载成本在别的地方考虑
void LocalSearch::preprocessInsertions(RouteInitial * R1, RouteInitial * R2)
{
	//从路径R1中移除每个节点的成本，以及将每个节点插入R2的位置和成本
	//其实上述函数里面提到的时间，就是对应的全局成功移动次数
	//找前三个插入位置的时候，需要遍历所有可能的插入位置
	for (NodeInitial * U = R1->depot->next; !U->isDepot; U = U->next)
	{
		// Performs the preprocessing
		// 如果插入成本小于0则代表缩短路径
		
		//只需要考虑交换之后的时间变化成本
		U->deltaRemoval = params.timeCost[U->prev->cour][U->next->cour] - params.timeCost[U->prev->cour][U->cour] - params.timeCost[U->cour][U->next->cour];
		//计算移去U的成本，相当于是三角形
		//if语句主要是判断是否要重新计算U插入路线2中的成本
		//对于每一个route和每一个客户点而言的
		//对应的移动次数被理解为是时间点
		//这个if语句可以理解为R2在上一次U插入之后又动过了，所以需要更新，否则就不需要更新
		if (R2->whenLastModified > bestInsertClient[R2->cour][U->cour].whenLastCalculated)
		{
			//whenlastmodified是和路径有关的，在updata的时候会把nbmoves给它
			//每一个元素都是ThreeBestInsert
			bestInsertClient[R2->cour][U->cour].reset();
			bestInsertClient[R2->cour][U->cour].whenLastCalculated = nbMoves;
			//一开始默认先加入一个元素，放到depot后面
			//bestcost里面就已经包含了时长的所有变化了
			bestInsertClient[R2->cour][U->cour].bestCost[0] = params.timeCost[0][U->cour] + params.timeCost[U->cour][R2->depot->next->cour] - params.timeCost[0][R2->depot->next->cour];
			bestInsertClient[R2->cour][U->cour].bestLocation[0] = R2->depot;
			//上面保存的是插入到起始位置的成本
			for (NodeInitial * V = R2->depot->next; !V->isDepot; V = V->next)
			{
				//然后开始遍历V中的每一个位置
				double deltaCost = params.timeCost[V->cour][U->cour] + params.timeCost[U->cour][V->next->cour] - params.timeCost[V->cour][V->next->cour];
				bestInsertClient[R2->cour][U->cour].compareAndAdd(deltaCost, V);
			}
		}
	}
}

void LocalSearch::insertNode(NodeInitial * U, NodeInitial * V)
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

//如果是swap的话，就再swap回去
void LocalSearch::swapNode(NodeInitial * U, NodeInitial * V)
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

vvd LocalSearch::getPenalty(const vvi& chromInner)
{
	vvd timeR, etimeR;
	vvd result;
	int isStartFeasible = 1;
	GetStartTime(params, chromInner, timeR, etimeR, isStartFeasible);
	//只需要算时间的变化情况就可以了
	double pen_in = 0.;
	double eptime = 0.;
	double lptime = 0.;
	for (int i = 0; i < (int)etimeR.size(); i++)
	{
		if (etimeR[i] <= 0) eptime -= etimeR[i];
		else lptime += etimeR[i];
	}
	pen_in = penaltyExcessDuration(eptime, lptime);
	result.push_back(pen_in);
	result.push_back(timeR[timeR.size() - 1]);
	return result;	//先是惩罚，再是总时间
}

//更新路径数据，把route更新了
//输入的需要是一条完整的myRoute
void LocalSearch::updateRouteData(RouteInitial * myRoute)
{
	int myplace = 0;
	double myload = 0.;
	double mytime = 0.;
	double myReversalDistance = 0.;	//翻转过来的路径成本
	double cumulatedX = 0.;
	double cumulatedY = 0.;
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
			if (!mynode->isDepot)
			{
				cumulatedX += params.cli[mynode->cour].coordX;
				cumulatedY += params.cli[mynode->cour].coordY;
				if (firstIt) myRoute->sector.initialize(params.cli[mynode->cour].polarAngle);
				else myRoute->sector.extend(params.cli[mynode->cour].polarAngle);
			}
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
		// Remember "when" this route has been last modified (will be used to filter unnecessary move evaluations)
		myRoute->whenLastModified = nbMoves ;		//保存一下移动次数
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
		myRoute->whenLastModified = nbMoves;	//保存一下移动次数
	}
	//如果是空的话
	//如果是空路径的话，就插入到empty里面去
	if (myRoute->nbCustomers == 0)
	{
		myRoute->polarAngleBarycenter = 1.e30;
		emptyRoutes.insert(myRoute->cour);
	}
	else
	{
		//这个公式暂时先不修改了
		myRoute->polarAngleBarycenter = atan2(cumulatedY/(double)myRoute->nbCustomers - params.cli[0].coordY, cumulatedX/(double)myRoute->nbCustomers - params.cli[0].coordX);
		emptyRoutes.erase(myRoute->cour);		//移除某一个元素
	}
}

//把indiv的结果载入进来，需要把一些参数初始化
//把指针给过去之后，对应的值也会发生变化
//其实点之间是用链表连接的，其相对于路径来说比较割裂
void LocalSearch::loadIndividual(const Individual & indiv)
{
	orderNodes.clear();
	emptyRoutes.clear();		//初始一个空路径集合
	nbMoves = 0; 
	for (int r = 0; r < params.nbVehicles; r++)		//第r辆车
	{
		//相当于是在建立一个路径的链表,首尾相连
		//对于depot来说每一条路径只有一个
		NodeInitial * myDepot = &depots[r];//向量，取对应的地址，这些不管空不空都有的
		NodeInitial * myDepotFin = &depotsEnd[r];
		RouteInitial * myRoute = &routes[r];//某一条route
		myDepot->prev = myDepotFin;	//相当于是建立了一个双向链表
		myDepotFin->next = myDepot;//为节点建立前后关系
		//建立仓库的前后关系
		routes[r].chromRLS.clear();//先把空间清除
		if (!indiv.chromR[r].empty())//如果这一条路径不空
		{
			//把这条路径上的客户点摘出来
			routes[r].chromRLS = indiv.chromR[r];		//这个需要保证正确
			//clients本来就是一个向量，然后相当于获取第一个node的地址给myClient
			//每次的myClient都不一样，保存的第一个点是第一个客户点
			NodeInitial * myClient = &clients[indiv.chromR[r][0]];
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
		//我其实就想要路径
		updateRouteData(&routes[r]);
		routes[r].whenLastTestedSWAPStar = -1;
		for (int i = 1; i <= params.nbClients; i++) // Initializing memory structures
			bestInsertClient[r][i].whenLastCalculated = -1;
	}
	std::sort(orderNodes.begin(), orderNodes.end());	//排序一下，恢复初始状态
	for (int i = 1; i <= params.nbClients; i++) // Initializing memory structures
		clients[i].whenLastTestedRI = -1;
}

void LocalSearch::checkIndividual(const Individual & indiv)
{
	std::vector<int> cusRecord;
	std::vector<int> routeRecord;
	int carNum=0;
	int carRoute=0;
	for (int r=0;r<params.nbVehicles;r++)
	{
		if (!indiv.chromR[r].empty())
		{
			carNum++;
			for (int i:indiv.chromR[r])
			{
				if (!cusRecord.empty() && std::find(cusRecord.begin(),cusRecord.end(),i)==cusRecord.end())
				{
					cusRecord.push_back(i);	//把i元素放进去
				}
				if (cusRecord.empty()) cusRecord.push_back(i);
			}
		}
		RouteInitial* innerR=&routes[r];
		if (innerR->nbCustomers>0)
		{
			carRoute++;
			int xunhuan=0;
			for (NodeInitial* innerU=innerR->depot->next;!innerU->isDepot;innerU=innerU->next)
			{
				xunhuan++;
				if (xunhuan>20) {std::cout<<"死循环了"<<xunhuan<<std::endl;}
				if (!routeRecord.empty() && std::find(routeRecord.begin(),routeRecord.end(),innerU->cour)==routeRecord.end())
				{
					routeRecord.push_back(innerU->cour);
				}
				if (routeRecord.empty()) routeRecord.push_back(innerU->cour);
			}
		}
	}
	if (cusRecord.size()!=indiv.cliNum) {std::cout<<"客户点路径中数量不对"<<cusRecord.size()<<"\t"<<indiv.cliNum<<std::endl;std::exit;}
	if (routeRecord.size()!=indiv.cliNum) {std::cout<<"load之后路径中数量不对"<<routeRecord.size()<<"\t"<<indiv.cliNum<<std::endl;std::exit;}
	// if (carNum!=indiv.eval.nbRoutes) {std::cout<<"车辆数1统计的不对"<<carNum<<"\t"<<indiv.eval.nbRoutes<<std::endl;std::exit;}
	// if (carRoute!=indiv.eval.nbRoutes) {std::cout<<"车辆数2统计的不对"<<carRoute<<"\t"<<indiv.eval.nbRoutes<<std::endl;std::exit;}
}

void LocalSearch::exportIndividual(Individual & indiv)
{
	std::vector < std::pair <double, int> > routePolarAngles ;
	for (int r = 0; r < params.nbVehicles; r++)
		routePolarAngles.push_back(std::pair <double, int>(routes[r].polarAngleBarycenter, r)); //保存第r条路径的中心极角
	std::sort(routePolarAngles.begin(), routePolarAngles.end()); // empty routes have a polar angle of 1.e30, and therefore will always appear at the end，空路径的中心极角是很大的，从小到大排序

	int pos = 0;	//记录当前一共过了多少个点了
	for (int r = 0; r < params.nbVehicles; r++)
	{
		indiv.chromR[r].clear();
		NodeInitial * node = depots[routePolarAngles[r].second].next;
		while (!node->isDepot)//当下一个点不是depot的时候
		{
			indiv.chromT[pos] = node->cour;
			indiv.chromR[r].push_back(node->cour);
			node = node->next;	//下一个点
			pos++;
		}
	}//这边相当于是把在LS解释之后的结果，再输出回去
	// indiv.cliNum=indiv.chromT.size();  	//把个体数也重新更新了
	indiv.evaluateCompleteCost(params);	//对成本还要重新估计
}

LocalSearch::LocalSearch(Params & params) : params (params)
{
	//clients里面本来就保存了客户点,为了防止客户点中间有间隙
	clients = std::vector < NodeInitial >(params.nbClients + 1);//这边多一个点的意义应该是让客户从1开始计数
	routes = std::vector < RouteInitial >(params.nbVehicles);
	depots = std::vector < NodeInitial >(params.nbVehicles);
	depotsEnd = std::vector < NodeInitial >(params.nbVehicles);
	bestInsertClient = std::vector < std::vector <ThreeBestInsert> >(params.nbVehicles, std::vector <ThreeBestInsert>(params.nbClients + 1));	   //外层是车的数量，内层是客户点+depot

	//所以说clients里面是包含第一个depot的,从数量角度上看
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
}