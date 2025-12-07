#include "Gather.h"
// #include "coptcpp_pch.h"
#include "gurobi_c++.h"

bool Gather::run(int minNowCar,Individual& gatherIndiv,int carLast)
{
    GRBEnv env = GRBEnv(true);
    env.set("LogFile", "gather.log");
    env.start();
    GRBModel model = GRBModel(env);

    thisDelete=0;   //每次进来的时候把删除的点都重新计算
    if (iteror==0)  //会有元素被删掉，所以需要重新计算
    {
        lastNum=params.newChromGather.size();
        iteror++;
        for (int i = 0;i<lastNum;i++)
        {
            for (int j=0;j<num_customers;j++)
            {
                if (std::find(params.newChromGather[i].begin(), params.newChromGather[i].end(), j+1) != params.newChromGather[i].end())
                {
                    anew[j].push_back(1); // 更新路径-客户覆盖关系
                }
                else
                {
                    anew[j].push_back(0);
                }
            }
        }
        if (lastNum> params.boolNum)
        {
            for (int i=0;i<lastNum-params.boolNum;i++)
            {
                params.newChromGather.erase(params.newChromGather.begin()); // 删除最早的路径
                params.newCostGather.erase(params.newCostGather.begin()); // 删除对应的成本
                for (int j=0;j<num_customers;j++)
                {
                    anew[j].erase(anew[j].begin());   //删除第一个元素
                    thisDelete++;
                }
            }
        }
        lastNum = params.newChromGather.size(); // 更新为删除之后的大小
    }
    // 路径数量
    int num_routes = params.newChromGather.size(); // Ω的大小
    for (int i = lastNum;i<num_routes;i++)
    {
        for (int j=0;j<num_customers;j++)
        {
            if (std::find(params.newChromGather[i].begin(), params.newChromGather[i].end(), j+1) != params.newChromGather[i].end())
            {
                anew[j].push_back(1); // 更新路径-客户覆盖关系
            }
            else
            {
                anew[j].push_back(0); //这个里面保存的是内部的随机扰动
            }
        }
    }
    if (num_routes> params.boolNum)
    {
        for (int i=0;i<num_routes-params.boolNum;i++)
        {
            params.newChromGather.erase(params.newChromGather.begin()); // 删除最早的路径
            params.newCostGather.erase(params.newCostGather.begin()); // 删除对应的成本
            for (int j=0;j<num_customers;j++)
            {
                anew[j].erase(anew[j].begin());   //删除第一个元素
                thisDelete++;
            }
        }
    }
    lastNum = params.newChromGather.size(); // 保存上一次的路径数量
    //获得内部的扰动路径
    int pertubMax=std::min(params.boolinner,lastNum);
    if (innerChromGather.size()==params.boolinner) pertubMax=50;    //如果满了的话，只更新50条
    std::uniform_int_distribution<int> dis(0, lastNum - 1);
    int reverse=0;
    for (int i=0;i<pertubMax;i++)
    {
        reverse=dis(params.ran); // 随机选择一个路径
        vvi tempChrom = params.newChromGather[reverse]; // 获取路径
        std::shuffle(tempChrom.begin(), tempChrom.end(), params.ran); // 打乱路径顺序
        innerChromGather.push_back(tempChrom); // 添加到新的路径集合中
        double tempCost = costRoute(tempChrom); // 计算路径成本
        innerCostGather.push_back(tempCost); // 添加到新的成本集合中
        for (int j=0;j<num_customers;j++)
        {
            if (std::find(tempChrom.begin(), tempChrom.end(), j+1) != tempChrom.end())
            {
                apertub[j].push_back(1); // 更新路径-客户覆盖关系
            }
            else
            {
                apertub[j].push_back(0);
            }
        }
        if (innerChromGather.size() > params.boolNum) 
        {
            innerChromGather.erase(innerChromGather.begin()); // 删除最早的路径
            innerCostGather.erase(innerCostGather.begin()); // 删除对应的成本
            for (int j = 0; j < num_customers; ++j) 
            {
                apertub[j].erase(apertub[j].begin());   //删除第一个元素
            }
        }
    }
    // 最大车辆数
    int beta = minNowCar;
    num_routes=params.newChromGather.size();
    int allroutes=params.newChromGather.size()+innerCostGather.size();
    // 每条路径的cost
    std::vector<double> cost_vec;
    cost_vec.reserve(params.newCostGather.size() + innerCostGather.size());
    cost_vec.insert(cost_vec.end(), params.newCostGather.begin(), params.newCostGather.end());
    cost_vec.insert(cost_vec.end(), innerCostGather.begin(), innerCostGather.end());
    std::vector<int> coeffs(num_customers * allroutes, 0);
    for (int i = 0; i < num_customers; ++i) {
        for (int j = 0; j < allroutes; ++j) {
            if (j < num_routes)
                coeffs[i * allroutes + j] = anew[i][j];
            else
                coeffs[i * allroutes + j] = apertub[i][j - num_routes];
        }
    }
    std::vector<GRBVar> lambda_var(allroutes);
    for (int k = 0; k < allroutes; ++k) 
    {
        lambda_var[k] = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, "choose_" + std::to_string(k));
    }
    
    for (int i = 0; i < num_customers; ++i) {
        GRBLinExpr cover = 0;
        for (int j = 0; j < allroutes; ++j) {
            int coeff = (j < num_routes) ? anew[i][j] : apertub[i][j - num_routes];
            cover += coeff * lambda_var[j];
        }
        model.addConstr(cover == 1, "cover_cust_" + std::to_string(i));
    }

    GRBLinExpr route_sum = 0;
    for (int k = 0; k < allroutes; ++k) route_sum += lambda_var[k];
    model.addConstr(route_sum <= beta, "max_routes");

    GRBLinExpr obj = 0;
    for (int k = 0; k < allroutes; ++k) 
    {
        obj += cost_vec[k] * lambda_var[k];
    }

    model.setObjective(obj, GRB_MINIMIZE);

    model.optimize();

    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
        std::cout << "Optimal value: " << model.get(GRB_DoubleAttr_ObjVal) << std::endl;
        gatherIndiv.chromR = std::vector<std::vector<int>>(params.nbVehicles);
        gatherIndiv.chromT = std::vector<int>(params.nbClients);
        int cusNum = 0;
        int roadNum = 0;
        for (int k = 0; k < allroutes; ++k) {
            if (lambda_var[k].get(GRB_DoubleAttr_X) > 1e-6) {
                if (k < num_routes) {
                    for (int i : params.newChromGather[k]) {
                        gatherIndiv.chromT[cusNum] = i;
                        cusNum++;
                    }
                    gatherIndiv.chromR[roadNum] = params.newChromGather[k];
                } else {
                    for (int i : innerChromGather[k - num_routes]) {
                        gatherIndiv.chromT[cusNum] = i;
                        cusNum++;
                    }
                    gatherIndiv.chromR[roadNum] = innerChromGather[k - num_routes];
                }
                roadNum++;
            }
        }
        gatherIndiv.evaluateCompleteCost(params);
        if (gatherIndiv.eval.isFeasible) {
            std::cout << "可行" << std::endl;
            return true;
        } else {
            std::cout << "不可行" << std::endl;
            return false;
        }
    } else {
        std::cout << "No optimal solution found." << std::endl;
        return false;
    }
    
    // //考虑在这边跑两次
    // NdArray<int,2> ainner(Shape<2>(num_customers, allroutes), coeffs.data(), num_customers * allroutes);
    // // 1. 添加变量 λ_k（二进制变量）
    // MVar<1> lambda_var = model.AddMVar(Shape<1>(allroutes), COPT_BINARY,"choose");
    // //设置初始解的值
    // // std::vector<double> mipstart(allroutes, 0.0);
    // // for (int i = 0; i < allroutes; ++i) 
    // // {
    // //     if (i < num_routes-thisDelete && i >= num_routes - carLast-thisDelete) 
    // //     {
    // //         mipstart[i] = 1.0;
    // //     }
    // // }
    // // model.SetMipStart(allroutes, mipstart.data());
    // // model.LoadMipStart();
    // NdArray<int, 1> nuri_upper(Shape<1>(num_customers), 1);
    // // 约束：每条路径的 λ_k * a_ik >= nuri_upper
    // model.AddMConstr(Mat::matmult(ainner,lambda_var) == nuri_upper);
    // // 约束路径数不超过β
    // NdArray<int, 1> sum_cor(Shape<1>(allroutes), 1);
    // model.AddMConstr(Mat::matmult(sum_cor, lambda_var) <= beta);
    // // 4. 设置目标为最小化
    // NdArray<double, 1> cost(Shape<1>(allroutes), cost_vec.data(), allroutes);
    // model.SetObjective(Mat::matmult(cost, lambda_var).Item(), COPT_MINIMIZE);  //转化为一个标量
    // // 5. 求解
    // // model.SetIntParam("MipStartMode",1);        //设置整数规划的相关结果
    // model.Solve();
    // // 6. 输出结果
    // if (model.GetIntAttr(COPT_INTATTR_HASMIPSOL)) 
    // {
    //     std::cout << "Optimal value: " << model.GetDblAttr(COPT_DBLATTR_BESTOBJ) << std::endl;
    //     gatherIndiv.chromR=std::vector < std::vector <int> >(params.nbVehicles); // 清空之前的路径
    //     gatherIndiv.chromT=std::vector <int>(params.nbClients);
    //     int cusNum=0;
    //     int roadNum=0;
    //     for (int k = 0; k < allroutes; ++k) 
    //     {
    //         Var item = lambda_var[k].Item(0);   //注意lambda_var[k]是一个一维数组，Item(0)获取它的值
    //         if (item.Get("Value") > 1e-6) 
    //         {
    //             if (k<num_routes) // 如果是原来的路径
    //             {
    //                 for (int i:params.newChromGather[k]) 
    //                 {
    //                     gatherIndiv.chromT[cusNum]=i; // 将路径中的客户添加到chromT中
    //                     cusNum++;
    //                 }
    //                 gatherIndiv.chromR[roadNum]=params.newChromGather[k]; // 将选中的路径添加到gatherIndiv中
    //             }
    //             else
    //             {
    //                 for (int i:innerChromGather[k-num_routes]) 
    //                 {
    //                     gatherIndiv.chromT[cusNum]=i; // 将路径中的客户添加到chromT中
    //                     cusNum++;
    //                 }
    //                 gatherIndiv.chromR[roadNum]=innerChromGather[k-num_routes]; // 将选中的路径添加到gatherIndiv中
    //             }
    //             roadNum++;
    //         }
    //     }
    //     gatherIndiv.evaluateCompleteCost(params); //计算一下成本
    //     if (gatherIndiv.eval.isFeasible)
    //     {
    //         std::cout<<"可行"<<std::endl;
    //         return true;
    //     }
    //     else 
    //     {
    //         std::cout<<"不可行"<<std::endl;  
    //         return false;
    //     }
    // } 
    // else 
    // {
    //     std::cout << "No optimal solution found." << std::endl;
    //     return false;
    // }
}

double Gather::costRoute(vvi chromR)
{
    vvd timeR,etimR;
    int isStartFeasible=1; //默认是可行的
    GetStartTime(params,chromR,timeR,etimR,isStartFeasible);
    double load=params.cli[chromR[0]].demand; //第一个点的载重
    double etime=0.0;
    double ltime=0.0;
    double cost=params.timeCost[0][chromR[0]];
    double allcost=0.0;
    if (etimR[1]>0) ltime+=etimR[1];
    for (int i=1;i<chromR.size();i++)
    {
        cost+=params.timeCost[chromR[i-1]][chromR[i]];
        load+=params.cli[chromR[i]].demand;
        if (etimR[i+1]<0) etime-=etimR[i+1]; //早到
        else if (etimR[i+1]>0) ltime+=etimR[i+1]; //迟到
    }
    cost+=params.timeCost[chromR[chromR.size()-1]][0]; //最后一个点回到depot的距离
    ltime+=etimR[etimR.size()-1]; //最后一个点回到depot的迟到时间
    double loadExcess= load - params.vehicleCapacity; //计算载重超出多少
    double loadcost=0.0;
    if (loadExcess > 0)
    {
        loadcost = params.penaltyCapacity * loadExcess; // 超载的惩罚成本
        isStartFeasible = 0; // 如果超载了，解不可行
    }
        
    allcost = cost + loadcost + params.penaltyEarly * etime + params.penaltyLate * ltime; // 计算总成本
    return allcost;
}