#define _CRT_SECURE_NO_WARNINGS 1
#include"实现.h"

int showEnter(string& u_id,Register r1)
{
	while (true)
	{
		int choose;
		cout << "1.顾客登录  2.员工登录  3.注册  4.修改信息  5.注销账户  6.菜单评分" << endl;
		cout << "请选择系统登录" << endl;
		cin >> choose;

		switch (choose)
		{
		case 1:
		{
			cout << "******顾客系统******" << endl;
			int sym;
			sym = r1.Check_in(u_id, 0);		//注意0是客户，1是商家
			if (sym == 1)
			{
				return 1;			//1的话就是顾客
			}
			else
			{
				return 0;
			}
		}
		case 2:
		{
			cout << "******员工系统******" << endl;		//是否需要登录
			int sym;
			sym = r1.Check_in(u_id, 1);		//注意0是客户，1是商家
			if (sym == 1)
			{
				return 2;			//2的话就是商家
			}
			else
			{
				return 0;
			}
			return 2;
		}
		case 3:
		{
			cout << "******新客注册******" << endl;
			r1.Relog_in(0);				//直接当成员工注册
			return 0;
		}
		case 4:
		{
			cout << "******修改信息******" << endl;
			r1.Change_Information(0);
			return 0;
		}
		case 5:
		{
			cout << "******注销账户******" << endl;
			r1.Cancel_Infomation(0);
			return 0;
		}
		case 6:
		{
			cout << "******菜单评分******" << endl;
			return 3;
		}
		default:
			system("pause");
			system("cls");
			break;
		}
		cin.clear();		//清空缓存区
		cin.ignore();
	}
}

void showOrder()
{
	cout << "1.添加菜品\t" << endl;
	cout << "2.修改菜品\t" << endl;
	cout << "3.删除菜品\t" << endl;
	cout << "4.查看菜单\t" << endl;
	cout << "0.点菜结束\t" << endl;
	cout << "请选择菜单功能" << endl;
}

void Register::Relog_in(int judge_user)		//用户进行注册
{
	if (USER[judge_user].num_user < Max_user)
	{
		string name;
		string password;
		cout << "新客注册，开启你的美食之旅吧" << endl;
		cout << "请输入用户名" << endl;
		cin >> name;
		cout << "请输入复杂一点的密码" << endl;
		cin >> password;
		USER[judge_user].use_now[USER[judge_user].num_user][0] = name;
		USER[judge_user].use_now[USER[judge_user].num_user][1] = password;
		USER[judge_user].num_user++;		//用户需要加一个
	}
	else
	{
		cout << "当前系统用户已经注册满了" << endl;
	}
}

void Register::Cancel_Infomation(int judge_user)
{
	string inner_ues;
	cout << "请输入需要注销的用户名" << endl;
	cin >> inner_ues;
	for (int i = 0; i < USER[judge_user].num_user; i++)
	{
		if (inner_ues == USER[judge_user].use_now[i][0])	//如果有个用户名的话
		{
			string inner_pass;
			cout << "请输入密码" << endl;
			cin >> inner_pass;
			if (inner_pass == USER[judge_user].use_now[i][1])
			{
				cout << "确定注销这个账户吗？1.确定  2.取消" << endl;
				if (getchar() == 1)
				{
					if (USER[judge_user].num_user == 1)		//如果只有一个用户的话
					{
						USER[judge_user].num_user--;		//直接减掉就可以了
					}
					else
					{
						for (int j = i; j < USER[judge_user].num_user - 1; j++)
						{
							USER[judge_user].use_now[j][0] = USER[judge_user].use_now[j + 1][0];
							USER[judge_user].use_now[j][1] = USER[judge_user].use_now[j + 1][1];
						}
						USER[judge_user].num_user--;
					}					
				}
				else
				{
					return;
				}
			}
		}
	}
}

int Register::Check_in(string& inner_name, int judge_user)	//进行初始化
{
	string name;
	string password;
	int try_time = 0;
	while (try_time < 5)
	{
		cout << "请输入用户名" << endl;
		cin >> name;
		cout << "请输入密码" << endl;
		cin >> password;
		for (int i = 0; i < USER[judge_user].num_user; i++)
		{
			if ((name == USER[judge_user].use_now[i][0]) and (password == USER[judge_user].use_now[i][1]))
			{
				cout << "登陆成功" << endl;
				inner_name = name;		//给用户名保存下来
				return 1;
			}
			else
			{
				cout << "用户名或密码错误，请重新输入" << endl;
				break;
			}
		}
		try_time++;
	}
	cout << "尝试次数过多，系统已锁定" << endl;
	return 0;
}

void Register::Change_Information(int judge_user)
{
	string name;
	string password;
	while (true)
	{
		cout << "请输入用户名" << endl;
		cin >> name;
		for (int i = 0; i < USER[judge_user].num_user; i++)
		{
			if (name == USER[judge_user].use_now[i][0])
			{
				cout << "请输入原始密码" << endl;
				cin >> password;
				if (password == USER[judge_user].use_now[i][1])
				{
					string inner_pass;
					cout << "请输入修改后的密码" << endl;
					cin >> inner_pass;
					string inner1_pass;
					cout << "请再次输入密码" << endl;
					cin >> inner1_pass;
					if (inner_pass == inner1_pass)
					{
						USER[judge_user].use_now[i][1] = inner1_pass;	//修改密码
						return;
					}
				}
			}
			else
			{
				cout << "密码错误,请重新输入" << endl;
				break;
			}
		}
		cout << "用户名错误或者此用户未注册" << endl;
		cout << "1.继续修改  2.退出" << endl;
		if (getchar() == '2')
		{
			return;
		}
	}
}


void do_Menu::show_Menu()
{
	cout << std::left << setw(14) << "菜名"
		<< std::left << setw(14) << "规格"
		<< std::left << setw(14) << "份数"	
		<< std::left << setw(14) << "价格" << endl;
	for (int i = 0; i < new_Menu.order_num; i++)
	{
		cout << std::left << setw(14)<<new_Menu.order_list[i].v_name<<
			std::left << setw(14) << (new_Menu.order_list[i].v_size > 0 ? "大份" : "小份")<<
			std::left << setw(14) << new_Menu.order_list[i].v_num <<
			std::left << setw(14) << new_Menu.order_list[i].vz_price  << endl;
	}
	cout << "当前总价格为:" << new_Menu.z_price << "元" << endl;
	system("pause");
	system("cls");			///清屏
}

int do_Menu::find_V(string name, int m_num)			//定义一个找菜的函数
{
	for (int i = 0; i < m_num; i++)		//定义为菜单的上限
	{
		if (name == mm.m_menu[i].m_name)		//如果找到了相同的菜单的话
		{
			return i;					//返回在菜单的某一个位置
		}
	}
	return -1;
}

int do_Menu::find_M(string name, int order_num)			//定义一个找菜的函数
{
	for (int i = 0; i < order_num; i++)		//定义为菜单的上限
	{
		if (name == new_Menu.order_list[i].v_name)		//如果找到了相同的菜单的话
		{
			return i;
		}
	}
	return -1;
}

void do_Menu::order_add(string name, int size, int num)		//点一次单
{
	int sym = find_V(name, mm.m_num);
	if (sym >= 0)
	{
		if (new_Menu.order_num < Max_order)	//未超过最大点单量的话
		{
			new_Menu.v_menu[new_Menu.order_num] = mm.m_menu[sym];	//这个相当于是创一个自己点的临时菜单
			new_Menu.order_list[new_Menu.order_num].v_name = name;
			new_Menu.order_list[new_Menu.order_num].v_num = num;
			new_Menu.order_list[new_Menu.order_num].v_size = size;
			new_Menu.order_list[new_Menu.order_num].vz_price = mm.m_menu[sym].m_price[size];
			new_Menu.z_price = new_Menu.z_price + mm.m_menu[sym].m_price[size];
			new_Menu.order_num++;
			cout << name << "添加成功" << endl;
			show_Menu();		//展示一下菜单
		}
	}
	else
	{
		cout << "该菜品当前没货" << endl;
		system("pause");
		system("cls");
	}
}

void do_Menu::order_change(string name)		//如果是修改的话就是改变数量和规格
{
	int sym = find_M(name, new_Menu.order_num);
	if (sym >= 0)
	{
		cout << "原来的份数为" << new_Menu.order_list[sym].v_num << endl;
		cout << "现在需要的份数为" << endl;
		cin >> new_Menu.order_list[sym].v_num;

		cout << "原来的规格为" << new_Menu.order_list[sym].v_size << endl;
		cout << "现在需要的规格为0.小份  1.大份" << endl;
		cin >> new_Menu.order_list[sym].v_size;

		new_Menu.z_price -= new_Menu.order_list[sym].vz_price;
		new_Menu.order_list[sym].vz_price = new_Menu.order_list[sym].v_num * new_Menu.v_menu[sym].m_price[new_Menu.order_list[sym].v_size];
		new_Menu.z_price += new_Menu.order_list[sym].vz_price;		//价格变化
		cout << "修改成功" << endl;
		show_Menu();
	}
	else
	{
		int choose = 0;
		cout << "没有点这个菜，需要添加吗？输入1.需要  2.不需要" << endl;
		cin >> choose;
		if (choose == 1)
		{
			int n_size;
			int n_num;
			cout << name << "需要规格为0.小份  1.大份：";
			cin >> n_size;
			cout << name << "需要份数为：";
			cin >> n_num;
			order_add(name, n_size, n_num);
		}
		else
		{
			show_Menu();
		}
	}
}

void do_Menu::order_delete(string name)
{
	int sym = find_M(name, new_Menu.order_num);
	if (sym >= 0)
	{
		if (new_Menu.order_num == 1)
		{
			new_Menu.order_num--;		//直接减掉就可以了
			new_Menu.z_price = 0;
			show_Menu();
		}
		else
		{
			for (int i = sym; i < new_Menu.order_num - 1; i++)
			{
				new_Menu.z_price -= new_Menu.order_list[i].vz_price;	//减去需要删除的价格
				new_Menu.order_list[i] = new_Menu.order_list[i + 1];
				new_Menu.v_menu[i] = new_Menu.v_menu[i + 1];	
			}
			new_Menu.order_num--;
			show_Menu();
		}
	}
	else
	{
		int choose = 0;
		cout << "没有点这个菜，需要添加吗？输入1.需要  2.不需要" << endl;
		cin >> choose;
		if (choose == 1)
		{
			int n_size;
			int n_num;
			cout << name << "需要规格为0.小份  1.大份：";
			cin >> n_size;
			cout << name << "需要份数为：";
			cin >> n_num;
			order_add(name, n_size, n_num);
		}
		else
		{
			show_Menu();
		}
	}
}

void do_Menu::order()			//进入重复点单的模式
{
	while (true)
	{
		int f_choose = 0;
		showOrder();			//展示一下功能选项
		cin >> f_choose;
		switch (f_choose)
		{
		case 1:
		{
			string d_cai;
			int d_size;
			int d_num;
			cout << "需要点什么菜" << endl;
			cin >> d_cai;
			cout << "要点多大份的菜0.小份  1.大份：" << endl;
			cin >> d_size;
			cout << "需要点多少份菜" << endl;
			cin >> d_num;
			order_add(d_cai, d_size, d_num);
			break;
		}
		case 2:
		{
			string d_cai;
			cout << "需要修改什么菜" << endl;
			cin >> d_cai;
			order_change(d_cai);
			break;
		}
		case 3:
		{
			string d_cai;
			cout << "需要删掉什么菜" << endl;
			cin >> d_cai;
			order_delete(d_cai);
			break;
		}
		case 4:
		{
			show_Menu();		//展示一下菜单
			break;
		}
		case 0:
		{
			cout << "菜单已提交，欢迎继续点单" << endl;
			//设置菜单的名字
			return;
		}
		default:
			break;
		}
	}
}

void OrderManage::genOrder(order_Menu m_inner)		//最后的订单还需要删除，注意在这个函数里面赋值
	{
		order_con[order_num] = m_inner;
		order_num++;	//相当于进行一个赋值
	}

void OrderManage::showOrder(string number)
{
	int find_is = findOrder(number);
	if (find_is >= 0)
	{
		inner_menu->show_Menu();
	}
}

int OrderManage::findOrder(string number)		//寻找订单是否在订单集合中
{
	for (int i = 0; i < order_num; i++)
	{
		if (order_con[i].order_name == number)
		{
			cout << "找到编号为：" << number << "的订单" << endl;
			return i;
		}
	}
	cout<< "未找到编号为：" << number << "的订单" << endl;
	return -1;
}

void OrderManage::changeOrder(string number)
{
	int find_is = findOrder(number);
	if (find_is >= 0)
	{
		inner_menu->new_Menu = order_con[find_is];
		cout << "当前订单为" << endl;
		inner_menu->show_Menu();
		string d_cai;
		cout << "需要修改什么菜" << endl;
		cin >> d_cai;
		inner_menu->order_change(d_cai);		//修改菜单
		order_con[find_is] = inner_menu->new_Menu;	//把修改的菜单返还回去
		cout << "修改成功" << endl;
	}
}

void OrderManage::delOrder(string number)
{
	int find_is = findOrder(number);
	if (find_is >= 0)
	{
		if (order_num == 1)
		{
			order_num--;
		}
		else
		{
			for (int i = find_is; i < order_num; i++)
			{
				order_con[i] = order_con[i + 1];
			}
			order_num--;
		}
		cout << "删除成功" << endl;
	}
}

void showManage()
{
	cout << "1.展示订单\t" << endl;
	cout << "2.修改订单\t" << endl;
	cout << "3.删除订单\t" << endl;
	cout << "4.支付订单\t" << endl;
	cout << "5.展示菜单评分\t" << endl;
	cout << "0.退出管理系统\t" << endl;
	cout << "请选择操作选项" << endl;
}


void OrderManage::do_order(OrderManage* orders, score_Menu* scores)//进入商家操作系统
{
	while (true)
	{
		int choose = 0;			//最好还是可以赋一个初值
		showManage();
		cin >> choose;
		string cus_name;
		cout << "请输入需要操作的订单编号" << endl;
		cin >> cus_name;
		switch (choose)
		{
		case 1:
			showOrder(cus_name);
			break;
		case 2:
			changeOrder(cus_name);
			break;
		case 3:
			delOrder(cus_name); 
			break;
		case 4:
		{
			check_out->check_sum(orders);		//进行结账
			break;
		}
		case 5:
		{
			scores->showScore();				//展示一下当前菜单的评分
			break;
		}
		case 0:
		{
			cout << "****操作结束，退出系统****" << endl;
			return;
		}
		default:
			break;
		}
	}
}

order_Menu& OrderManage::get_inner(order_Menu& cus_Menu,int order_id)
{
	cus_Menu = order_con[order_id];
	return cus_Menu;
}

float CheckOut::check_sum(OrderManage* orders)
{
	order_Menu cus_order;
	string order_name;
	cout << "请输入当前的订单编号" << endl;
	cin >> order_name;
	int find_is = orders->findOrder(order_name);
	if (find_is >= 0)
	{
		cus_order = orders->get_inner(cus_order, find_is);
		cout << "当前订单总计" << cus_order.z_price << "元";
		orders->delOrder(order_name);
		cus_history[order_num] = cus_order;
		order_num++;
		return cus_order.z_price;
	}
	else
	{
		cout << "编号为" << order_name << "的订单不存在" << endl;
	}
}

void CheckOut::empty_order()
{
	order_num = 0;		//全部删除掉
}

void CheckOut::find_order(OrderManage* orders)
{
	string cus_name;
	cout << "请输入需要寻找的订单编号(用户名)" << endl;
	cin >> cus_name;
	int find_is = orders->findOrder(cus_name);
	if (find_is >= 0)
	{
		cout << "找到了编号为" << cus_name << "的订单" << endl;
		orders->showOrder(cus_name);
	}
	else
	{
		cout<< "未找到编号为" << cus_name << "的订单" << endl;
	}
}

score_Menu::score_Menu()
{
	score_m = menu_f.mm;		//把菜单赋值过去,结构体不能直接等于
}

void score_Menu::cus_score()
{
	cout << "请输入用户名" << endl;
	string cus_name;
	cin >> cus_name;
	int find_is = order_m->findOrder(cus_name);
	if (find_is >= 0)
	{
		own_order = order_m->get_inner(own_order, find_is);		//如果要接收引用的话，就必须要是一个引用
		for (int i = 0; i < own_order.order_num; i++)		//还需要知道这个菜的编号
		{
			int cai_id = menu_f.find_M(own_order.order_list[i].v_name, score_m.m_num);
			float inner_score = 0;
			cout << "0-5分请对菜品打分" << endl;
			cout << "对于" << own_order.order_list[i].v_name << "的评分为" << endl;
			cin >> inner_score;
			pingfen[cai_id][0] = (pingfen[cai_id][0] * pingfen[cai_id][1] + inner_score) / (pingfen[cai_id][1] + 1);
			pingfen[cai_id][1]++;	//多一个评分
		}
	}
	else
	{
		cout << "近期未查询到编号未" << cus_name << "的订单" << endl;
	}
}

void score_Menu::showScore()
{
	cout << std::left << setw(14) << "菜名"
		<< std::left << setw(14) << "评分"
		<< std::left << setw(14) << "打分人数" << endl;
	for (int i = 0; i < score_m.m_num; i++)
	{
		cout << std::left << setw(14) << score_m.m_menu[i].m_name <<
			std::left << setw(14) << pingfen[i][0] <<
			std::left << setw(14) << pingfen[i][1] << endl;
	}
}
