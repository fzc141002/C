#define _CRT_SECURE_NO_WARNINGS 1
#include "test.h"


void ownPrint(const string title)
{
	cout << "********" << title << "********" << endl;
}

void showMenu()		//定义函数体
{
	string a[6] = { "1.添加联系人","2.搜索联系人","3.删除联系人",
		"4.修改联系人","5.清空联系人","0.退出通讯录" };
	cout<< "****************************" <<endl;
	for (int i = 0; i < 6; i++)
	{
		ownPrint(a[i]);
	}
	cout << "****************************" << endl;
}

void addPerson(Menu* abs)		//n表示通讯录中有几个
{
	//添加联系人
	if (abs->m_Size >= Max)
	{
		cout << "通讯录已经满了" << endl;
	}
	else
	{
		Person new_p;
		cout << "请输入新联系人的姓名" << endl;
		cin >> new_p.name;
		cout << "请选择性别1.男  2.女" << endl;
		while (true)
		{
			cin >> new_p.sex;
			if (new_p.sex != 1 and new_p.sex != 2)
			{
				cout << "请输入1或者2" << endl;
			}
			else
			{
				break;
			}
		}
		cout << "请输入新联系人的年龄" << endl;
		cin >> new_p.age;
		cout << "请输入新联系人的电话" << endl;
		while (true)
		{
			cin >> new_p.tel;
			if (new_p.tel.size() != 11 )
			{
				cout << "请输入11位的电话号码" << endl;
			}
			else
			{
				break;
			}
		}
		cout << "请输入新联系人的住址" << endl;
		cin >> new_p.address;
		abs->personArray[abs->m_Size] = new_p;
	}
	abs->m_Size++;
	cout << "添加成功" << endl;
	//添加成功之后需要清屏
	system("pause");
	system("cls");
}

int isExist(Menu* abs)
{
	if (abs->m_Size == 0)
	{
		cout << "当前记录为空" << endl;
	}
	int inner = 0;
	cout << "请选择按照什么方式查询1.姓名 2.电话" << endl;
	cin >> inner;
	if (inner == 1)
	{
		for (int i = 0; i < abs->m_Size; i++)
		{
			cout << "输入查询名字" << endl;
			string inner_name;
			cin >> inner_name;
			if (inner_name == abs->personArray[i].name)
			{
				showPerson(abs->personArray[i]);
				return i;		//返回对应的人在通讯录中的位置
			}
		}
	}
	else
	{
		for (int i = 0; i < abs->m_Size; i++)
		{
			cout << "输入查询电话" << endl;
			string inner_tel;
			cin >> inner_tel;
			if (inner_tel == abs->personArray[i].tel)
			{
				showPerson(abs->personArray[i]);
				return i;
			}
		}
	}
	return -1;
}

void showPerson(Person man)
{
	cout << "姓名为" << man.name << "\t";
	cout << "性别为" << (man.sex==1 ? "男":"女") << "\t";	//展示男生女生，通过选择语句，注意在合适的地方用括号
	cout << "年龄为" << man.age << "\t";
	cout << "电话为" << man.tel << "\t";
	cout << "地址为" << man.address << endl;
}

void findPerson(Menu* abs)		//根据输入的姓名或者电话查询
{
	int find_is = isExist(abs);
	if (find_is >= 0)
	{
		cout << "找到了联系人" << endl;
	}
	else
	{
		cout << "没找到联系人" << endl;
	}
	system("pause");
	system("cls");
}

void delPerson(Menu* abs)
{
	int find_is = isExist(abs);			//查找这个人
	if (find_is >= 0)
	{
		cout << "找到了联系人" << endl;
		for (int i = find_is; i < abs->m_Size; i++)
		{
			abs->personArray[i] = abs->personArray[i + 1];
		}
		abs->m_Size--;
		cout << "删除成功" << endl;
	}
	else
	{
		cout << "没找到联系人" << endl;
	}
	system("pause");
	system("cls");
}

void modifyPerson(Menu* abs)
{
	int find_is = isExist(abs);			//查找这个人
	if (find_is >= 0)
	{
		cout << "请输入姓名" << endl;
		cin >> abs->personArray[find_is].name;
		cout << "请输入性别" << endl;
		cin >> abs->personArray[find_is].sex;
		cout << "请输入年龄" << endl;
		cin >> abs->personArray[find_is].age;
		cout << "请输入电话" << endl;
		cin >> abs->personArray[find_is].tel;
		cout << "请输入地址" << endl;
		cin >> abs->personArray[find_is].address;
		cout << "修改成功" << endl;
	}
	else
	{
		cout << "没找到联系人" << endl;
	}
	system("pause");
	system("cls");
}

void delMenu(Menu* abs)
{
	abs->m_Size = 0;
	cout << "通讯录已经清空" << endl;
	system("pause");
	system("cls");
}
