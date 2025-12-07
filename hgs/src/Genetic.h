#ifndef GENETIC_H
#define GENETIC_H

#include "Population.h"
#include "Individual.h"
#include "Initial.h"
#include "GES.h"
#include "Gather.h"
#include "ChooseType.h"
#include "DestoryAndRepair.h"
#include "GlobalSearch.h"

class Genetic
{
public:

	Params & params;				// Problem parameters
	Initial initial;				// Initial population
	Split split;					// Split algorithm
	LocalSearch localSearch;		// Local Search structure
	DestoryAndRepair destoryAndRepair;
	GES ges;						// GES structure
	Population population;			// Population (public for now to give access to the solutions, but should be be improved later on)
	Individual offspring;			// First individual to be used as input for the crossover，后代
	Gather gather;					// Gather data from the population and write to a file
	GlobalSearch globalSearch;	    //用来尝试做最后的路径缩减
	Individual gatherIndiv;
	Individual gsIndiv;

	int lastbest;
	// OX Crossover
	void crossoverOX(Individual & result, const Individual & parent1, const Individual & parent2);
	void rouletteWheelSelection(Individual& selected);
    // Running the genetic algorithm until maxIterNonProd consecutive iterations or a time limit
    void run() ;//可以理解为是遗传算法的主程序入口

	// Constructor
	Genetic(Params& params);
};

#endif
