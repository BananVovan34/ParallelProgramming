#include <iostream>
#include <algorithm>
#include <limits>
#include <string>
#include <omp.h>
#define NOMINMAX
#include <Windows.h>
#include "Lab2.h"

using namespace std;

namespace
{
    struct ThreadProbe
    {
        int actual = 0;
        bool completed = false;
        bool inconclusive = false;
    };

    ThreadProbe probeThreadCount(int requested)
    {
        wchar_t path[32768];
        DWORD length = GetModuleFileNameW(nullptr, path, 32768);

        if (length == 0 || length >= 32768)
        {
            cout << "Не удалось определить путь программы\n";
            return { 0, false, true };
        }

        wstring command = L"\"" + wstring(path) + L"\" --lab2-thread-probe " + to_wstring(requested);
        STARTUPINFOW startup = {};
        startup.cb = sizeof(startup);
        PROCESS_INFORMATION process = {};

        if (!CreateProcessW(path, command.data(), nullptr, nullptr, FALSE,
            CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process))
        {
            cout << "Не удалось запустить пробу, код Windows: " << GetLastError() << '\n';
            return { 0, false, true };
        }

        CloseHandle(process.hThread);
        DWORD wait = WaitForSingleObject(process.hProcess, 10000);

        if (wait != WAIT_OBJECT_0)
        {
            TerminateProcess(process.hProcess, 0);
            WaitForSingleObject(process.hProcess, 1000);
            CloseHandle(process.hProcess);
            cout << "Запрошено: " << requested << ", проба не завершена: "
                 << (wait == WAIT_TIMEOUT ? "тайм-аут 10 секунд" : "ошибка ожидания") << '\n';
            return { 0, false, true };
        }

        DWORD code = 0;
        bool received = GetExitCodeProcess(process.hProcess, &code) != FALSE;
        CloseHandle(process.hProcess);

        if (!received)
        {
            cout << "Не удалось получить результат пробы\n";
            return { 0, false, true };
        }

        if (code > static_cast<DWORD>(lab2ProbeExitBase)
            && code <= static_cast<DWORD>(lab2ProbeExitBase + requested))
        {
            int actual = static_cast<int>(code) - lab2ProbeExitBase;
            cout << "Запрошено: " << requested << ", создано: " << actual << '\n';
            return { actual, actual == requested, false };
        }

        cout << "Запрошено: " << requested << ", отказ пробы, код процесса: " << code << '\n';
        return {};
    }

    void showThreadCount()
    {
        cout << "\n1. Количество нитей\n";
        cout << "Доступно логических процессоров: " << omp_get_num_procs() << '\n';
        cout << "Настройка следующей команды: " << omp_get_max_threads() << '\n';
        const int searchLimit = lab2ThreadSearchLimit;
        int lower = 0;
        int upper = 0;
        ULONGLONG start = GetTickCount64();
        cout << "Поиск границы, защитный предел: " << searchLimit << '\n';
        cout << "Каждая проба запускается отдельно; динамический размер отключён\n";

        for (int requested = 1; ; requested = min(searchLimit, requested * 2))
        {
            ThreadProbe probe = probeThreadCount(requested);

            if (probe.inconclusive || GetTickCount64() - start > 90000)
            {
                cout << "Поиск остановлен без определения максимума. Подтверждено: " << lower << '\n';
                return;
            }

            if (!probe.completed)
            {
                upper = requested;
                break;
            }

            lower = requested;

            if (requested == searchLimit)
            {
                break;
            }
        }

        if (lower == 0)
        {
            cout << "Не удалось подтвердить даже команду из одной нити\n";
            return;
        }

        if (upper == 0)
        {
            cout << "Создано не менее " << lower << " нитей. Защитный предел достигнут; максимум не найден\n";
            return;
        }

        while (upper - lower > 1)
        {
            int middle = lower + (upper - lower) / 2;
            ThreadProbe probe = probeThreadCount(middle);

            if (probe.inconclusive || GetTickCount64() - start > 90000)
            {
                cout << "Поиск остановлен без определения максимума. Подтверждено: " << lower << '\n';
                return;
            }

            if (probe.completed)
            {
                lower = middle;
            }
            else
            {
                upper = middle;
            }
        }

        cout << "\nПовторная проверка границы " << lower << " / " << upper << ":\n";

        for (int attempt = 0; attempt < 3; attempt++)
        {
            ThreadProbe success = probeThreadCount(lower);
            ThreadProbe failure = probeThreadCount(upper);

            if (success.inconclusive || failure.inconclusive || !success.completed
                || failure.completed || GetTickCount64() - start > 90000)
            {
                cout << "Граница не подтверждена повторно; устойчивый максимум не установлен\n";
                return;
            }
        }

        cout << "Экспериментальный максимум полного выполнения запроса: " << lower << '\n';
        cout << "Запрос " << upper << " не выполнен полностью в трёх повторных пробах\n";
        cout << "Результат относится к текущим ресурсам, runtime и настройкам среды\n";
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

int measureLab2Team(int requested)
{
    omp_set_dynamic(0);
    int actual = 0;

#pragma omp parallel num_threads(requested) shared(actual)
    {
#pragma omp single
        actual = omp_get_num_threads();
    }

    return actual;
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
