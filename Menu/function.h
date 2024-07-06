#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
using namespace std;
#include <string>
#define Max 1000

void showMenu();

struct Person		//建立一个人的结构体
{
	string name;
	int sex;
	int age;
	string tel;
	string address;
};

struct Menu
{
	Person personArray[Max];
	int m_Size;	//当前通讯录中的人数
};

void addPerson(Menu* abs);
void findPerson(Menu* abs);
void showPerson(Person man);
void delPerson(Menu* abs);
void modifyPerson(Menu* abs);
void delMenu(Menu* abs);
