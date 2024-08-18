#define _CRT_SECURE_NO_WARNINGS 1

#include<iostream>
using namespace std;
#include<string>

//class Person
//{
//	//可以通过调用共有部分的函数，来改变私有部分的参数
//public:
//	void Setname(string n)
//	{
//		name = n;
//	}
//	string Getname()
//	{
//		return name;
//	}
//private:
//	string name;
//	int age;
//	string lover;
//};
//
//int main()
//{
//
//	Person p1;
//	p1.Setname("fzc");
//	cout<<p1.Getname()<<endl;
//	return 0;
//}


//设计立方体类
//全局函数和成员函数
//class Cube
//{
//public:
//	void set_ele(double l = 0, double w = 0, double h = 0)
//	{
//		c_l = l;
//		c_w = w;
//		c_h = h;
//		cout << "该立方体的长宽高分别为:" << l << "\t" << w << "\t" << h << endl;
//	}
//	double calS()
//	{
//		double S= 2 * (c_l * c_w + c_l * c_w + c_w * c_h);
//		cout << "该立方体的面积为:" << S << endl;
//		return S;
//	}
//	double calV()
//	{
//		double V= c_l * c_w * c_h;
//		cout << "该立方体的体积为:" << V << endl;
//		return V;
//	}
//
//	bool judge(Cube& c)
//	{
//		if (c.c_h == c_h && c.c_w == c_w && c.c_l == c_l)
//		{
//			return true;
//		}
//		return false;
//	}
//private:
//	double c_l;
//	double c_w;
//	double c_h;
//};
//
//int main()
//{
//	Cube c1;
//	c1.set_ele(10, 10, 10);
//	c1.calS();
//	c1.calV();
//
//	Cube c2;
//	c2.set_ele();
//	cout<<c1.judge(c2)<<endl;
//	return 0;
//}



class Person
{
	//1.构造函数
	//没有返回值，前面也不用写void
	//函数名与类名相同
	//可以有参数，可以发生重载
	//创建对象的时候，构造函数自动调用，且只调用一次
public:	//在调用构造函数的时候，需要有public的作用域
	//1的大类中都是普通构造函数
	//1.1下面这个是无参构造(默认构造函数)
	Person()
	{
		cout << "Person的构造函数的调用" << endl;
	}

	//1.2下面这个是有参构造
	Person(int a,int h)
	{

		age = a;
		height = new int(h);
		cout << "Person的构造函数的调用" << endl;
	}
	
	//2拷贝构造函数
	Person(const Person& p)		//相当于把自己复制一份
	{
		age = p.age;
		height = new int (*p.height);	//因为new本身就是返回一个指针，所以一开始的指针要解引用
	}


	//2.析构函数
	//没有返回值，前面也不用写void
	//函数名与类名相同，前面加~
	//析构函数不能有参数，所以不会发生重载
	//对象在销毁前会调用析构，且只调用一次
	~Person()
	{
		//析构代码将堆区开辟的数据释放
		if (height != NULL)
		{
			delete height;
			height = NULL;
		}
		cout << "Person的析构函数调用" << endl;
	}


	int age;	//一开始构造一个年龄
	int* height;	//创建一个身高的指针，要建立在堆区
};

//调用构造函数
void test01()
{
	//1.括号法
	Person p1;	//默认构造函数的调用
	Person p2(10,180);	//有参构造，需要把参数直接输入
	Person p3(p2);	//拷贝构造函数，把之前构造的类传进去
	//注意调用默认构造的时候不要加（）  如果Person p1()这样会被默认为函数声明
	//2.显示法
	Person p1;
	Person p2 = Person(10,180);		//有参构造，注意Person(10)是一个匿名对象，等号左侧相当于取名
	//匿名对象执行玩马上就被系统销毁，马上就析构
	Person p3 = Person(p2);		//拷贝构造
	//不要利用拷贝构造函数初始化匿名对象
	//因为Person(p3)编译器会认为是一个对象的声明，直接变成Person p3，会重定义
	//3.隐式转换法，隐式转换赋初值的时候不能直接用，可以通过一个数组的方式传递进去
	Person p4 = { 10,180 };	//相当于Person p4=Person(10)
	Person p5 = p4;	//想当于直接传参
}

//使用一个已经创建完毕的对象来初始化一个新的对象
void test02()
{
	Person p1(20,180);
	Person p2(p1);
}

//值传递的方式给函数传递参数
void dowork(Person p)
{
	;	//空操作
}
void test03()
{
	Person p;
	dowork(p);	//在实参调用的过程中，会拷贝一份临时变量，但是不会影响原变量，相当于在传递的过程中重新建立了一个变量Person p1=p;
}

//3.值方式返回局部变量
Person dowork2()
{
	Person p;
	return p;	//返回的是一个新的对象
}
void test03()
{
	Person p = dowork2();	//返回的时候相当于也会复制
}
int main()
{
	Person p;	//如果把变量定义在这，那么析构函数出现在return 0之后，短暂出现

	int b = 8;
	int& c = b;
	cout << c << endl;
	return 0;
}
