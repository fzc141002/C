#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
#include<string>
using namespace std;

class baseDrink
{
public:
	virtual void Boil() = 0;		//煮水
	virtual void Brew() = 0;		//冲泡
	virtual void Pour() = 0;		//倒入杯子
	virtual void Putin() = 0;		//倒入一些佐料
	virtual void makeDrink()	//制作饮品
	{
		Boil();
		Brew();
		Pour();
		Putin();
	}
};

class makeCaffe:public baseDrink
{
public:
	virtual void Boil()		//煮水
	{
		cout << "煮水" << endl;
	}
	virtual void Brew()		//冲泡
	{
		cout << "冲泡" << endl;
	}
	virtual void Pour()		//倒入杯子
	{
		cout << "倒入杯子" << endl;
	}
	virtual void Putin()		//倒入一些佐料
	{
		cout << "加入佐料" << endl;
	}
};

void doDrink(baseDrink* bd)
{
	bd->makeDrink();
	delete bd;		//不要出现内存释放
}

void test()
{
	doDrink(new makeCaffe);		//注意需要设置子对象
	//相当于baseDrink* bd=new makeCaffe，这个指针在函数调用结束之后要手动销毁
}

int main()
{
	
}
