#include <iostream>
#include <limits>
#include <string>
#include <omp.h>
#include "Lab2.h"

using namespace std;

namespace
{
    void showThreadCount()
    {
        cout << "\n1. Количество нитей\n";
        cout << "Доступно логических процессоров: " << omp_get_num_procs() << '\n';
        cout << "Настройка следующей команды: " << omp_get_max_threads() << '\n';
        cout << "Проверяемые размеры команды: от 1 до 128\n";

        for (int requested = 1; requested <= 128; requested *= 2)
        {
            int actual = 0;

#pragma omp parallel num_threads(requested) shared(actual)
            {
#pragma omp single
                actual = omp_get_num_threads();
            }

            cout << "Запрошено: " << requested << ", создано: " << actual << '\n';
        }
    }

    void showConditionalParallel()
    {
        cout << "\n2. Условное распараллеливание\n";
        const int sizes[] = { 10, 1000000 };

        for (int size : sizes)
        {
            long long sum = 0;
            int actual = 0;

#pragma omp parallel if(size >= 100000) num_threads(4) shared(actual) reduction(+:sum)
            {
#pragma omp single
                actual = omp_get_num_threads();

#pragma omp for
                for (int i = 1; i <= size; i++)
                {
                    sum += i;
                }
            }

            long long expected = static_cast<long long>(size) * (size + 1) / 2;
            cout << "Размер: " << size << ", нитей: " << actual
                 << ", сумма: " << sum
                 << ", проверка: " << (sum == expected ? "успешно" : "ошибка") << '\n';
        }
    }

    void showAllProcessors()
    {
        cout << "\n3. Одна нить на каждый доступный логический процессор\n";
        int processors = omp_get_num_procs();

#pragma omp parallel num_threads(processors)
        {
#pragma omp critical(outputLab2)
            cout << "Нить " << omp_get_thread_num() << " из "
                 << omp_get_num_threads() << '\n';
        }
    }

    void nestedRegion(int level, string parent, int& leafCount)
    {
#pragma omp parallel num_threads(2) shared(leafCount) firstprivate(level, parent)
        {
            int thread = omp_get_thread_num();
            string path = parent.empty() ? to_string(thread) : parent + "." + to_string(thread);

#pragma omp critical(outputLab2)
            cout << "Уровень " << level << ", путь " << path
                 << ", размер команды: " << omp_get_num_threads() << '\n';

            if (level < 3)
            {
                nestedRegion(level + 1, path, leafCount);
            }
            else
            {
#pragma omp atomic
                leafCount++;
            }
        }
    }

    void showNestedRegions()
    {
        cout << "\n4. Три уровня вложенных областей\n";
        int previousNested = omp_get_nested();

        for (int enabled = 0; enabled <= 1; enabled++)
        {
            omp_set_nested(enabled);
            int leafCount = 0;
            cout << "\nВложенность " << (enabled ? "разрешена" : "запрещена") << ":\n";
            nestedRegion(1, "", leafCount);
            cout << "Выполнений на третьем уровне: " << leafCount << '\n';
        }

        omp_set_nested(previousNested);
    }

    void showSingleAndMaster()
    {
        cout << "\n5. single и master\n";
        int singleCount = 0;
        int masterCount = 0;

#pragma omp parallel num_threads(4) shared(singleCount, masterCount)
        {
#pragma omp single
            {
                singleCount++;
                cout << "single: нить " << omp_get_thread_num() << '\n';
            }

#pragma omp master
            {
                masterCount++;
                cout << "master: нить " << omp_get_thread_num() << '\n';
            }

#pragma omp barrier
#pragma omp critical(outputLab2)
            cout << "Нить " << omp_get_thread_num() << " завершила оба этапа\n";
        }

        cout << "Выполнений single: " << singleCount
             << ", выполнений master: " << masterCount << '\n';
    }

    void showMasterInSingle()
    {
        cout << "\n6. Выполнение single нитью-мастером\n";

#pragma omp parallel num_threads(1)
        {
#pragma omp single
            cout << "Команда из одной нити: single выполнила нить "
                 << omp_get_thread_num() << '\n';
        }

#pragma omp parallel num_threads(4)
        {
#pragma omp single
            cout << "Команда из нескольких нитей: single выполнила нить "
                 << omp_get_thread_num() << '\n';
        }
    }

    void showMasterOnly()
    {
        cout << "\n7. Выполнение master в команде\n";

#pragma omp parallel num_threads(4)
        {
            int value = 1;

#pragma omp master
            {
                value = 2;
            }

#pragma omp critical(outputLab2)
            cout << "Нить " << omp_get_thread_num() << ": локальное значение = " << value << '\n';
        }
    }
}

void runLab2()
{
    cout << "\nЛабораторная работа #2\n";
    int previousDynamic = omp_get_dynamic();
    omp_set_dynamic(0);

    int task = -1;

    do
    {
        cout << "\n1. Проверка числа нитей\n";
        cout << "2. Опция if\n";
        cout << "3. Нити на доступных процессорах\n";
        cout << "4. Вложенные области\n";
        cout << "5. single и master\n";
        cout << "6. Мастер в single\n";
        cout << "7. Локальные данные и master\n";
        cout << "8. Выполнить все задания\n";
        cout << "0. Назад\nВыбор: ";

        if (!(cin >> task))
        {
            if (cin.eof())
            {
                break;
            }

            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            task = -1;
            cout << "Некорректный ввод\n";
            continue;
        }

        switch (task)
        {
        case 1:
            showThreadCount();
            break;

        case 2:
            showConditionalParallel();
            break;

        case 3:
            showAllProcessors();
            break;

        case 4:
            showNestedRegions();
            break;

        case 5:
            showSingleAndMaster();
            break;

        case 6:
            showMasterInSingle();
            break;

        case 7:
            showMasterOnly();
            break;

        case 8:
            showThreadCount();
            showConditionalParallel();
            showAllProcessors();
            showNestedRegions();
            showSingleAndMaster();
            showMasterInSingle();
            showMasterOnly();
            break;

        case 0:
            break;

        default:
            cout << "Такого задания нет\n";
        }
    } while (task != 0);

    omp_set_dynamic(previousDynamic);
}
