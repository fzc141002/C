#ifndef DISANDTIMECLUSTER_H
#define DISANDTIMECLUSTER_H

//参考文献：https://doi.org/10.1016/j.tre.2011.07.001

#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <fstream>
#include <string>
#include <iomanip>
#include <ctime>
#include "Params.h"


// 遗传算法参数
struct GAParams {
    int population_size=100;     // 种群大小
    int num_clusters=5;        // 聚类数k
    int num_generations=300;     // 最大迭代次数
    double crossover_prob=0.65;   // 交叉概率
    double mutation_prob1=0.2;   // 交叉内置变异概率
    double mutation_prob2=0.05;   // 变异概率
    double alpha1=0.5;           // 空间距离权重
    double alpha2=0.5;           // 时间距离权重
    double k1=1;
    double k2=1.5;
    double k3=2;       // 时间距离惩罚系数
    int max_time_window=1670;     // 最大时间窗宽度，或者是1440
};

class VRPTWClustering {
private:
    Params& params;                // 问题参数
    GAParams paramsGA;                          // 遗传算法参数
    std::vector<std::vector<double>> spatial_dist=params.timeCost;  // 空间距离矩阵
    std::vector<std::vector<double>> temporal_dist;  // 时间距离矩阵
    std::vector<std::vector<double>> spatiotemporal_dist; // 时空距离矩阵

    // 计算时间节省值
    double calculateTimeSaving(double arrival_time, double c, double d) 
    {
        if (arrival_time < c) 
            return paramsGA.k2 * arrival_time + paramsGA.k1 * d - (paramsGA.k1 + paramsGA.k2) * c;
        else if (arrival_time <= d) 
            return -paramsGA.k1 * arrival_time + paramsGA.k1 * d;
        else 
            return -paramsGA.k3 * arrival_time + paramsGA.k3 * d;
    }

    // 计算时间距离
    double calculateTemporalDistance(int c1_id, int c2_id) 
    {
        Client from=params.cli[c1_id];
        Client to=params.cli[c2_id];
        double a_i = from.timeWindow[0];
        double b_i = from.timeWindow[1];
        double s_i = from.serviceDuration;
        double t_ij = spatial_dist[c1_id][c2_id]; // 行驶时间等于空间距离
        
        double a_prime = a_i + s_i + t_ij;  //针对每一个点来说
        double b_prime = b_i + s_i + t_ij;
        
        double c_j = to.timeWindow[0];
        double d_j = to.timeWindow[1];
        
        // 积分计算期望时间节省
        double integral = 0.0;
        double width = b_prime - a_prime;
        
        double t_zero=((paramsGA.k1 + paramsGA.k2) * c_j - paramsGA.k1 * d_j)/paramsGA.k2;

        if (b_prime+MY_EPSILON<t_zero || (a_prime >t_zero+MY_EPSILON && b_prime+MY_EPSILON < c_j)|| a_prime>d_j+MY_EPSILON || (a_prime >MY_EPSILON+ c_j && b_prime+MY_EPSILON < d_j))
            integral+=(calculateTimeSaving(a_prime, c_j, d_j)+calculateTimeSaving(b_prime,c_j,d_j))* width*0.5; // 全部早到
        else if ((a_prime+MY_EPSILON < t_zero && b_prime+MY_EPSILON <c_j))
            integral+=(calculateTimeSaving(b_prime,c_j,d_j)*(b_prime-t_zero)*0.5+ calculateTimeSaving(a_prime,c_j,d_j)*(t_zero-a_prime))*0.5; // 部分早到
        else if ((a_prime+MY_EPSILON < t_zero && b_prime+MY_EPSILON <d_j))
            integral+=calculateTimeSaving(a_prime,c_j,d_j)*(t_zero-a_prime)*0.5+(paramsGA.k1*(d_j-c_j)*(c_j-t_zero)*0.5)+
            (b_prime-c_j)*(paramsGA.k1*(d_j-c_j)+calculateTimeSaving(b_prime,c_j,d_j))*0.5;
        else if ((a_prime+MY_EPSILON < t_zero && b_prime>d_j+MY_EPSILON))
            integral+=calculateTimeSaving(a_prime,c_j,d_j)*(t_zero-a_prime)*0.5+(paramsGA.k1*(d_j-c_j)*(c_j-t_zero)*0.5)+
            (d_j-c_j)*paramsGA.k1*(d_j-c_j)*0.5+(b_prime-d_j)*calculateTimeSaving(b_prime,c_j,d_j)*0.5;
        else if (a_prime>t_zero+MY_EPSILON && b_prime+MY_EPSILON<d_j)
            integral+=((paramsGA.k1*(d_j-c_j)+calculateTimeSaving(a_prime,c_j,d_j))*(c_j-a_prime)*0.5)
            +((paramsGA.k1*(d_j-c_j)+calculateTimeSaving(b_prime,c_j,d_j))*(b_prime-c_j)*0.5);
        else if (a_prime>t_zero+MY_EPSILON && b_prime>d_j+MY_EPSILON)
            integral+=((paramsGA.k1*(d_j-c_j)+calculateTimeSaving(a_prime,c_j,d_j))*(c_j-a_prime)*0.5)
            +(paramsGA.k1*(d_j-c_j)*(d_j-c_j)*0.5)+((b_prime-d_j)*calculateTimeSaving(b_prime,c_j,d_j)*0.5);
        else if (a_prime > c_j+MY_EPSILON && b_prime>d_j+MY_EPSILON)
            integral+=(d_j-a_prime)*calculateTimeSaving(a_prime,c_j,d_j)*0.5+(b_prime-d_j)*calculateTimeSaving(b_prime,c_j,d_j)*0.5;
    
        double expected_saving = integral / width;
        double temporal_distance = paramsGA.k1 * paramsGA.max_time_window - expected_saving;
        return temporal_distance;
    }

    // 计算无向时间距离
    double calculateUndirectedTemporalDistance(int i, int j) 
    {
        double d_ij = calculateTemporalDistance(i, j);
        double d_ji = calculateTemporalDistance(j, i);
        return std::max(d_ij, d_ji);
    }

    // 归一化处理
    double normalizeValue(double value, double min_val, double max_val) 
    {
        if (max_val == min_val) return 0.0;
        return (value - min_val) / (max_val - min_val);
    }

    // 染色体类
    class Chromosome 
    {
    public:
        std::vector<int> medoids;  // 中心点索引列表
        double fitness;            // 适应度
        double objective_value;    // 目标函数值（距离和）

        Chromosome(int k) : medoids(k) {}

        // 计算目标函数值和适应度
        void evaluate(const VRPTWClustering& clustering) 
        {
            const auto& st_dist = clustering.spatiotemporal_dist;
            int n = clustering.params.nbClients;
            double total_distance = 0.0;
            
            for (int i = 1; i <= n; i++) 
            {
                double min_dist = INFINITY;
                for (int m : medoids) 
                {
                    min_dist = std::min(min_dist, st_dist[i][m]);
                }
                total_distance += min_dist;
            }
            
            objective_value = total_distance;   //总距离越小越好
            fitness = 1.0 / (total_distance + 1e-6); // 适应度越大越好
        }
    };

    // 生成初始种群
    std::vector<Chromosome> initializePopulation() 
    {
        int n = params.nbClients;
        std::vector<Chromosome> population(paramsGA.population_size, Chromosome(paramsGA.num_clusters));
        
        std::uniform_int_distribution<int> dist(1, n); // 客户索引从1开始（depot为0）
        
        for (auto& chrom : population) 
        {
            std::vector<int> used;
            for (int i = 0; i < paramsGA.num_clusters; i++) 
            {
                int medoid;
                do {
                    medoid = dist(params.ran);  //抽
                } while (std::find(used.begin(), used.end(), medoid) != used.end());
                chrom.medoids[i] = medoid;      //中心放进去
                used.push_back(medoid);
            }
            chrom.evaluate(*this);  //类成员函数*this表示本身
        }
        return population;
    }

    // 轮盘赌选择
    Chromosome selection(const std::vector<Chromosome>& population) 
    {
        double total_fitness = 0.0;
        for (const auto& chrom : population) 
        {
            total_fitness += chrom.fitness;
        }
        
        std::uniform_real_distribution<double> dist(0.0, total_fitness);
        double r = dist(params.ran);
        double sum = 0.0;
        
        for (const auto& chrom : population) 
        {
            sum += chrom.fitness;
            if (sum >= r) 
            {
                return chrom;
            }
        }
        
        return population[0];
    }

    // D_MX交叉算子
    Chromosome crossover(const Chromosome& parent1, const Chromosome& parent2) 
    {
        Chromosome child(paramsGA.num_clusters);
        std::vector<int> used;
        
        // 随机选择交叉点
        std::uniform_int_distribution<int> dist(1, paramsGA.num_clusters - 1);
        int crossover_point = dist(params.ran);
        
        // 继承父代1的前半部分
        for (int i = 0; i < crossover_point; i++) 
        {
            child.medoids[i] = parent1.medoids[i];
            used.push_back(parent1.medoids[i]);
        }
        
        // 从父代2继承不重复的后半部分
        for (int i = crossover_point; i < paramsGA.num_clusters; i++) 
        {
            for (int medoid : parent2.medoids) 
            {
                if (std::find(used.begin(), used.end(), medoid) == used.end()) 
                {
                    child.medoids[i] = medoid;
                    used.push_back(medoid);
                    break;
                }
            }
        }
        
        // 内置变异（概率p_m1）
        std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
        if (prob_dist(params.ran) < paramsGA.mutation_prob1) 
        {
            mutate(child);
        }
        
        child.evaluate(*this);
        return child;
    }

    // 变异操作
    void mutate(Chromosome& chrom) 
    {
        int n = params.nbClients; // 忽略depot
        std::vector<int> all_customers;
        for (int i = 1; i <= n; i++) 
        {
            all_customers.push_back(i);
        }
        
        // 找到非中心点客户
        std::vector<int> non_medoids;
        for (int c : all_customers) 
        {
            if (std::find(chrom.medoids.begin(), chrom.medoids.end(), c) == chrom.medoids.end()) 
            {
                non_medoids.push_back(c);
            }
        }
        
        if (non_medoids.empty()) return; // 无可用非中心点，不变异
        
        // 随机选择一个中心点替换
        std::uniform_int_distribution<int> medoid_dist(0, paramsGA.num_clusters - 1);
        int medoid_idx = medoid_dist(params.ran);
        
        std::uniform_int_distribution<int> non_medoid_dist(0, non_medoids.size() - 1);
        int non_medoid_idx = non_medoid_dist(params.ran);
        
        chrom.medoids[medoid_idx] = non_medoids[non_medoid_idx];    //把原来那个中心点拿出来
    }

public:
    VRPTWClustering(Params& params): params(params){};

    // 计算距离矩阵
    void calculateDistanceMatrices() 
    {
        int n = params.nbClients;
        temporal_dist.resize(n+1, std::vector<double>(n+1, 0.0));
        spatiotemporal_dist.resize(n+1, std::vector<double>(n+1, 0.0));
        
        // 计算时间距离矩阵
        for (int i = 1; i <= n; i++) 
        {
            for (int j = i; j <= n; j++) 
            {
                temporal_dist[i][j] = temporal_dist[j][i] = calculateUndirectedTemporalDistance(i, j);
            }
        }
        
        // 归一化处理
        double min_s = 1e18, max_s = -1e18;
        double min_t = 1e18, max_t = -1e18;
        
        for (int i = 1; i <= n; i++) 
        {
            for (int j = 1; j <= n; j++) 
            {
                if (i != j) {
                    min_s = std::min(min_s, spatial_dist[i][j]);
                    max_s = std::max(max_s, spatial_dist[i][j]);
                    min_t = std::min(min_t, temporal_dist[i][j]);
                    max_t = std::max(max_t, temporal_dist[i][j]);
                }
            }
        }
        
        // 计算时空距离矩阵
        for (int i = 1; i <= n; i++) 
        {
            for (int j = 1; j <= n; j++) 
            {
                if (i == j) 
                {
                    spatiotemporal_dist[i][j] = 0.0;
                } 
                else 
                {
                    double norm_s = normalizeValue(spatial_dist[i][j], min_s, max_s);
                    double norm_t = normalizeValue(temporal_dist[i][j], min_t, max_t);
                    spatiotemporal_dist[i][j] = paramsGA.alpha1 * norm_s + paramsGA.alpha2 * norm_t;
                }
            }
        }
    }

    // 执行遗传算法聚类
    std::vector<int> performClustering() 
    {
        std::vector<Chromosome> population = initializePopulation();
        Chromosome best_chrom(paramsGA.num_clusters);
        best_chrom.objective_value = INFINITY;
        
        for (int gen = 0; gen < paramsGA.num_generations; gen++) 
        {
            std::vector<Chromosome> new_population;
            
            // 精英保留
            Chromosome elite = population[0];   //排序
            for (const auto& chrom : population) 
            {
                if (chrom.objective_value < elite.objective_value) 
                {
                    elite = chrom;
                }
            }
            new_population.push_back(elite);
            
            // 生成新种群
            while (new_population.size() < paramsGA.population_size) 
            {
                Chromosome parent1 = selection(population);
                Chromosome parent2 = selection(population);
                
                // 交叉
                std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
                Chromosome child = (prob_dist(params.ran) < paramsGA.crossover_prob) ? 
                                   crossover(parent1, parent2) : parent1;
                
                // 变异
                if (prob_dist(params.ran) < paramsGA.mutation_prob2) 
                {
                    mutate(child);
                }
                
                child.evaluate(*this);
                new_population.push_back(child);
            }
            
            population = new_population;
            
            // 更新最佳解
            Chromosome current_best = population[0];
            for (const auto& chrom : population) 
            {
                if (chrom.objective_value < current_best.objective_value) 
                {
                    current_best = chrom;
                }
            }
            
            if (current_best.objective_value < best_chrom.objective_value) 
            {
                best_chrom = current_best;
            }
            
            // 输出每代的最优解（可选）
            if (gen % 10 == 0) 
            {
                std::cout << "Generation " << gen << ": Best objective = " 
                          << best_chrom.objective_value << std::endl;
            }
        }
        
        // 返回最优解的中心点索引
        return best_chrom.medoids;
    }
    
    // 分配客户到各个簇
    std::vector<std::vector<int>> assignCustomersToClusters(const std::vector<int>& medoids) 
    {
        std::vector<std::vector<int>> clusters(paramsGA.num_clusters);
        
        // 为每个客户分配到最近的中心点所在的簇
        for (int i = 1; i <= params.nbClients; i++) 
        {  // 跳过depot（索引0）
            double min_dist = INFINITY;
            int best_cluster = -1;
            
            for (int k = 0; k < paramsGA.num_clusters; k++) 
            {
                int medoid = medoids[k];
                double dist = spatiotemporal_dist[i][medoid];
                if (dist < min_dist) 
                {
                    min_dist = dist;
                    best_cluster = k;
                }
            }
            
            if (best_cluster != -1) 
            {
                clusters[best_cluster].push_back(i);
            }
        }
        
        return clusters;
    }
    
    // 输出聚类结果
    void printClusteringResults(const std::vector<int>& medoids, 
                                const std::vector<std::vector<int>>& clusters) 
    {
        std::cout << "\n===== 聚类结果 =====" << std::endl;
        
        for (int k = 0; k < paramsGA.num_clusters; k++) 
        {
            std::cout << "簇 " << k + 1 << " (中心点: 客户" << medoids[k] << "): ";
            std::cout << "客户数量 = " << clusters[k].size() << ", 成员 = [";
            
            for (int i = 0; i < clusters[k].size(); i++) 
            {
                std::cout << clusters[k][i];
                if (i < clusters[k].size() - 1) std::cout << ", ";
            }
            
            std::cout << "]" << std::endl;
        }
    }
};

#endif // DISANDTIMECLUSTER_H