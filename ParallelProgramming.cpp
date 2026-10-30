#include <iostream>
#include <limits>
#include <charconv>
#include <string>
#define NOMINMAX
#include <Windows.h>
#include "Lab1.h"
#include "Lab2.h"
#include "Lab3.h"
#include "Lab4.h"
#include "Lab6.h"

using namespace std;

int main(int argc, char* argv[])
{
    if (argc >= 2 && string(argv[1]) == "--lab2-thread-probe")
    {
        SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
        int requested = 0;

        if (argc != 3)
        {
            return 0;
        }

        string argument = argv[2];
        auto result = from_chars(argument.data(), argument.data() + argument.size(), requested);

        if (result.ec != errc() || result.ptr != argument.data() + argument.size()
            || requested < 1 || requested > lab2ThreadSearchLimit)
        {
            return 0;
        }

        // В служебном режиме код завершения передаёт фактический размер команды.
        return lab2ProbeExitBase + measureLab2Team(requested);
    }

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
            if (cin.eof())
            {
                break;
            }

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
