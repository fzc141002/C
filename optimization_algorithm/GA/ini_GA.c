#define _CRT_SECURE_NO_WARNINGS 1

#include"GA1.h"

//记得写一个通用的累加函数
int main()
{
	IniPop();	//初始化种群
	//Find();
	int cyc_num = 0;
 	while (cyc_num< GENERATOR_NUM)
	{
		select();
		crossover_and_variation();
		//Find();
		cyc_num++;
	}
	int sys_max = 0;
	double big = 0;
	for (int i = 0; i < INI_NUM; i++)
	{
		if (DNA_result[i].fitness > big)
		{
			big = DNA_result[i].fitness;
			sys_max = i;
		}
	}
	printf("最终的x,y为:%f  %f\n", DNA_result[sys_max].true_result[0], DNA_result[sys_max].true_result[1]);
	printf("最大适应度为:%f\n", DNA_result[sys_max].fitness);
}
