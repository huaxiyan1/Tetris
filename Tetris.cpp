#include <iostream>
#include <string>
#include <windows.h>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <vector>
using namespace std;
wstring a[7];
int width = 12, height = 18, Width = 80, Height = 30, piece = 0, score = 0, speed = 20;
unsigned char *p = NULL;
bool key[4];
vector<int>Lines;
bool fit(int type, int rotation, int X, int Y);
int rotate(int x, int y, int r);

int main() {
	cout << "   操作方法:用→↓←控制方块移动,按Z旋转方块\n" << endl;
	cout << "   计分规则:每个方块落地加25分;同时消去n行加 2的n次方乘100 分\n" << endl;
	cout << "   请确保输入法调为英文或关闭\n" << endl;
	cout << "   选择难度:按Q选择简单,按E选择困难\n" << endl;
	bool style = false;
	//选择模式
	while (!style) {
		this_thread::sleep_for(100ms); // 稍微暂停以避免过于频繁地检查按键状态
		if ((0x8000 & GetAsyncKeyState('Q')) != 0) {
			speed = 20;
			style = true;
		}
		if ((0x8000 & GetAsyncKeyState('E')) != 0) {
			speed = 10;
			style = true;
		}
	}
	cout << "   按下Z键开始游戏\n" << endl;
	bool gameStarted = false;
	// 按下“Z”键来开始游戏
	while (!gameStarted) {
		this_thread::sleep_for(100ms);
		if ((0x8000 & GetAsyncKeyState('Z')) != 0) {
			gameStarted = true;
		}
	}
	srand(static_cast<unsigned int>(time(0)));//srand(time(0));设置随机
	//方块设置
	a[0].append(L"..X...X...X...X.");
	a[1].append(L"..X..XX..X......");
	a[2].append(L".X...XX...X.....");
	a[3].append(L".....XX..XX.....");
	a[4].append(L"..X..XX...X.....");
	a[5].append(L".....XX...X...X.");
	a[6].append(L"..X...X..XX.....");
	//创建俄罗斯方块场地
	p = new unsigned char[width * height];
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			p[x + width * y] = (x == 0 || x == width - 1 || y == height - 1) ? 9 : 0;
		}
	}
	//创建整个屏幕缓冲区
	wchar_t *screen = new wchar_t[Width * Height];//wchar_t后续用于表示Score
	for (int i = 0; i < Width * Height; ++i) {
		screen[i] = ' ';
	}
	HANDLE console = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(console);//<windows.h>
	DWORD count = 0;
	bool GameOver = false, Hold = false, Down = false;
	//当前方块状态
	int Type = rand() % 7, Rotation = 0, CurrentX = width / 2 - 2, CurrentY = 0, counter = 0;
	while (!GameOver) {
		this_thread::sleep_for(50ms);//thread 20帧
		counter++;
		Down = (counter == speed);
		WriteConsoleOutputCharacterW(console, screen, Width *Height, {0, 0}, &count);//更新游戏界面
		for (int k = 0; k < 4; ++k)
			key[k] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28Z"[k]))) != 0; //Right Left Down Z
		//按键旋转处理
		CurrentX += (key[0] && fit(Type, Rotation, CurrentX + 1, CurrentY)) ? 1 : 0; //Right
		CurrentX -= (key[1] && fit(Type, Rotation, CurrentX - 1, CurrentY)) ? 1 : 0; //Left
		CurrentY += (key[2] && fit(Type, Rotation, CurrentX, CurrentY + 1)) ? 1 : 0; //Down
		if (key[3]) {
			Rotation += (!Hold&& fit(Type, Rotation + 1, CurrentX, CurrentY)) ? 1 : 0; //Z
			Hold = true;
		} else
			Hold = false;
		if (Down) {
			if (fit(Type, Rotation, CurrentX, CurrentY + 1))
				CurrentY++;
			else {
				for (int x = 0; x < 4; ++x) {
					for (int y = 0; y < 4; ++y) {
						if (a[Type][rotate(x, y, Rotation)] == L'X')
							p[(CurrentY + y) * width + (CurrentX + x)] = Type + 1;
					}
				}
				piece++;
				if (piece % 10 == 0 && speed >= 10) {
					speed -= 3;
				}
				for (int y = 0; y < 4; ++y) {
					if (CurrentY + y < height - 1) {
						bool line = true;
						for (int x = 1; x < width - 1; ++x) {
							if ((p[(CurrentY + y) * width + x]) == 0) {
								line = false;
							}
						}
						if (line) {
							for (int i = 1; i < width - 1; ++i) {
								p[(CurrentY + y)*width + i] = 8;
							}
							Lines.push_back(CurrentY + y);
						}
					}
				}
				score += 25;
				if (!Lines.empty()) {
					score += (1 << Lines.size()) * 100; //左移表示幂
				}
				Rotation = 0, CurrentX = width / 2 - 2, CurrentY = 0, Type = rand() % 7; //<cstdlib>
				GameOver = !fit(Type, Rotation, CurrentX, CurrentY);
			}
			counter = 0;
		}
		//绘制游戏界面
		for (int x = 0; x < width; ++x) {
			for (int y = 0; y < height; ++y) {
				screen[(y + 2) * Width + x + 2] = L" ABCDEFG=#"[p[y * width + x]];
			}
		}
		//绘制当前方块
		for (int x = 0; x < 4; ++x) {
			for (int y = 0; y < 4; ++y) {
				if (a[Type][rotate(x, y, Rotation)] == L'X') {
					screen[(CurrentY + y + 2)*Width + CurrentX + x + 2] = Type + 65;
				}
			}
		}
		swprintf_s(&screen[2 * Width + width + 6], 16, L"Score: %d", score);
		if (!Lines.empty()) {
			WriteConsoleOutputCharacterW(console, screen, Width *Height, {0, 0}, &count);
			this_thread::sleep_for(400ms);
			for (auto it = Lines.begin(); it != Lines.end(); ++it) {
				auto &v = *it;
				for (int x = 1; x < width - 1; ++x) {
					for (int y = v; y > 0; --y)
						p[y * width + x] = p[(y - 1) * width + x];
				}
				for (int i = 1; i < width - 1; ++i) {
					p[i] = 0;
				}
			}
			Lines.clear();
		}
	}
	delete []p;
	delete []screen;
	CloseHandle(console);
	cout << "   游戏结束!得分:" << score << "!\n" << endl;
	system("pause");
	return 0;
}

//碰撞检测
bool fit(int type, int rotation, int X, int Y) {

	for (int x = 0; x < 4; ++x) {
		for (int y = 0; y < 4; ++y) {
			int pi = rotate(x, y, rotation);//旋转后的形状坐标
			int fi = (Y + y) * width + (x + X);//游戏内位置坐标
			//越界检测
			if (x + X >= 0 && x + X < width) {
				if (y + Y > +0 && y + Y < height) {
					if (a[type][pi] == L'X' && p[fi] != 0)
						return false;
				}
			}
		}
	}
	return true;
}

int rotate(int x, int y, int r) {
	int i = 0;

	switch (r % 4) {
		case 1:
			i = 12 + y - 4 * x;
			break;
		case 2:
			i = 15 - 4 * y - x;
			break;
		case 3:
			i = 3 - y + 4 * x;
			break;
		case 0:
			i = x + 4 * y;
			break;
	}
	return i;
}
