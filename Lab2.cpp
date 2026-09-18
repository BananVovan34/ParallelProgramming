#include <iostream>
#include <omp.h>
#include "Lab2.h"

using namespace std;

namespace
{
    void nestedRegion(int level)
    {
        int thread = omp_get_thread_num();
        int count = omp_get_num_threads();

#pragma omp critical(outputLab2)
        cout << "Уровень " << level << ": нить " << thread
             << " из " << count << '\n';

        if (level < 3)
        {
#pragma omp parallel num_threads(2)
            nestedRegion(level + 1);
        }
    }

}

void runLab2()
{
    cout << "\nЛабораторная работа #2\n";
    cout << "Процессоров: " << omp_get_num_procs() << '\n';
    cout << "Максимум нитей: " << omp_get_max_threads() << '\n';

    cout << "\nОбщая параллельная область:\n";
#pragma omp parallel num_threads(omp_get_num_procs())
    {
#pragma omp critical(outputLab2)
        cout << "Нить " << omp_get_thread_num() << " из "
             << omp_get_num_threads() << '\n';
    }

    cout << "\nВложенность запрещена:\n";
    omp_set_nested(0);
#pragma omp parallel num_threads(2)
    nestedRegion(1);

    cout << "\nВложенность разрешена:\n";
    omp_set_nested(1);
#pragma omp parallel num_threads(2)
    nestedRegion(1);
    omp_set_nested(0);
}
