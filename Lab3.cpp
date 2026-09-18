#include <iostream>
#include <omp.h>
#include "Lab3.h"

using namespace std;

int threadValue = 0;
#pragma omp threadprivate(threadValue)

void runLab3()
{
    cout << "\nЛабораторная работа #3\n";

    int sharedValue = 10;
    int firstValue = 20;
    int lastValue = -1;

    cout << "\nshared, private и firstprivate:\n";
#pragma omp parallel num_threads(4) shared(sharedValue) firstprivate(firstValue)
    {
        int privateValue = omp_get_thread_num();
        firstValue += privateValue;

#pragma omp critical(outputLab3)
        cout << "Нить " << privateValue << ": shared=" << sharedValue
             << ", firstprivate=" << firstValue << '\n';
    }

#pragma omp parallel for num_threads(4) lastprivate(lastValue)
    for (int i = 0; i < 12; i++)
    {
        lastValue = i * i;
    }

    cout << "lastprivate после последней логической итерации: " << lastValue << '\n';

    threadValue = 100;
    cout << "\nthreadprivate и copyin:\n";
#pragma omp parallel num_threads(4) copyin(threadValue)
    {
        threadValue += omp_get_thread_num();

#pragma omp critical(outputLab3)
        cout << "Нить " << omp_get_thread_num()
             << ": threadprivate=" << threadValue << '\n';
    }
}
