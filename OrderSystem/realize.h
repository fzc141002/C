#define _CRT_SECURE_NO_WARNINGS 1
#pragma once
#include<iostream>
#include<string>
#include <iomanip>
#include<stdio.h>
using namespace std;
#define Max_menu 1000		//定义菜单的最大上限
#define Max_order 1000		//定义最大的点餐上限
#define Max_con 1000		//系统中最多存储1000个订单
#define Max_user 1000		//定义系统中做多存储多少个用户信息

class Register;
int showEnter(string& u_id, Register r1);
void showOrder();
void showManage();

struct userMessage
{
	string use_now[Max_user][2];	//定义客户信息，第一维是用户名，第二维是密码
	int num_user;
};


class Register
{
public:
	void Relog_in(int judge_user);	//这个相当于设置一个注册的函数
	int Check_in(string& inner_name, int judge_user);		//进行初始化
	void Change_Information(int judge_user);
	void Cancel_Infomation(int judge_user);
	userMessage USER[2] = { {
	{{"1111","2222"},
	{"2222","3333"},
	{"3333","4444"},
	{"4444","5555"}
	},4 },
	{{{"1234","1234"}}
		,1} };								//在这里面定义客户信息
	//再定义一个客户
};



struct Vegetable
{
	string m_name;				//定义菜的名字
	float m_price[2];				//定义价格
};

struct Menu_now					//定义一下当前菜单
{
	Vegetable m_menu[Max_menu];
	int m_num = 5;				//定义菜单当前菜的数量
};

struct V_order				//定义点餐的时候包含的东西
{
	string v_name;			//分别定义菜的名字，规模和数量
	int v_size = 0;
	int v_num = 0;
	float vz_price = 0;
};

struct order_Menu
{
	V_order order_list[Max_order];		//第一个维度是种类，第二个维度保存的是份数
	Vegetable v_menu[Max_menu];
	int order_num = 0;				//定义点餐的数量
	float z_price = 0;			//定义当前的总价，未赋初值
	string order_name;			//定义一个订单名字
};

class do_Menu
{
public:
	order_Menu new_Menu;		//定义一个新的菜单
	Menu_now mm = { {
				{"水煮白菜",{5.0,10.0}},
				{"红烧肉",{25.0,50.0}},
				{"剁椒鱼肉",{50.0,100.0}},
				{"荔枝肉",{30.0,40.0}},
				{"清蒸鲈鱼",{70.0,100.0}}
				},5 };				//当前现有的菜单，菜单直接写死在这

public:
	void show_Menu();
	int find_V(string name, int m_num);			//定义一个找菜的函数
	int find_M(string name, int order_num);			//定义一个在菜单里寻找的函数
	void order_add(string name, int size, int num);		//点一次单
	void order_change(string name);		//如果是修改的话就是改变数量和规格
	void order_delete(string name);
	void order();			//进入重复点单的模式
};

class CheckOut;		//先告诉编译器后面会写这个类
class score_Menu;		//提前定义
//进入订单管理的模式，查看消费历史
class OrderManage
{

private:
	order_Menu order_con[Max_con];		//需要生成订单
	int order_num;						//定义当前保存了多少个订单

public:
	CheckOut* check_out;
	order_Menu& get_inner(order_Menu& cus_Menu, int order_id);
	do_Menu* inner_menu;
	void genOrder(order_Menu m_inner);		//最后的订单还需要删除，注意在这个函数
	void showOrder(string number);
	int findOrder(string number);		//寻找订单是否在订单集合中
	void changeOrder(string number);
	void delOrder(string number);
	void do_order(OrderManage* orders, score_Menu* scores);		//进入订单管理系统
};

class CheckOut
{
public:
	order_Menu cus_history[Max_con];
	int order_num;
	float check_sum(OrderManage* orders);	//消费结束之后需要在订单管理系统中删除这个订单
	void find_order(OrderManage* orders);
	void empty_order();
};

class score_Menu		//对于菜单进行评分，顾客针对自己已经点的菜单进行评分
{
public:
	Menu_now score_m;
	OrderManage* order_m;
	do_Menu menu_f;		//这边相当于没有赋初值，直接就使用了，如果是一个指针，那就相当于是一个野指针
	float pingfen[Max_menu][2];
	order_Menu own_order;
	score_Menu();
	void cus_score();
	void showScore();
};
