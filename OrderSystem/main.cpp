#define _CRT_SECURE_NO_WARNINGS 1
#include<iostream>
#include<string>
#include <iomanip>
#include"实现.h"
using namespace std;

OrderManage order_ini;
Register r;		//定义一个登录
score_Menu s_menu;

int main()
{
	int statue_judge = 0;
	string user_id="";
	statue_judge=showEnter(user_id,r);		//登录系统，需要有一个返回值去判断
	//最好是还能返回一下客户名
	//登录之后进入点菜系统，需要区分是顾客还是员工
	system("pause");
	system("cls");		//这边也需要清屏一下
	switch (statue_judge)
	{
	case 1:		//1的话是客户
	{
		do_Menu cus_m;		//在这边定义一个菜单
		cus_m.order();
		cout << "当前菜单为" << endl;
		cus_m.show_Menu();
		cus_m.new_Menu.order_name = user_id;	//获得用户名有点问题
		//点完单之后，在订单管理系统当中生成订单
		order_ini.genOrder(cus_m.new_Menu);		//向系统中提交并且生成一个订单
		break;
	}
	case 2:		//2的话是商家，商家进入的应该是订单管理
		order_ini.do_order(&order_ini, &s_menu);
		break;
	case 3:		//3的话是进行菜单评分系统
		s_menu.cus_score();
		break;
	default:
		break;
	}
}
