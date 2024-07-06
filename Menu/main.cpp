#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
using namespace std;
#include <string>
#include "test.h"



int main()
{
	//为了实现不同的功能，多选择用switch
	//初始创建通讯录
	Menu address_book;				//初始化一个通讯录的结构体
	address_book.m_Size = 0;		//定义初始长度为0

	while (true)
	{
		int choose = 0;
		showMenu();
		cout << "请选择功能输入" << endl;
		cin >> choose;

		switch (choose)
		{
		case 1:
			addPerson(&address_book);
			break;
		case 2:
			findPerson(&address_book);
			break;
		case 3:
			delPerson(&address_book);
			break;
		case 4:
			modifyPerson(&address_book);
			break;
		case 5:
			delMenu(&address_book);
			break;
		case 0:
			cout << "欢迎下次使用" << endl;
			system("pause");		//可以暂停一会
			return 0;
			break;
		default:
			break;
		}
	}
}
