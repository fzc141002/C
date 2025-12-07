#include "C_Interface.h"
#include "Population.h"
#include "Params.h"
#include "Genetic.h"
#include <string>
#include <iostream>
#include <vector>
#include <cmath>

//从给定的最佳种群中，提取最优解的信息
Solution *prepare_solution(Population &population, Params &params)
{
	// Preparing the best solution
	Solution *sol = new Solution;
	sol->time = (double)(clock() - params.startTime) / (double)CLOCKS_PER_SEC;

	if (population.getBestFound() != nullptr) {
		// Best individual
		auto best = population.getBestFound();

		// setting the cost
		sol->cost = best->eval.penalizedCost;

		// finding out the number of routes in the best individual
		int n_routes = 0;
		for (int k = 0; k < params.nbVehicles; k++)
			if (!best->chromR[k].empty()) ++n_routes;

		// filling out the route information，填充路线信息
		sol->n_routes = n_routes;
		sol->routes = new SolutionRoute[n_routes];
		for (int k = 0; k < n_routes; k++) {
			sol->routes[k].length = (int)best->chromR[k].size();
			sol->routes[k].path = new int[sol->routes[k].length];
			std::copy(best->chromR[k].begin(), best->chromR[k].end(), sol->routes[k].path);
		}
	}
	else {
		sol->cost = 0.0;
		sol->n_routes = 0;
		sol->routes = nullptr;
	}
	return sol;
}

//这一段代码提供了一个c语言的调用接口
extern "C" Solution *solve_cvrp(
	int n, double *x, double *y, double *serv_time, double *dem,
	double d_time,double r_time,double vehicleCapacity, char isRoundingInteger, char isDurationConstraint,
	int max_nbVeh, const AlgorithmParameters *ap, char verbose)
{
	//Solution *result;
	Solution *result=NULL;

	try {
		std::vector<double> x_coords(x, x + n);
		std::vector<double> y_coords(y, y + n);
		std::vector<double> service_time(serv_time, serv_time + n);
		std::vector<double> demands(dem, dem + n);
		std::vector<double> due_data(d_time, d_time + n);
		std::vector<double> ready_time(r_time, r_time + n);
		std::vector<std::vector<double> > distance_matrix(n, std::vector<double>(n));
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				distance_matrix[i][j] = std::sqrt(
					(x_coords[i] - x_coords[j])*(x_coords[i] - x_coords[j])
					+ (y_coords[i] - y_coords[j])*(y_coords[i] - y_coords[j])
				);
				distance_matrix[i][j]=std::round(distance_matrix[i][j]*10)/10;	//保留一位小数
				// if (isRoundingInteger)
				// 	distance_matrix[i][j] = std::round(distance_matrix[i][j]);
			}
		}

		Params params(x_coords,y_coords,distance_matrix,service_time,demands,due_data,ready_time,vehicleCapacity,max_nbVeh,isDurationConstraint,verbose,*ap);

		// Running HGS and returning the result
		Genetic solver(params);
		solver.run();
		result = prepare_solution(solver.population, params);
	}
	catch (const std::string &e) { std::cout << "EXCEPTION | " << e << std::endl; }
	catch (const std::exception &e) { std::cout << "EXCEPTION | " << e.what() << std::endl; }

	return result;
}

extern "C" Solution *solve_cvrp_dist_mtx(
	int n, double *x, double *y, double *dist_mtx, double *serv_time, double *dem, double d_time, double r_time,
	double vehicleCapacity, char isDurationConstraint,
	int max_nbVeh, const AlgorithmParameters *ap, char verbose)
{
	//Solution *result;
	Solution *result=NULL;
	std::vector<double> x_coords;
	std::vector<double> y_coords;

	try {
		if (x != nullptr && y != nullptr) {
			x_coords = {x, x + n};
			y_coords = {y, y + n};
		}

		std::vector<double> service_time(serv_time, serv_time + n);
		std::vector<double> demands(dem, dem + n);
		std::vector<double> due_data(d_time, d_time + n);
		std::vector<double> ready_time(r_time, r_time + n);
		std::vector<std::vector<double> > distance_matrix(n, std::vector<double>(n));
		for (int i = 0; i < n; i++) { // row
			for (int j = 0; j < n; j++) { // column
				distance_matrix[i][j] = dist_mtx[n * i + j];
			}
		}

		Params params(x_coords,y_coords,distance_matrix,service_time,demands,due_data,ready_time,vehicleCapacity,max_nbVeh,isDurationConstraint,verbose,*ap);
		
		// Running HGS and returning the result
		Genetic solver(params);
		solver.run();
		result = prepare_solution(solver.population, params);
	}
	catch (const std::string &e) { std::cout << "EXCEPTION | " << e << std::endl; }
	catch (const std::exception &e) { std::cout << "EXCEPTION | " << e.what() << std::endl; }

	return result;
}

extern "C" void delete_solution(Solution *sol)
{
	for (int i = 0; i < sol->n_routes; ++i)
		delete[] sol->routes[i].path;

	delete[] sol->routes;
	delete sol;
}