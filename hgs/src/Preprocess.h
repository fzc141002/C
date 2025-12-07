#ifndef PREPROCESS_H
#define PREPROCESS_H
#include "Params.h"
#include <vector>
#include <unordered_set>    //基于哈希实现的无序集合
#include <utility>

#define my_MAX 1e10


class Preprocess
{
private:
    Params& params;
    std::vector<std::vector<int>> keypoints;
    bool changed;
    void Rule1();
    void Rule2();
    void Rule3();
    void Rule4();

public:
    std::unordered_set<std::pair<int, int>, PairHash> directedEdges;    //在构造edge的时候会自动生成hash结构体，通用性比较好

    Preprocess(Params& params);
    void preprocessTimeWindows(); // 预处理时间窗
    void edgedelete();  //边的删除
    void infeasibleEdgeGather();
    void feasibleEdgeGather();
};

#endif // PREPROCESS_H
