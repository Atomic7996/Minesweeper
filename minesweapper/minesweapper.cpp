﻿#include <iostream>
#include <ctime>
#include <windows.h>
#include <fstream>
#include <string>
#include <iomanip>

#define closedCell 0
#define freeCell 0
#define explodedMine 1
#define uncheckedCell 2
#define openedCell 3
#define flagMine 4
#define mine 9

using namespace std;

char** generateField(char** field, int fieldHeight, int fieldWidth, int mines)
{
	int usedSpace = 0;
	int mineSpace = 0;

	field = new char* [fieldHeight];
	//Создание поля с пустыми ячейками
	for (int i = 0; i < fieldHeight; i++)
	{
		field[i] = new char[fieldWidth];

		for (int j = 0; j < fieldWidth; j++)
			field[i][j] = freeCell;
	}
	//Размещение мин на поле
	for (int i = 0; i < fieldHeight; i++)
	{
		for (int j = 0; j < fieldWidth; j++)
		{
			if (mines <= 0)
				break;

			if (field[i][j] == mine)
			{
				mineSpace++;
				continue;
			}

			if (rand() % ((fieldHeight * fieldWidth) - (i * fieldWidth) - j - usedSpace + mineSpace) == 0)
			{
				field[i][j] = mine;
				mines--;
				usedSpace++;
				mineSpace = i = 0;
				j = -1;
			}
		}
	}
	//Размещение цифр
	for (int i = 0; i < fieldHeight; i++)
	{
		for (int j = 0; j < fieldWidth; j++)
		{
			if (field[i][j] > 8)
			{
				if (i - 1 >= 0 && field[i - 1][j] != mine)
					field[i - 1][j]++;

				if (i + 1 < fieldHeight && field[i + 1][j] != mine)
					field[i + 1][j]++;

				if (j - 1 >= 0 && field[i][j - 1] != mine)
					field[i][j - 1]++;

				if (j + 1 < fieldWidth && field[i][j + 1] != mine)
					field[i][j + 1]++;

				if (i - 1 >= 0 && j - 1 >= 0 && field[i - 1][j - 1] != mine)
					field[i - 1][j - 1]++;

				if (i - 1 >= 0 && j + 1 < fieldWidth && field[i - 1][j + 1] != mine)
					field[i - 1][j + 1]++;

				if (i + 1 < fieldHeight && j - 1 >= 0 && field[i + 1][j - 1] != mine)
					field[i + 1][j - 1]++;

				if (i + 1 < fieldHeight && j + 1 < fieldWidth && field[i + 1][j + 1] != mine)
					field[i + 1][j + 1]++;
			}
		}
	}

	cout << endl;
	cout << "Игровое поле успешно сгенерировано\n\n";
	Sleep(1000);

	return field;
}

int checkNearCell(char** field, char** playerView, int check, int i, int j)
{
	if (field[i][j] == freeCell && playerView[i][j] == closedCell)
	{
		playerView[i][j] = uncheckedCell;
		check++;
	}

	if (field[i][j] > 0 && field[i][j] < 9 && playerView[i][j] == closedCell)
	{
		playerView[i][j] = openedCell;
	}

	return check;
}

void openSpace(char** field, char** playerView, int fieldHeight, int fieldWidth, int x, int y)
{
	int check = 0;
	int xDiv2 = x / 2;
	//Берутся только нечетные координаты, так как после каждой ячейки идет пробел
	if (!(x % 2))
	{
		if (y < fieldHeight && xDiv2 < fieldWidth)
		{
			if (playerView[y][xDiv2] == closedCell && field[y][xDiv2] != 0)
				playerView[y][xDiv2] = openedCell;

			if (playerView[y][xDiv2] == closedCell && field[y][xDiv2] == freeCell)
			{
				playerView[y][xDiv2] = uncheckedCell;
				check++;

				for (int i = 0; i < fieldHeight; i++)
				{
					for (int j = 0; j < fieldWidth; j++)
					{
						if (playerView[i][j] == uncheckedCell)
						{
							playerView[i][j] = openedCell;
							check--;
							//Проверка ячеек вокруг выбранной
							if (i - 1 >= 0)
								check = checkNearCell(field, playerView, check, i - 1, j);

							if (i - 1 >= 0 && j - 1 >= 0)
								check = checkNearCell(field, playerView, check, i - 1, j - 1);

							if (j - 1 >= 0)
								check = checkNearCell(field, playerView, check, i, j - 1);

							if (i + 1 < fieldHeight && j - 1 >= 0)
								check = checkNearCell(field, playerView, check, i + 1, j - 1);

							if (i + 1 < fieldHeight)
								check = checkNearCell(field, playerView, check, i + 1, j);

							if (i + 1 < fieldHeight && j + 1 < fieldWidth)
								check = checkNearCell(field, playerView, check, i + 1, j + 1);

							if (j + 1 < fieldWidth)
								check = checkNearCell(field, playerView, check, i, j + 1);

							if (i - 1 >= 0 && j + 1 < fieldWidth)
								check = checkNearCell(field, playerView, check, i - 1, j + 1);

							if (check > 0)
								i = 0;

							j = -1;
						}
					}
				}
			}

			if (field[y][xDiv2] == mine && playerView[y][xDiv2] != flagMine)
				playerView[y][xDiv2] = explodedMine;
		}
	}
}

void gameplay(char** field, int fieldHeight, int fieldWidth, int mines, string selectedPreset)
{
	HANDLE hIn, hOut;
	INPUT_RECORD inRec;
	DWORD numRead;
	CONSOLE_CURSOR_INFO curs;
	CONSOLE_SCREEN_BUFFER_INFO buffer;
	COORD zero = { 0,0 };
	boolean gameOver = false;
	boolean win = false;
	int x;
	int y;
	int xDiv2;
	int flag = 0;
	int cleanSpace = 2;
	int input = 1;
	int markedMines = 0;
	int openedCells = 0;
	int minesAtBeginning = mines;
	char** playerView = new char* [fieldHeight];

	clock_t startTime = clock();
	//Скрытие игрового поля для игрока
	for (int i = 0; i < fieldHeight; i++)
	{
		playerView[i] = new char[fieldWidth];

		for (int j = 0; j < fieldWidth; j++)
			playerView[i][j] = closedCell;
	}
	//Определение длины кол-ва мин для вывода в консоль
	for (int i = mines; (i /= 10) > 0;)
		cleanSpace++;

	system("cls");
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleCursorInfo(hOut, &curs);
	SetConsoleMode(hIn, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);

	while (!gameOver)
	{
		curs.bVisible = false;
		SetConsoleCursorInfo(hOut, &curs);
		SetConsoleCursorPosition(hOut, zero);

		for (int i = 0; i < fieldHeight; i++)
		{
			for (int j = 0; j < fieldWidth; j++)
			{
				switch (playerView[i][j])
				{
				case closedCell: {
					SetConsoleTextAttribute(hOut, (WORD)(0 | 8));
					WriteConsoleW(hOut, L"■ ", wcslen(L"■ "), NULL, NULL);
					SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
					break;
				}
				case explodedMine: {
					SetConsoleTextAttribute(hOut, (WORD)(0 | 4));
					WriteConsoleW(hOut, L"☼ ", wcslen(L"☼ "), NULL, NULL);
					SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
					gameOver = true;
					break;
				}
				case openedCell: {
					openedCells++;

					switch (field[i][j])
					{
					case freeCell: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 8));
						WriteConsoleW(hOut, L"□ ", wcslen(L"□ "), NULL, NULL);
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 1: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 9));
						cout << "1 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 2: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 10));
						cout << "2 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 3: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 12));
						cout << "3 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 4: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 5));
						cout << "4 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 5: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 4));
						cout << "5 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 6: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 1));
						cout << "6 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 7: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 2));
						cout << "7 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 8: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 8));
						cout << "8 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case mine: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						WriteConsoleW(hOut, L"☼ ", wcslen(L"☼ "), NULL, NULL);
					}
					}
					break;
				}
				case flagMine: {
					SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
					WriteConsoleW(hOut, L"X ", wcslen(L"X "), NULL, NULL);
					SetConsoleTextAttribute(hOut, (WORD)(0 | 15));

					if (field[i][j] == mine)
						markedMines++;
					else
						markedMines--;
				}
				}
			}
			cout << endl;
		}

		if (markedMines == minesAtBeginning || openedCells >= (fieldHeight * fieldWidth - minesAtBeginning))
			win = gameOver = true;

		openedCells = 0;
		markedMines = 0;

		cout << "\nMines left ";

		for (int i = cleanSpace; i != 0; i--)
			cout << ' ';

		GetConsoleScreenBufferInfo(hOut, &buffer);
		buffer.dwCursorPosition.X -= cleanSpace;
		SetConsoleCursorPosition(hOut, buffer.dwCursorPosition);
		cout << mines;

		cout << "\nНажмите 0 для выхода" << endl;
		curs.bVisible = true;
		SetConsoleCursorInfo(hOut, &curs);
		ReadConsoleInput(hIn, &inRec, 1, &numRead);

		//Нажатие мыши
		switch (inRec.EventType)
		{
		case MOUSE_EVENT: {
			//Открытие ячейки
			if (inRec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				x = inRec.Event.MouseEvent.dwMousePosition.X;
				y = inRec.Event.MouseEvent.dwMousePosition.Y;
				openSpace(field, playerView, fieldHeight, fieldWidth, x, y);
			}
			//Установка флажка
			if (inRec.Event.MouseEvent.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
			{
				x = inRec.Event.MouseEvent.dwMousePosition.X;
				y = inRec.Event.MouseEvent.dwMousePosition.Y;
				xDiv2 = x / 2;

				if (y < fieldHeight && xDiv2 < fieldWidth && !(x % 2))
				{
					if (playerView[y][xDiv2] == closedCell)
						flag = 1;

					if (playerView[y][xDiv2] == flagMine)
						flag = 2;

					if (flag == 1)
					{
						playerView[y][xDiv2] = flagMine;
						mines--;
					}

					if (flag == 2)
					{
						playerView[y][xDiv2] = closedCell;
						mines++;
					}

					flag = 0;
				}
			}

			break;
		}
		case KEY_EVENT: {
			if (inRec.Event.KeyEvent.uChar.AsciiChar == '0')
				gameOver = true;
		}
		}
	}
	//Конец игры
	if (gameOver)
	{
		clock_t endTime = clock();

		system("cls");
		SetConsoleCursorPosition(hOut, zero);

		for (int i = 0; i < fieldHeight; i++)
		{
			playerView[i] = new char[fieldWidth];

			for (int j = 0; j < fieldWidth; j++)
				playerView[i][j] = openedCell;
		}
		//Вывод открытого поля
		for (int i = 0; i < fieldHeight; i++)
		{
			for (int j = 0; j < fieldWidth; j++)
			{
				switch (playerView[i][j])
				{
				case openedCell: {

					switch (field[i][j])
					{
					case freeCell: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 8));
						WriteConsoleW(hOut, L"□ ", wcslen(L"□ "), NULL, NULL);
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 1: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 9));
						cout << "1 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 2: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 10));
						cout << "2 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 3: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 12));
						cout << "3 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 4: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 5));
						cout << "4 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 5: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 4));
						cout << "5 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 6: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 1));
						cout << "6 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 7: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 2));
						cout << "7 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case 8: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 8));
						cout << "8 ";
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
						break;
					}
					case mine: {
						SetConsoleTextAttribute(hOut, (WORD)(0 | 4));
						WriteConsoleW(hOut, L"☼ ", wcslen(L"☼ "), NULL, NULL);
						SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
					}
					}
					break;
				}
				}
			}
			cout << endl;
		}

		if (!win)
		{
			SetConsoleTextAttribute(hOut, (WORD)(0 | 4));
			cout << "\nПОРАЖЕНИЕ\n";
		}

		if (win)
		{
			SetConsoleTextAttribute(hOut, (WORD)(0 | 10));
			cout << "\n!ПОБЕДА!\n";

			int time = (int)(endTime - startTime) / CLOCKS_PER_SEC;
			int minutes = 0;
			int seconds = 0;
			int option;

			minutes = time / 60;
			seconds = time % 60;

			SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
			cout << "\nВаше время - ";
			if (minutes < 10)
				cout << " 0" << minutes << " минут ";
			else
				cout << " " << minutes << " минут ";
			if (seconds < 10)
				cout << " 0" << seconds << " секунд\n";
			else
				cout << " " << seconds << " секунд\n";

			
		}

		SetConsoleTextAttribute(hOut, (WORD)(0 | 15));
		cout << "\nНажмите любую клавишу для выхода";

		do {
			ReadConsoleInput(hIn, &inRec, 1, &numRead);
		} while (inRec.EventType != KEY_EVENT);
	}

	system("cls");
}

int main()
{
	setlocale(LC_ALL, "rus");
	srand(time(NULL));

	int option;
	int toMainMenu;
	int fieldHeight, fieldWidth, mines, minesCheck;
	char** field = new char*;
	boolean fieldWasGenerated = false;
	string selectedPreset;

	do {
		system("cls");
		cout << "Добро пожаловать в игру САПЁР!\n";
		cout << "\nУправление в меню осуществляется с помощью ввода номеров команд,\nпредставленных на экране.\n\n";

		cout << "1. Начать игру\n";
		cout << "2. Правила игры\n";
		cout << "3. Список результатов\n";
		cout << "0. Выход из игры\n> ";
		cin >> option;

		switch (option)
		{
		case 1: {
			fieldHeight = fieldWidth = 8;
			mines = 12;
			field = generateField(field, fieldHeight, fieldWidth, mines);
			fieldWasGenerated = true;
			selectedPreset = "Easy_8x8_12";

			if (fieldWasGenerated = true)
				gameplay(field, fieldHeight, fieldWidth, mines, selectedPreset);

			continue;
		}
		
		default:
			continue;
		}

	} while (option != 0);

	return 0;
}