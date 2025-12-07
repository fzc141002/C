#include "GES.h"

//尝试把车辆缩减到极致
void GES::run(Individual& indiv,int gesTime)
{
    std::cout<<"进入缩减"<<std::endl;
    int innerTime=0;
    inner_indiv = indiv;   //可以理解为一个深拷贝
    Individual innercopyIndiv = inner_indiv;
    initial();  //基本参数初始化
    while (innerTime<gesTime)
    {
        std::cout<<"第"<<innerTime<<"次缩减"<<std::endl;
        ejectionPool.clear();   //每次进来弹出池初始化一下
        // destroy1(inner_indiv);
        //随机破坏
        // destroy2(inner_indiv);
        // 选择装载率最低的破坏
        destoryChoose();    //随机选择一个破坏
        inner_indiv.evaluateCompleteCost(params);   //更新一下可行性，删除完了之后的个体了
        updataIndiv();           //把一些内置的参数更新了
        clock_t innerStrartTime = clock();  //GES开始循环的时间
        int gesNum = 0;
        copy_indiv = inner_indiv;
        while(!ejectionPool.empty() && (double)(clock()-innerStrartTime)/(double)CLOCKS_PER_SEC < params.ap.GES_timeLimit)
        {
            int insertID = 0;       //表示弹出池中某个元素的下标
            bool easyInsert = false;
            if (gesNum == 0)        //如果第一次进来，这个条件不对
            {
                std::uniform_int_distribution<> dis(0, ejectionPool.size()-1);
                insertID = dis(params.ran);
            }
            else
                insertID = ejectionPool.size()-1; //后进先出，获得最后一个元素,注意需要获得的是最后一个元素的下标
            gesNum++;       //用来标志不是第一个
            //注意第一个元素是元素本身，第二个元素是位置
            //这边考虑插入的时候可以设置一个允许插入的池子大小，看看效果
            easyInsert = insertCustomer(ejectionPool[insertID],insertID);   //看看有没有插入成功，插入的时候顺带更新了
            if (!easyInsert)
            {
                easyInsert = squeeze(ejectionPool[insertID],insertID);  //如果说最外面不可行的话，就要squeeze随机插入
                if (!easyInsert)
                {
                    //进行弹出组合
                    vvi bestjection;
                    //参数分别是插入的路径ID，插入的点在ejctionPool里面的ID，以及内部弹出池，还有判断插入的点是否被弹出
                    eject(insertLoseRoute);    //把节点弹出
                }
            } 
        }
        if (ejectionPool.empty())
        {
            insertIs=true;
            innercopyIndiv=inner_indiv;
            std::cout<<"内部缩减车辆数"<<inner_indiv.eval.nbRoutes<<std::endl;
            std::cout<<"当前成本为"<<inner_indiv.eval.penalizedCost<<std::endl;
            if (inner_indiv.eval.nbRoutes==params.mincar)
                break;
        }
        else
        {
            insertIs=false;
            inner_indiv=innercopyIndiv; //需要恢复回去
            if (inner_indiv.eval.nbRoutes==params.mincar)
                break;
            std::cout<<"缩减失败"<<inner_indiv.eval.nbRoutes<<std::endl;
            std::cout<<"当前成本为"<<inner_indiv.eval.penalizedCost<<std::endl;
        }    
        destroyUp();    //更新权重
        innerTime++;    //缩减了一次之后就会增加次数
    }
    indiv=inner_indiv;   //更新当前个体,这里可能最后还是要再更新一下
    exportIndivGes(indiv);    //导出的时候要全部都更新一下
    //如果有缩减的话，进一步更新
    std::cout<<"退出缩减"<<std::endl;
}

void GES::exportIndivGes(Individual& indiv)
{
	int pos = 0;	//记录当前一共过了多少个点了
	for (int r = 0; r < params.nbVehicles; r++)
	{
		if (!indiv.chromR[r].empty())
		for (int i:indiv.chromR[r])
        {
            indiv.chromT[pos] = i;
            pos++;
        }
	}//这边相当于是把在LS解释之后的结果，再输出回去
	indiv.evaluateCompleteCost(params);	//对成本还要重新估计
}

//随机选择一辆车破坏
void GES::destroy1(Individual& indiv)
{
    if (indiv.eval.nbRoutes <= 1) return;   //如果只有一辆车的话就不需要弹出
    std::uniform_int_distribution<> dis(0, indiv.eval.nbRoutes - 1);
    int routeID = dis(params.ran);
    ejectionPool.insert(ejectionPool.end(),indiv.chromR[routeID].begin(),indiv.chromR[routeID].end());
    //插入之后原来的解要被破坏
    penaltyCounter=std::vector<int>(params.nbClients+1,1);    //初始化计数器
    //不能直接删除
    indiv.chromR.erase(indiv.chromR.begin() + routeID);
    indiv.chromR.push_back(std::vector<int>());
}

//选择装载率低的破坏
void GES::destroy2(Individual& indiv)
{
    if (indiv.eval.nbRoutes <= 1) return;   //如果只有一辆车的话就不需要弹出
    double minLoad = 1.e30;
    int minRoute = -1;
    for (int i = 0; i < params.nbVehicles; i++)
    {
        if (!indiv.chromR[i].empty())
        {
            double load = 0;
            for (int j = 0; j < indiv.chromR[i].size(); j++)
            {
                load += params.cli[indiv.chromR[i][j]].demand;
            }
            if (load < minLoad)
            {
                minLoad = load;
                minRoute = i;
            }
        }
    }
    ejectionPool.insert(ejectionPool.end(),indiv.chromR[minRoute].begin(),indiv.chromR[minRoute].end());
    penaltyCounter=std::vector<int>(params.nbClients+1,1);    //初始化计数器
    indiv.chromR.erase(indiv.chromR.begin() + minRoute);
    indiv.chromR.push_back(std::vector<int>());
}

void GES::destroy3(Individual& indiv)
{
    if (indiv.eval.nbRoutes <= 1) return;   //如果只有一辆车的话就不需要弹出
    double minCus = 1.e30;
    int minRoute = -1;
    for (int i = 0; i < params.nbVehicles; i++)
    {
        if (!indiv.chromR[i].empty())
        {
            double cus = indiv.chromR[i].size();
            if (cus < minCus)
            {
                minCus = cus;
                minRoute = i;
            }
        }
    }
    ejectionPool.insert(ejectionPool.end(),indiv.chromR[minRoute].begin(),indiv.chromR[minRoute].end());
    penaltyCounter=std::vector<int>(params.nbClients+1,1);    //初始化计数器
    indiv.chromR.erase(indiv.chromR.begin() + minRoute);
    indiv.chromR.push_back(std::vector<int>());
}

bool GES::squeeze(int rIn,int pos)
{
    //选择在载重率最小的一个插入
    double minLoad = 1.e30;
    int insertID = -1;
    for (int i = 0; i < params.nbVehicles; i++)
    {
        if (!inner_indiv.chromR[i].empty())
        {
            double load = 0;
            for (int j = 0; j < inner_indiv.chromR[i].size(); j++)
            {
                load += params.cli[inner_indiv.chromR[i][j]].demand;
            }
            if (load < minLoad)
            {
                minLoad = load;
                insertID = i;
            }
        }
    }
    std::uniform_int_distribution<> dis2(0, inner_indiv.chromR[insertID].size() - 1);
    int insertPos = dis2(params.ran);   //插入路径
    copy_indiv = inner_indiv;   //先复制回去，后面可以修改一下逻辑
    inner_indiv.chromR[insertID].insert(inner_indiv.chromR[insertID].begin() + insertPos, rIn);   //插入元素
    inner_indiv.evaluateCompleteCost(params);
    updataIndiv();
    ejectionPool.erase(ejectionPool.begin()+pos);   //先直接删除这个元素
    //需要更新成不可行
    inner_indiv.eval.isFeasible=0;
    //注意惩罚系数乘以多少可以理解为是一个超参,用了ls之后，所有的都可能发生变化，所以要全部更新
    //run里面本身即会有导出individual，这个里面又有评估
    localSearch.run(inner_indiv,params.penaltyCapacity*10, params.penaltyEarly*1, params.penaltyLate*10,params.routepen,1,ejectionPool);  //1代表不能用空路径拓展的方式
    if (inner_indiv.eval.isFeasible)
    {
        updataIndiv();      //只有更新成功了才可以用
        return true;
    }
    else
    {
        //相当于是要构造这么一条不可行路径,这个也要更新路径
        inner_indiv=copy_indiv;
        inner_indiv.chromR[insertID].insert(inner_indiv.chromR[insertID].begin() + insertPos, rIn);   //如果插入失败了，就要把原来的插入回去
        insertLoseRoute=insertID;       //后续对于这条路径弹出
        inner_indiv.evaluateCompleteCost(params);
        updataIndiv();
        penaltyCounter[rIn]++;       //如果squeeze不成功的话，在这边就加了
        return false;
    }
}


void GES::updataIndiv()     //这个感觉很少会用到更新整个个体的时候，最后导出的时候可能需要用到
{
    //需要更新的内容：chromR
    redundancyL.clear();
    culredundancyL.clear();
    minredundancyL.clear();
    ejetionTime.clear();
    TfeasibleIS.clear();
    for (int r = 0; r < params.nbVehicles; r++)
    {
        if (!inner_indiv.chromR[r].empty())
        {
            //可行性更新
            TfeasibleIS.push_back(inner_indiv.chromRFeasible[r]);
            std::deque<double> redundancyL_temp;
            std::deque<double> culredundancyL_temp;
            double limitEmin=1e10;
            //注意还要加上空白的等待时间，先看最后一个点
            double limitout=params.cli[0].timeWindow[1]-inner_indiv.timeRS[r][inner_indiv.timeRS[r].size()-1];
            redundancyL_temp.push_back(limitout);
            if (limitout < limitEmin)
                limitEmin=limitout;
            culredundancyL_temp.push_back(limitEmin);
            //这边逆序看的主要原因是冗余时间要看后面的点，插入点的时候是插入前一个点的位置
            for (int i = inner_indiv.chromR[r].size()-1; i >=0; i--)
            {
                //注意这边的下标
                double innerlimit=params.cli[inner_indiv.chromR[r][i]].timeWindow[1]-inner_indiv.timeRS[r][i+1];
                if (inner_indiv.etimeRS[r][i+1] < 0)
                    innerlimit-=inner_indiv.etimeRS[r][i+1];    //这一段等待时间应该也要算上允许的误差
                redundancyL_temp.push_front(innerlimit);
                if (innerlimit < limitEmin)
                    limitEmin = innerlimit;
                culredundancyL_temp.push_front(limitEmin);
            }
            redundancyL.push_back(redundancyL_temp);        //这边保存的是chromR+1个点
            culredundancyL.push_back(culredundancyL_temp);
            minredundancyL.push_back(limitEmin);            //这个不太确定有没有用
            ejetionTime.push_back(insertNum);   //用来减少一些没意义的运算
        }
    }
}

void GES::updataRoute(int rIn)   //插入之后对于一条路径进行更新
{
    redundancyL[rIn].clear();
    culredundancyL[rIn].clear();

    vvd timeR, etimeR;	//注意这个地方把depot也保存下来了
    int isStartFeasible = 1;	//只要有一个不可行就是不可行
    GetStartTime(params, inner_indiv.chromR[rIn], timeR, etimeR, isStartFeasible);
    inner_indiv.timeRS[rIn]=timeR;  //时间直接在内更新了
    inner_indiv.etimeRS[rIn]=etimeR;
    inner_indiv.chromRFeasible[rIn]=isStartFeasible;
    TfeasibleIS[rIn] = isStartFeasible;  //验证当前解是否可行，不用考虑重量吗

    std::deque<double> redundancyL_temp;
    std::deque<double> culredundancyL_temp;
    double limitEmin=1e10;
    double limitout=params.cli[0].timeWindow[1]-timeR[timeR.size()-1];
    redundancyL_temp.push_back(limitout);
    if (limitout < limitEmin)
        limitEmin=limitout;
    culredundancyL_temp.push_back(limitEmin);
    //这边逆序看的主要原因是冗余时间要看后面的点，插入点的时候是插入前一个点的位置
    for (int i = inner_indiv.chromR[rIn].size()-1; i >=0; i--)
    {
        //注意这边的下标
        double innerlimit=params.cli[inner_indiv.chromR[rIn][i]].timeWindow[1]-timeR[i+1];
        if (etimeR[i+1] < 0)
            innerlimit-=etimeR[i+1];
        redundancyL_temp.push_front(innerlimit);
        if (innerlimit < limitEmin)
            limitEmin = innerlimit;
        culredundancyL_temp.push_front(limitEmin);  //从chromR上的第一个点开始的
    }
    redundancyL[rIn] = redundancyL_temp;        //这边保存的是chromR+1个点
    culredundancyL[rIn] = culredundancyL_temp;
    minredundancyL[rIn] = limitEmin;            //这个不太确定有没有用
    ejetionTime[rIn] = insertNum;   //用来减少一些没意义的运算
}

bool GES::insertCustomer(int vIn,int pos)   //输入的是真正的ID号
{
    //暂时把所有可以插入的点都放进来
    std::vector<std::pair<int,int>> insertPos;      //我感觉可以给这个池子设一个大小
    for (int i = 0; i < inner_indiv.chromR.size(); i++)
    {
        if (params.cli[vIn].demand + inner_indiv.loadRS[i] + MY_EPSILON <= params.vehicleCapacity
        && TfeasibleIS[i] && !inner_indiv.chromR[i].empty())    //并且需要是一个可行解
        {
            for (int j = 0; j <= inner_indiv.chromR[i].size(); j++)
            {
                double dis1=0;
                double dis2=0;
                double dis3=0;
                if (j==0)
                {
                    //如果是第一段路，直接用GET判读可行性
                    vvd timeR, etimeR;	//注意这个地方把depot也保存下来了
                    int isStartFeasible = 1;	//只要有一个不可行就是不可行
                    vvi chromRtemp=inner_indiv.chromR[i];
                    chromRtemp.insert(chromRtemp.begin()+j,vIn);
                    GetStartTime(params, chromRtemp, timeR, etimeR, isStartFeasible);   //判断一下插入之后时间上可不可行
                    if (isStartFeasible==1)
                        insertPos.emplace_back(i,0);    //表示第i条路的第0个段路径插入，从0到第一个点
                }   
                else
                {
                    if (j==inner_indiv.chromR[i].size())
                    {
                        dis1=params.timeCost[inner_indiv.chromR[i][j-1]][0];
                        //上一点结束时间+路径时间+这一个点的服务时间+到达下一个点的时间
                        dis3=params.timeCost[vIn][0];
                    }
                    else
                    {
                        dis1=params.timeCost[inner_indiv.chromR[i][j-1]][inner_indiv.chromR[i][j]];
                        dis3=params.timeCost[vIn][inner_indiv.chromR[i][j]];
                    }
                    dis2=inner_indiv.timeRS[i][j]+params.timeCost[inner_indiv.chromR[i][j-1]][vIn];
                    dis2+=params.cli[inner_indiv.chromR[i][j-1]].serviceDuration;   //加上服务时间
                    dis2=std::max(dis2,params.cli[vIn].timeWindow[0]);
                    if (dis2>params.cli[vIn].timeWindow[1]+MY_EPSILON) continue;//如果插入点的开始时间大于等于结束时间，就不用插入了          
                    double time_inner=dis2-inner_indiv.timeRS[i][j]-params.cli[inner_indiv.chromR[i][j-1]].serviceDuration;
                    //多出来的时间小于允许的冗余时间
                    if (time_inner+params.cli[vIn].serviceDuration+dis3+MY_EPSILON-dis1<culredundancyL[i][j])
                        insertPos.emplace_back(i,j);    //表示第i条路的第j个段路径插入
                }   
            }
        }
    }
    //可以进行一下可行性检验
    if (!insertPos.empty()) //这个池子里面肯定是满足重量约束的
    {
        std::uniform_int_distribution<> dis(0, insertPos.size()-1);
        int insertID = dis(params.ran);
        int rIn = insertPos[insertID].first;
        int jIn = insertPos[insertID].second;
        inner_indiv.chromR[rIn].insert(inner_indiv.chromR[rIn].begin()+jIn,vIn);
        ejectionPool.erase(ejectionPool.begin()+pos);       //需要从池子中删掉
        vvd timeR,etimeR;
        int isStartFeasible=1;
        GetStartTime(params, inner_indiv.chromR[rIn], timeR, etimeR, isStartFeasible);
        inner_indiv.evaluateCompleteCost(params);
        updataIndiv();

        return true;
    }
    else return false;   
}


void GES::calALLStartTime(vvi& route,std::deque<double>& etime)
{
    etime=std::deque<double>(route.size(),0.0);
    for (int i=0;i<(int)route.size();i++)
    {
        if (i==0)
        {
            double inner_time=params.cli[0].timeWindow[0]+params.timeCost[0][route[i]];
            etime[i]=std::max(inner_time,params.cli[route[i]].timeWindow[0]);
        }
        else
        {
            double inner_time=etime[i-1]+params.timeCost[route[i-1]][route[i]];
            inner_time+=params.cli[route[i-1]].serviceDuration; //服务时间也要加上
            etime[i]=std::max(inner_time,params.cli[route[i]].timeWindow[0]);
        }
    }
}

double GES::calStartTime(vvi& route,int rpos)
{
    double etime=0.0;
    for (int i=0;i<(int)route.size();i++)
    {
        if (i==0)
        {
            double inner_time=params.cli[0].timeWindow[0]+params.timeCost[0][route[i]];
            etime=std::max(inner_time,params.cli[route[i]].timeWindow[0]);
        }
        else
        {
            double inner_time=etime+params.timeCost[route[i-1]][route[i]];
            inner_time+=params.cli[route[i-1]].serviceDuration; //服务时间也要加上
            etime=std::max(inner_time,params.cli[route[i]].timeWindow[0]);
        }
        if (i==rpos)
            return etime;
    }
    return 0.0;
}


//这个rIn路径里面包含vIn这个点，所有也有可能会弹出
void GES::eject(int rIn)
{
    //寻找所有可能的弹出组合，并且让Psum最小，按照字典序列弹出
    vvi bestEjections;
    int ejNow=0;        //这个里面保存的是字典序号
    std::vector<int> ejectList; //内部的一个弹出池子
    std::vector<std::vector<int>> ejectPool;  //所有可能的较好的弹出组合，还有保存包含vin的弹出情况
    bool backtrack = false;  //是否回溯
    int ejfirst = 0;    //这个是用来向后拓展的
    int bestPsum=INT_MAX;   //这两个值应该要放在这个位置
    std::deque<double> aTimeRecord;
    calALLStartTime(inner_indiv.chromR[rIn],aTimeRecord);  //获得最一开始的到达时间

    int maymin=0;       //确定一个足够小的就可以减枝出来了

    while (ejNow >= 0)   //当总弹出数小于一定数量时
    {
        if (!ejectList.empty()) ejfirst=ejectList.front();  //就等于第一个元素
        int currentPsum = 0;
        if (ejNow > 0 && ejNow<=params.ap.ejMAX)    //不能超过最大值
        {
            // //减枝的情况
            vvi copyroute=inner_indiv.chromR[rIn];
            double loadejt=0;
            for (const int value: ejectList)
            {
                copyroute.erase(std::remove(copyroute.begin(), copyroute.end(), inner_indiv.chromR[rIn][value]), copyroute.end());
                loadejt+=params.cli[inner_indiv.chromR[rIn][value]].demand;
            }
            vvd timeR,etimeR;
            int isStartFeasible=1;
            //载重也要检查一下
            GetStartTime(params, copyroute, timeR, etimeR, isStartFeasible);
            //时间和重量都要可行
            if (isStartFeasible==1 && inner_indiv.loadRS[rIn]-loadejt+MY_EPSILON<=params.vehicleCapacity)
            {
                for (const int ej: ejectList)
                    currentPsum+=penaltyCounter[inner_indiv.chromR[rIn][ej]];
                if (currentPsum < bestPsum)
                {
                    //执行a减枝操作
                    bestPsum = currentPsum;
                    bestEjections.clear();
                    bestEjections = ejectList;
                    ejNow--;
                    //如果减完之后还大于等于0的话，就继续操作
                    if (ejNow>0)
                    {
                        ejectList.pop_back();   //删除最后一个元素，进行回溯
                        ejectList.back()++;
                    }
                    else if(ejNow==0)
                    {
                        ejectList.back()++;     //看一下会不会超出上界
                        ejNow++;
                    }
                }
            }
        }
        //在长度一样的情况下，生成下一个组合
        //需要关注在什么样的情况下，可以放到弹出池子里
        //最开始进去的时候，用来放第一个元素
        if (ejectList.empty() && ejfirst<=inner_indiv.chromR[rIn].size()-1)  
        {
            if (!inner_indiv.chromR[rIn].empty())
            {
                ejectList.emplace_back(ejfirst);
                ejNow = 1;
            }
            else ejNow--;   //直接就退出循环了
            continue;
        }
        //开始正常循环的时候，在一个根目录下
        int lastpos=ejectList.back();
        if (ejNow < params.ap.ejMAX && lastpos < inner_indiv.chromR[rIn].size()-1)
        {
            ejectList.emplace_back(lastpos+1);  //往后拓展最后一个元素
            ejNow++;
        }
        else
        {
            if (lastpos < inner_indiv.chromR[rIn].size()-1)
            { 
                ejectList.back()++;     //这个相当于是递增之后的情况了
                //考虑b和c的减枝情况,正常来说减枝不会导致死循环
                vvi copyroute=inner_indiv.chromR[rIn];  //还是需要移除这个路径
                for (const int& value: ejectList)
                    copyroute.erase(std::remove(copyroute.begin(), copyroute.end(), inner_indiv.chromR[rIn][value]), copyroute.end());
                double timeback=calStartTime(copyroute,lastpos);    //算当前这个点的最早到达时间
                //b情况
                if (timeback>MY_EPSILON+params.cli[inner_indiv.chromR[rIn][lastpos]].timeWindow[1])
                    backtrack = true;   //需要回溯
                //c情况
                if (timeback==aTimeRecord[lastpos] && inner_indiv.loadRS[rIn]<=params.vehicleCapacity)
                    backtrack = true;   //需要回溯
                if (backtrack)
                {
                    backtrack = false;
                    ejNow--;
                    if (ejNow>0)
                    {
                        ejectList.pop_back();   //删除最后一个元素，进行回溯
                        ejectList.back()++;
                    }
                    else if(ejNow==0)
                    {
                        ejectList.back()++;
                        ejNow++;
                    }
                }
            }
            else
            {
                if (ejNow == 1)
                {
                    ejNow=-1;    //0
                    ejectList.pop_back();    //没元素了
                }
                else
                {
                    ejectList.pop_back();
                    ejectList.back()++;
                    ejNow--;    //这边就减少一个元素
                } 
            }
        }
    }
    vvi ejpoint;
    for (int ii:bestEjections)
    {
        ejpoint.push_back(inner_indiv.chromR[rIn][ii]);
    }
    ejectionPool.insert(ejectionPool.end(),ejpoint.begin(),ejpoint.end());
    vvi copyRoute=inner_indiv.chromR[rIn];
    for (int rr:bestEjections)
        inner_indiv.chromR[rIn].erase(std::remove(inner_indiv.chromR[rIn].begin(),inner_indiv.chromR[rIn].end(),copyRoute[rr]),inner_indiv.chromR[rIn].end());
    if (inner_indiv.chromR[rIn].empty())        //如果空了，就全部都删掉了
    {
        inner_indiv.chromR.erase(inner_indiv.chromR.begin() + rIn);
        inner_indiv.chromR.push_back(std::vector<int>());
        inner_indiv.evaluateCompleteCost(params);
        updataIndiv();
    }
    else
    {
        inner_indiv.evaluateCompleteCost(params);
        updataIndiv();
    }
}


bool GES::feasibleJudge(Individual& indiv)   //判断是否可行
{
    for (int i = 0; i < indiv.chromR.size(); i++)
    {
        if (!indiv.chromR[i].empty())
        {
            if (TfeasibleIS[i]==0)
                return false;
        }
    }
    return true;
}
