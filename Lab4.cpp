#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <limits>
#include <omp.h>
#include "Lab4.h"

using namespace std;

namespace
{
    long long scalarProductSequential(const vector<int>& first, const vector<int>& second)
    {
        long long result = 0;

        for (size_t i = 0; i < first.size(); i++)
        {
            result += static_cast<long long>(first[i]) * second[i];
        }

        return result;
    }

    long long scalarProductParallel(const vector<int>& first, const vector<int>& second)
    {
        long long result = 0;
        int size = static_cast<int>(first.size());

#pragma omp parallel for reduction(+:result) schedule(static)
        for (int i = 0; i < size; i++)
        {
            result += static_cast<long long>(first[i]) * second[i];
        }

        return result;
    }

    int findMaxParallel(const vector<int>& values)
    {
        int size = static_cast<int>(values.size());
        vector<int> localMaximum(omp_get_max_threads(), numeric_limits<int>::min());

#pragma omp parallel
        {
            int thread = omp_get_thread_num();
            int result = numeric_limits<int>::min();

#pragma omp for schedule(static)
            for (int i = 0; i < size; i++)
            {
                if (values[i] > result)
                {
                    result = values[i];
                }
            }

            localMaximum[thread] = result;
        }

        return *max_element(localMaximum.begin(), localMaximum.end());
    }

    void showDistribution(int scheduleType, const char* name)
    {
        cout << name << ": ";

        if (scheduleType == 0)
        {
#pragma omp parallel for ordered schedule(static, 2) num_threads(4)
            for (int i = 0; i < 12; i++)
            {
                int thread = omp_get_thread_num();

#pragma omp ordered
                cout << i << "->" << thread << ' ';
            }
        }
        else if (scheduleType == 1)
        {
#pragma omp parallel for ordered schedule(dynamic, 2) num_threads(4)
            for (int i = 0; i < 12; i++)
            {
                int thread = omp_get_thread_num();

#pragma omp ordered
                cout << i << "->" << thread << ' ';
            }
        }
        else
        {
#pragma omp parallel for ordered schedule(guided, 2) num_threads(4)
            for (int i = 0; i < 12; i++)
            {
                int thread = omp_get_thread_num();

#pragma omp ordered
                cout << i << "->" << thread << ' ';
            }
        }

        cout << '\n';
    }

}

void runLab4()
{
    cout << "\nЛабораторная работа #4\n";

    int size;
    cout << "Размер векторов (0 - 10000000): ";
    cin >> size;

    if (!cin || size <= 0)
    {
        cin.clear();
        size = 10000000;
    }

    vector<int> first(size);
    vector<int> second(size);
    mt19937 generator(42);
    uniform_int_distribution<int> distribution(-1000, 1000);

    for (int i = 0; i < size; i++)
    {
        first[i] = distribution(generator);
        second[i] = distribution(generator);
    }

    double startTime = omp_get_wtime();
    long long sequentialProduct = scalarProductSequential(first, second);
    double sequentialProductTime = omp_get_wtime() - startTime;

    startTime = omp_get_wtime();
    long long parallelProduct = scalarProductParallel(first, second);
    double parallelProductTime = omp_get_wtime() - startTime;

    startTime = omp_get_wtime();
    int sequentialMax = *max_element(first.begin(), first.end());
    double sequentialMaxTime = omp_get_wtime() - startTime;

    startTime = omp_get_wtime();
    int parallelMax = findMaxParallel(first);
    double parallelMaxTime = omp_get_wtime() - startTime;

    cout << fixed << setprecision(6);
    cout << "\nСкалярное произведение: " << parallelProduct << '\n';
    cout << "Последовательно: " << sequentialProductTime << " сек.\n";
    cout << "Параллельно: " << parallelProductTime << " сек.\n";
    cout << "Ускорение: " << sequentialProductTime / parallelProductTime << '\n';
    cout << "Проверка: " << (sequentialProduct == parallelProduct ? "успешно" : "ошибка") << '\n';

    cout << "\nМаксимум: " << parallelMax << '\n';
    cout << "Последовательно: " << sequentialMaxTime << " сек.\n";
    cout << "Параллельно: " << parallelMaxTime << " сек.\n";
    cout << "Ускорение: " << sequentialMaxTime / parallelMaxTime << '\n';
    cout << "Проверка: " << (sequentialMax == parallelMax ? "успешно" : "ошибка") << '\n';
    cout << defaultfloat;

    cout << "\nРаспределение 12 итераций между четырьмя нитями:\n";
    showDistribution(0, "static ");
    showDistribution(1, "dynamic");
    showDistribution(2, "guided ");
}
