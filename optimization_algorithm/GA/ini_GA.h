#define _CRT_SECURE_NO_WARNINGS 1

#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

#define INI_NUM 200
#define DNA_NUM 2
#define LOW 0
#define UP 10
#define DNA_BIT 16
#define INT_BIT 4
#define GENERATOR_NUM 100
#define CROSS_RATE 0.8
#define	VAR_RATE 0.005
#define LIMIT_LOW 0.8


struct Population 
{
	int x[DNA_BIT];
	int y[DNA_BIT];
}animal[INI_NUM], selected[INI_NUM];
typedef struct Population Pop;

struct DNA_results
{
	double true_result[DNA_NUM];
	double fitness;
}DNA_result[INI_NUM], D_selected[INI_NUM];
typedef struct DNA_results Final;

int Judge_in(int x, Final* d, int need);	//need表示是否需要转换，1是原件需要，0是不需要,2是备份需要
void IniPop();
double Fitness(double x, double y);
void DNA2to10(Pop* p, Final* d);
void select();
void crossover_and_variation();
void Find();
