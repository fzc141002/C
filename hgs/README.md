# src 文件夹说明

`src` 文件夹包含项目的主要源代码。以下是每个代码文件的用途说明：

- **Makefile**  

- **main.cpp**  
    主函数，算法入口

- **AlgorithmParameters.cpp**  
    保存算法层面的参数，如迭代次数等

- **ALNSInitial.cpp**  
    alns算法初始解生成模块，迭代生成初始解

- **C_Interface.cpp**  
    C语言或其他外部程序提供调用HGS算法的接口，实现了C风格的API。

- **ChooseType.cpp**  
    定义了插入、破坏和停止准则对应的编号

- **DestoryAndRepair.cpp**  
    定义了两种破坏方式的具体实现，同时包含修复过程

- **Gather.cpp**  
    实现了集合覆盖增强技术

- **Genetic.cpp**  
    实现了遗传算法过程中主体流程，包括交叉操作以及LCS精英结构保留等

- **GES.cpp**  
    实现了车辆缩减操作

- **GlobalSearch.cpp**  
    原本的目标是想从全局上再缩减距离，但是效果不是很好

- **Individual.cpp**  
    定义了个体的包含的一些属性，以及功能函数

- **Initial.cpp**  
    实现了聚类的初始化操作

- **InstanceCVRPLIB.cpp**  
    数据层面的读入操作

- **LocalSearch.cpp**  
    实现了九种移动方式，并且在特定的时候调用该函数

- **Params.cpp**  
    保存惩罚系数等算法内部的参数，用户自定义

- **Population.cpp**  
    定义了种群相关函数，包括种群整体初始化，种群适应度计算，惩罚系数更新等

- **Preprocess.cpp**  
    实现了预处理部分的函数，包括时间窗缩紧和不可行路径池提取

- **Split.cpp**  
    定义了不同的分割函数，将一条完整的客户序列分配到不同的车上去

- **StartTime.cpp**  
    定义了一个路径时刻表计算函数，根据当前路径以及出发时间确实整个路径的时间窗违反情况