#include <iostream>
#include <iomanip>
#include <omp.h>
#include "Lab1.h"

using namespace std;

namespace
{
    const char* getOpenMPVersion()
    {
#if _OPENMP >= 202111
        return "5.2";
#elif _OPENMP >= 202011
        return "5.1";
#elif _OPENMP >= 201811
        return "5.0";
#elif _OPENMP >= 201511
        return "4.5";
#elif _OPENMP >= 201307
        return "4.0";
#elif _OPENMP >= 201107
        return "3.1";
#elif _OPENMP >= 200805
        return "3.0";
#elif _OPENMP >= 200505
        return "2.5";
#elif _OPENMP >= 200203
        return "2.0";
#else
        return "1.x";
#endif
    }

}

void runLab1()
{
    cout << "\nЛабораторная работа #1\n";

#ifdef _OPENMP
    cout << "OpenMP включен\n";
    cout << "_OPENMP = " << _OPENMP << " (стандарт " << getOpenMPVersion() << ")\n";
#else
    cout << "OpenMP не включен в настройках компилятора\n";
    return;
#endif

    cout << "Доступно процессоров: " << omp_get_num_procs() << '\n';
    cout << "Максимум нитей: " << omp_get_max_threads() << '\n';

    int realThreads = 0;

#pragma omp parallel
    {
#pragma omp single
        realThreads = omp_get_num_threads();
    }

    cout << "Фактически создано нитей: " << realThreads << '\n';

    const int repeats = 1000000;
    volatile double timerValue = 0.0;
    double startTime = omp_get_wtime();

    for (int i = 0; i < repeats; i++)
    {
        timerValue = omp_get_wtime();
    }

    double endTime = omp_get_wtime();
    double callTime = (endTime - startTime) / repeats;

    cout << scientific << setprecision(6);
    cout << "Точность omp_get_wtime(): " << omp_get_wtick() << " сек.\n";
    cout << "Среднее время вызова: " << callTime << " сек.\n";
    cout << "Последнее значение таймера: " << timerValue << '\n';
    cout << defaultfloat;
}
