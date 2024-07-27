#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
#include<string>
using namespace std;

//写一个计算器的类,传统的方式写
class Calcualtor
{
public:

	int getResult(string oper)
	{
		if (oper == "+")		//符号判断
		{
			return m_Num1 + m_Num2;
		}
		else if (oper == "-")
		{
			return m_Num1 - m_Num2;
		}
		else if (oper == "*")
		{
			return m_Num1 * m_Num2;
		}
	}

	int m_Num1;
	int m_Num2;
};

class AbstractCalculator		//通过多态的方式写计算器，创建加减的子对象
{
public:
	virtual int getResult()
	{
		return 0;
	}

	int m_Num1;
	int m_Num2;
};

class AddC :public AbstractCalculator
{
public:

	int getResult()
	{
		return m_Num1 + m_Num2;
	}
};

class SubC :public AbstractCalculator
{
public:
	
	int getResult()		//重写函数的话，函数名称至少需要一样
	{
		return m_Num1 - m_Num2;
	}
};

void test()
{
	//父类的引用或者指针指向子对象
	AbstractCalculator* ac = new AddC;
	ac->m_Num1 = 10;
	ac->m_Num2 = 20;
	cout << "加法得到的结果为:" << ac->getResult() << endl;
	delete ac;	//创建在堆区的变量记得销毁
	ac = NULL;
}

int main()
{
	test();
}
