#include <iostream>
#include <limits>
#define NOMINMAX
#include <Windows.h>
#include "Lab1.h"
#include "Lab2.h"
#include "Lab3.h"
#include "Lab4.h"
#include "Lab6.h"

using namespace std;

int main()
{
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    int lab = -1;

    do
    {
        cout << "\nВыберите лабораторную работу:\n";
        cout << "1. Основы OpenMP и измерение времени\n";
        cout << "2. Параллельные и последовательные области\n";
        cout << "3. Модель данных\n";
        cout << "4. Распределение работы\n";
        cout << "6. Синхронизация и метод Гаусса\n";
        cout << "0. Выход\n";
        cout << "Выбор: ";

        if (!(cin >> lab))
        {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Некорректный ввод\n";
            continue;
        }

        switch (lab)
        {
        case 1:
            runLab1();
            break;

        case 2:
            runLab2();
            break;

        case 3:
            runLab3();
            break;

        case 4:
            runLab4();
            break;

        case 6:
            runLab6();
            break;

        case 0:
            cout << "Завершение программы...\n";
            break;

        default:
            cout << "Такой лабораторной работы нет\n";
        }
    } while (lab != 0);

    return 0;
}
