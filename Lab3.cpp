#include <iostream>
#include <limits>
#include <omp.h>
#include "Lab3.h"

using namespace std;

namespace
{
    int threadValue = 0;
#pragma omp threadprivate(threadValue)

    void showPrivate()
    {
        cout << "\n1. private (пример 12)\n";
        int value = 1;
        cout << "До области: value = " << value << '\n';

#pragma omp parallel num_threads(4) private(value)
        {
            value = omp_get_thread_num();

#pragma omp critical(outputLab3)
            cout << "Нить " << omp_get_thread_num() << ": private = " << value << '\n';
        }

        cout << "После области: value = " << value
             << ", проверка: " << (value == 1 ? "успешно" : "ошибка") << '\n';
    }

    void showShared()
    {
        cout << "\n2. shared (пример 13)\n";
        int values[10] = {};
        int actual = 0;
        cout << "Массив до области: ";

        for (int value : values)
        {
            cout << value << ' ';
        }
        cout << '\n';

#pragma omp parallel num_threads(4) shared(values, actual)
        {
            int thread = omp_get_thread_num();
            values[thread] = 1;

#pragma omp single
            actual = omp_get_num_threads();
        }

        bool correct = true;
        cout << "Массив после области: ";

        for (int i = 0; i < 10; i++)
        {
            cout << values[i] << ' ';

            if (values[i] != (i < actual ? 1 : 0))
            {
                correct = false;
            }
        }

        cout << "\nНитей: " << actual
             << ", проверка: " << (correct ? "успешно" : "ошибка") << '\n';
    }

    void showFirstprivate()
    {
        cout << "\n3. firstprivate (пример 14)\n";
        int value = 1;
        int errors = 0;
        cout << "До области: value = " << value << '\n';

#pragma omp parallel num_threads(4) firstprivate(value) reduction(+:errors)
        {
            int before = value;

            if (before != 1)
            {
                errors++;
            }

            value = omp_get_thread_num();

#pragma omp critical(outputLab3)
            cout << "Нить " << omp_get_thread_num() << ": до = " << before
                 << ", после = " << value << '\n';
        }

        cout << "После области: value = " << value
             << ", проверка: " << (value == 1 && errors == 0 ? "успешно" : "ошибка") << '\n';
    }

    void showThreadprivate()
    {
        cout << "\n4. threadprivate (пример 15)\n";

#pragma omp parallel num_threads(4)
        {
            threadValue = 0;
        }

        threadValue = 1;
        int firstTeam = 0;
        int secondTeam = 0;
        int errors = 0;

#pragma omp parallel num_threads(4) shared(firstTeam) reduction(+:errors)
        {
            int thread = omp_get_thread_num();
            int before = threadValue;

            if (before != (thread == 0 ? 1 : 0))
            {
                errors++;
            }

            threadValue = thread;

#pragma omp single
            firstTeam = omp_get_num_threads();

#pragma omp critical(outputLab3)
            cout << "Первая область, нить " << thread << ": до = " << before
                 << ", после = " << threadValue << '\n';
        }

        cout << "Между областями: threadValue = " << threadValue << '\n';

        if (threadValue != 0)
        {
            errors++;
        }

#pragma omp parallel num_threads(4) shared(secondTeam) reduction(+:errors)
        {
            int thread = omp_get_thread_num();

            if (threadValue != thread)
            {
                errors++;
            }

#pragma omp single
            secondTeam = omp_get_num_threads();

#pragma omp critical(outputLab3)
            cout << "Вторая область, нить " << thread
                 << ": сохранено = " << threadValue << '\n';
        }

        cout << "Размеры команд: " << firstTeam << " и " << secondTeam
             << ", проверка: " << (firstTeam == secondTeam && errors == 0 ? "успешно" : "ошибка") << '\n';
    }

    void showCopyin()
    {
        cout << "\n5. copyin (пример 16)\n";
        threadValue = 1;
        int errors = 0;
        cout << "До области: threadValue = " << threadValue << '\n';

#pragma omp parallel num_threads(4) copyin(threadValue) reduction(+:errors)
        {
            int thread = omp_get_thread_num();
            int before = threadValue;

            if (before != 1)
            {
                errors++;
            }

            threadValue += thread;

#pragma omp critical(outputLab3)
            cout << "Нить " << thread << ": до = " << before
                 << ", после = " << threadValue << '\n';
        }

        cout << "После области: threadValue = " << threadValue
             << ", проверка: " << (threadValue == 1 && errors == 0 ? "успешно" : "ошибка") << '\n';
    }

    void showLastprivate()
    {
        cout << "\n6. lastprivate\n";
        int value = -1;
        cout << "До цикла: value = " << value << '\n';

#pragma omp parallel for num_threads(4) schedule(static, 1) lastprivate(value)
        for (int i = 0; i < 12; i++)
        {
            value = i * i;

#pragma omp critical(outputLab3)
            cout << "Нить " << omp_get_thread_num() << ": i = " << i
                 << ", value = " << value << '\n';
        }

        cout << "После цикла: value = " << value
             << ", ожидается 121, проверка: " << (value == 121 ? "успешно" : "ошибка") << '\n';
    }

    void showReduction()
    {
        cout << "\n7. reduction\n";
        const int size = 1000000;
        long long sum = 0;

#pragma omp parallel for num_threads(4) reduction(+:sum)
        for (int i = 1; i <= size; i++)
        {
            sum += i;
        }

        long long expected = static_cast<long long>(size) * (size + 1) / 2;
        cout << "Сумма от 1 до " << size << ": " << sum
             << ", ожидается " << expected
             << ", проверка: " << (sum == expected ? "успешно" : "ошибка") << '\n';
    }

    void showCopyprivate()
    {
        cout << "\n8. copyprivate\n";
        int errors = 0;

#pragma omp parallel num_threads(4) reduction(+:errors)
        {
            int value = -1;
            int source = -1;

#pragma omp single copyprivate(value, source)
            {
                value = 42;
                source = omp_get_thread_num();
            }

            if (value != 42 || source < 0 || source >= omp_get_num_threads())
            {
                errors++;
            }

#pragma omp critical(outputLab3)
            cout << "Нить " << omp_get_thread_num() << ": получено = " << value
                 << ", источник = " << source << '\n';
        }

        cout << "Проверка copyprivate: " << (errors == 0 ? "успешно" : "ошибка") << '\n';
    }
}

void runLab3()
{
    cout << "\nЛабораторная работа #3\n";
    int previousDynamic = omp_get_dynamic();
    omp_set_dynamic(0);
    int task = -1;

    do
    {
        cout << "\n1. private\n";
        cout << "2. shared\n";
        cout << "3. firstprivate\n";
        cout << "4. threadprivate\n";
        cout << "5. copyin\n";
        cout << "6. lastprivate\n";
        cout << "7. reduction\n";
        cout << "8. copyprivate\n";
        cout << "9. Выполнить все примеры\n";
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
            showPrivate();
            break;

        case 2:
            showShared();
            break;

        case 3:
            showFirstprivate();
            break;

        case 4:
            showThreadprivate();
            break;

        case 5:
            showCopyin();
            break;

        case 6:
            showLastprivate();
            break;

        case 7:
            showReduction();
            break;

        case 8:
            showCopyprivate();
            break;

        case 9:
            showPrivate();
            showShared();
            showFirstprivate();
            showThreadprivate();
            showCopyin();
            showLastprivate();
            showReduction();
            showCopyprivate();
            break;

        case 0:
            break;

        default:
            cout << "Такого примера нет\n";
        }
    } while (task != 0);

    omp_set_dynamic(previousDynamic);
}
