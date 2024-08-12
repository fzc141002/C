#include"gurobi_c++.h"
#include<iostream>
using namespace std;

//try to rewrite the official case myself
//     maximize    x +   y + 2 z
//	   subject to  x + 2 y + 3 z <= 4
//				   x + y >= 1
//                 x, y, z binary
int main()
{
	try
	{
		//定义一个环境变量
		GRBEnv env = GRBEnv(true);
		env.start();
		//建立一个模型
		GRBModel model = GRBModel(env);		//注意小写的model是c里面的用法

		//定义变量
		GRBVar x = model.addVar(0, 1, 0, GRB_BINARY, "x");
		GRBVar y = model.addVar(0, 1, 0, GRB_BINARY, "y");
		GRBVar z = model.addVar(0, 1, 0, GRB_BINARY, "z");

		//定义目标函数，注意添加优化方向
		model.setObjective(x + y + 2 * z,GRB_MAXIMIZE);

		//定义约束
		model.addConstr(x + 2 * y + 3 * z <= 4, "约束1");
		model.addConstr(x + y >= 1, "约束2");


		//模型求解
		model.optimize();
		cout << "模型求解完成" << endl;
		//查看模型求解的状态，其中返回的是枚举变量
		int m_statue = model.get(GRB_IntAttr_Status);
		if (m_statue == GRB_OPTIMAL)
		{
			//获得当前目标函数的值
			double objval = model.get(GRB_DoubleAttr_ObjVal);
			cout << "当前目标函数为" << objval << endl;
			//输出当前模型找到了几个可行解 
			cout << "当前模型可行解的数量为" << model.get(GRB_IntAttr_SolCount) << endl;
			cout << x.get(GRB_StringAttr_VarName) << ":" << x.get(GRB_DoubleAttr_X) << endl;
			cout << y.get(GRB_StringAttr_VarName) << ":" << y.get(GRB_DoubleAttr_X) << endl;
			cout << z.get(GRB_StringAttr_VarName) << ":" << z.get(GRB_DoubleAttr_X) << endl;

		}
		else
		{
			cout << "模型未找到最优解" << endl;
		}
	}
	catch (GRBException e)
	{
		cout << "错误的问题为" << e.getErrorCode() << endl;
		cout << e.getMessage() << endl;
	}
	catch (...)
	{
		cout << "遇到了其他问题" << endl;
	}

	return 0;
}
