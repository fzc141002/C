#ifndef C_INTERFACE_H
#define C_INTERFACE_H
#include "AlgorithmParameters.h"

struct SolutionRoute	
{
	//定义一个解路径的结构体
	int length;
	int * path;
};

struct Solution
{
	//定义一个解
	double cost;
	double time;
	int n_routes;
	struct SolutionRoute * routes;
};

#ifdef __cplusplus
extern "C"
#endif
struct Solution * solve_cvrp(
	int n, double* x, double* y, double* serv_time, double* dem,
	double d_time, double r_time, double vehicleCapacity, char isRoundingInteger, char isDurationConstraint,
	int max_nbVeh, const AlgorithmParameters* ap, char verbose);

#ifdef __cplusplus
extern "C"
#endif
struct Solution *solve_cvrp_dist_mtx(
	int n, double* x, double* y, double* dist_mtx, double* serv_time, double* dem, double d_time, double r_time,
	double vehicleCapacity, char isDurationConstraint,
	int max_nbVeh, const AlgorithmParameters* ap, char verbose);

#ifdef __cplusplus
extern "C"
#endif
void delete_solution(struct Solution * sol);


#endif //C_INTERFACE_H
