#include <iostream>
#include <stdlib.h>
#include "Genetic.h"
#include "commandline.h"
#include "LocalSearch.h"
#include "Split.h"
#include "InstanceCVRPLIB.h"
#include "Preprocess.h"
#include "DisAndTimeCluster.h"
using namespace std;

int main(int argc, char* argv[])
{
	try
	{
		// Reading the arguments of the program
		CommandLine commandline(argc, argv);

		// Print all algorithm parameter values
		if (commandline.verbose) print_algorithm_parameters(commandline.ap);

		// Reading the data file and initializing some data structures
		if (commandline.verbose) std::cout << "----- READING INSTANCE: " << commandline.pathInstance << std::endl;
		InstanceCVRPLIB cvrp(commandline.pathInstance, commandline.isRoundingInteger);

		//这个参数输入需要改变一下
		Params params(cvrp.x_coords, cvrp.y_coords, cvrp.dist_mtx, cvrp.service_time, cvrp.demands, cvrp.due_data, cvrp.ready_time,
			cvrp.vehicleCapacity, cvrp.nbVeh, cvrp.isDurationConstraint, commandline.verbose, commandline.ap);
		cout<<"总重量为:"<<params.totalDemand<<endl;
		std::string fullPath = commandline.pathInstance;
		size_t pos = fullPath.find_last_of("/\\");
		std::string filename = (pos == std::string::npos) ? fullPath : fullPath.substr(pos + 1);
		
		// 去掉.txt或.TXT后缀（不区分大小写）
		if (filename.length() > 4) {
			std::string suffix = filename.substr(filename.length() - 4);
			if (suffix == ".txt" || suffix == ".TXT") {
				filename = filename.substr(0, filename.length() - 4);
			}
		}
		params.fileName = filename;
		// Running HGS，定义了一个类对象
		
		//时空聚类
		// VRPTWClustering clustering(params);
		// clustering.calculateDistanceMatrices();
		// std::vector<int> medoids = clustering.performClustering();
		// std::vector<std::vector<int>> clusters = clustering.assignCustomersToClusters(medoids);
		// clustering.printClusteringResults(medoids, clusters);

		Preprocess pre(params);
		// //返回前后续点
		// pre.preprocessTimeWindows();
		pre.edgedelete();
		params.deleteEdges=pre.directedEdges;	//赋值进去


		//聚类之后每个子问题都单独求一遍
		Genetic solver(params);
		solver.run();

		// Exporting the best solution
		if (solver.population.getBestFound() != NULL)
		{
			if (params.verbose) std::cout << "----- WRITING BEST SOLUTION IN : " << commandline.pathSolution << std::endl;
			solver.population.exportCVRPLibFormat(*solver.population.getBestFound(), commandline.pathSolution);
			solver.population.exportSearchProgress(commandline.pathSolution + ".PG.csv", commandline.pathInstance);
		}
	}
	catch (const string& e) { std::cout << "EXCEPTION | " << e << std::endl; }
	catch (const std::exception& e) { std::cout << "EXCEPTION | " << e.what() << std::endl; }
	return 0;
}