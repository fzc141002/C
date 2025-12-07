#ifndef INSTANCECVRPLIB_H
#define INSTANCECVRPLIB_H
#include<string>
#include<vector>

class InstanceCVRPLIB
{
public:
	std::vector<double> x_coords;	//vector表示动态数组
	std::vector<double> y_coords;
	std::vector< std::vector<double> > dist_mtx;	//二维向量，存储距离
	std::vector<double> service_time;
	std::vector<double> demands;
	std::vector<double> due_data;
	std::vector<double> ready_time;
	int nbVeh;												// 定义一下车的数量
	double vehicleCapacity = 1.e30;							// Capacity limit
	bool isDurationConstraint = true;						// Indicates if the problem includes duration constraints
	int nbClients;											// Number of clients (excluding the depot)，出去depot的客户点数量

	InstanceCVRPLIB(std::string pathToInstance, bool isRoundingInteger);
	//构造函数，用于创建对象
	//用std的原因，明确指定了string类的来源
};


#endif //INSTANCECVRPLIB_H
