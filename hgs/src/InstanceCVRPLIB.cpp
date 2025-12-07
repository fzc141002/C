#include <fstream>
#include <cmath>
#include <iostream>
#include "InstanceCVRPLIB.h"

InstanceCVRPLIB::InstanceCVRPLIB(std::string pathToInstance, bool isRoundingInteger = true)
{

	std::string content, content2, content3;
	double serviceTimeData = 0.;	//服务时间

	// Read INPUT dataset
	std::ifstream inputFile(pathToInstance);//文件读入，相当于也是一个构造函数
	if (inputFile.is_open())
	{
		std::string spilt_name;
		getline(inputFile, spilt_name);
		if (std::stoi(spilt_name.substr(4, 1)) == 2)
			nbClients = 200;
		else if (std::stoi(spilt_name.substr(4, 1)) == 4)
			nbClients = 400;
		std::cout << "The number of customer points is:" << nbClients << std::endl;
		getline(inputFile, content);	//跳过前三行的内容
		getline(inputFile, content);	//跳过前三行的内容
		getline(inputFile, content);	//跳过前三行的内容
		inputFile >> nbVeh >> vehicleCapacity;
		std::cout << "The number of car is:" << nbVeh << "\tThe capacity of vehicle:" << vehicleCapacity << std::endl;

		getline(inputFile, content);	//再跳四行
		getline(inputFile, content);
		getline(inputFile, content);
		getline(inputFile, content);
		
		x_coords = std::vector<double>(nbClients + 1);		//初始化一下
		y_coords = std::vector<double>(nbClients + 1);
		demands = std::vector<double>(nbClients + 1);
		service_time = std::vector<double>(nbClients + 1);
		due_data = std::vector<double>(nbClients + 1);
		ready_time = std::vector<double>(nbClients + 1);

		int node_number;
		
		for (int i = 0; i <= nbClients; i++)
		{
			inputFile >> node_number >> x_coords[i] >> y_coords[i] >> demands[i] >> ready_time[i]
				>> due_data[i] >> service_time[i];
			if (node_number != i) throw std::string("The node numbering is not in order.");
		}

		if (nbClients <= 0) throw std::string("Number of nodes is undefined");
		if (vehicleCapacity == 1.e30) throw std::string("Vehicle capacity is undefined");

		// 计算距离
		dist_mtx = std::vector < std::vector< double > >(nbClients + 1, std::vector <double>(nbClients + 1));	//初始化一个二维向量，后面是构造函数的参数
		for (int i = 0; i <= nbClients; i++)
		{
			for (int j = 0; j <= nbClients; j++)
			{
				dist_mtx[i][j] = std::sqrt(
					(x_coords[i] - x_coords[j]) * (x_coords[i] - x_coords[j])
					+ (y_coords[i] - y_coords[j]) * (y_coords[i] - y_coords[j])
				);
				dist_mtx[i][j]=std::round(dist_mtx[i][j]*100)/100;	//保留一位小数
				// std::cout<<"dist_mtx["<<i<<"]["<<j<<"]="<<dist_mtx[i][j]<<std::endl;	//输出距离矩阵
				//是否进行四舍五入
				// if (isRoundingInteger) dist_mtx[i][j] = round(dist_mtx[i][j]);
			}
		}

		// Reading depot information (in all current instances the depot is represented as node 1, the program will return an error otherwise)
		// inputFile >> content >> content2 >> content3 >> content3;
		// if (content != "DEPOT_SECTION") throw std::string("Unexpected data in input file: " + content);
		// if (content2 != "1") throw std::string("Expected depot index 1 instead of " + content2);
		// if (content3 != "EOF") throw std::string("Unexpected data in input file: " + content3);
	}
	else throw std::string("Impossible to open instance file: " + pathToInstance);
}
