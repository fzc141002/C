#ifndef CHOOSETYPE_H
#define CHOOSETYPE_H

enum class InsertionHeuristic 
{
    Distance = 1,
    Cost = 2
};

enum class PerturbationType 
{
    Sequential = 0,
    Concentric = 1
};

enum class StoppingCriterionType 
{
    Time = 1,
    Iteration = 2
};

#endif