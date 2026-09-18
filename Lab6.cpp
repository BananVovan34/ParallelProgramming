#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <omp.h>
#include "Lab6.h"

using namespace std;

namespace
{
    class LockBarrier
    {
    private:
        omp_lock_t lock;
        int threads;
        int arrived;
        int generation;

    public:
        explicit LockBarrier(int threadsCount)
        {
            threads = threadsCount;
            arrived = 0;
            generation = 0;
            omp_init_lock(&lock);
        }

        ~LockBarrier()
        {
            omp_destroy_lock(&lock);
        }

        void wait()
        {
            int currentGeneration;
            bool lastThread = false;

            omp_set_lock(&lock);
            currentGeneration = generation;
            arrived++;

            if (arrived == threads)
            {
                arrived = 0;
                generation++;
                lastThread = true;
            }

            omp_unset_lock(&lock);

            if (!lastThread)
            {
                while (true)
                {
                    int visibleGeneration;

                    omp_set_lock(&lock);
                    visibleGeneration = generation;
                    omp_unset_lock(&lock);

                    if (visibleGeneration != currentGeneration)
                    {
                        break;
                    }
                }
            }
        }
    };

    void createSystem(vector<vector<double>>& matrix, vector<double>& right,
        vector<double>& expected, int size)
    {
        mt19937 generator(42);
        uniform_real_distribution<double> distribution(-1.0, 1.0);

        matrix.assign(size, vector<double>(size));
        right.assign(size, 0.0);
        expected.resize(size);

        for (int i = 0; i < size; i++)
        {
            expected[i] = 1.0 + i % 5;
            double rowSum = 0.0;

            for (int j = 0; j < size; j++)
            {
                if (i != j)
                {
                    matrix[i][j] = distribution(generator);
                    rowSum += abs(matrix[i][j]);
                }
            }

            matrix[i][i] = rowSum + 1.0;
        }

        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                right[i] += matrix[i][j] * expected[j];
            }
        }
    }

    bool preparePivot(vector<vector<double>>& matrix, vector<double>& right, int column)
    {
        int pivot = column;

        for (int row = column + 1; row < static_cast<int>(matrix.size()); row++)
        {
            if (abs(matrix[row][column]) > abs(matrix[pivot][column]))
            {
                pivot = row;
            }
        }

        if (abs(matrix[pivot][column]) < 1e-12)
        {
            return false;
        }

        if (pivot != column)
        {
            swap(matrix[pivot], matrix[column]);
            swap(right[pivot], right[column]);
        }

        return true;
    }

    vector<double> backSubstitution(const vector<vector<double>>& matrix,
        const vector<double>& right)
    {
        int size = static_cast<int>(matrix.size());
        vector<double> answer(size);

        for (int i = size - 1; i >= 0; i--)
        {
            double sum = right[i];

            for (int j = i + 1; j < size; j++)
            {
                sum -= matrix[i][j] * answer[j];
            }

            answer[i] = sum / matrix[i][i];
        }

        return answer;
    }

    vector<double> gaussianSequential(vector<vector<double>> matrix, vector<double> right)
    {
        int size = static_cast<int>(matrix.size());

        for (int column = 0; column < size; column++)
        {
            if (!preparePivot(matrix, right, column))
            {
                return {};
            }

            for (int row = column + 1; row < size; row++)
            {
                double factor = matrix[row][column] / matrix[column][column];
                matrix[row][column] = 0.0;

                for (int j = column + 1; j < size; j++)
                {
                    matrix[row][j] -= factor * matrix[column][j];
                }

                right[row] -= factor * right[column];
            }
        }

        return backSubstitution(matrix, right);
    }

    vector<double> gaussianParallel(vector<vector<double>> matrix, vector<double> right)
    {
        int size = static_cast<int>(matrix.size());
        bool correct = true;

#pragma omp parallel shared(matrix, right, correct, size)
        {
            for (int column = 0; column < size; column++)
            {
#pragma omp single
                if (correct)
                {
                    correct = preparePivot(matrix, right, column);
                }

                if (correct)
                {
#pragma omp for schedule(static)
                    for (int row = column + 1; row < size; row++)
                    {
                        double factor = matrix[row][column] / matrix[column][column];
                        matrix[row][column] = 0.0;

                        for (int j = column + 1; j < size; j++)
                        {
                            matrix[row][j] -= factor * matrix[column][j];
                        }

                        right[row] -= factor * right[column];
                    }
                }
            }
        }

        if (!correct)
        {
            return {};
        }

        return backSubstitution(matrix, right);
    }

    double findError(const vector<double>& answer, const vector<double>& expected)
    {
        if (answer.size() != expected.size())
        {
            return -1.0;
        }

        double error = 0.0;

        for (size_t i = 0; i < answer.size(); i++)
        {
            error = max(error, abs(answer[i] - expected[i]));
        }

        return error;
    }

    void demonstrateLocks()
    {
        const int threads = 4;
        LockBarrier barrier(threads);
        omp_lock_t criticalLock;
        omp_init_lock(&criticalLock);
        int counter = 0;

#pragma omp parallel num_threads(threads) shared(barrier, criticalLock, counter)
        {
            int thread = omp_get_thread_num();

#pragma omp critical(outputLab6)
            cout << "Нить " << thread << " перед барьером\n";

            barrier.wait();

#pragma omp critical(outputLab6)
            cout << "Нить " << thread << " после барьера\n";

            omp_set_lock(&criticalLock);
            counter++;
            omp_unset_lock(&criticalLock);
        }

        omp_destroy_lock(&criticalLock);
        cout << "Счетчик после критической секции: " << counter << '\n';
    }

}

void runLab6()
{
    cout << "\nЛабораторная работа #6\n";
    cout << "\nБарьер и критическая секция на замках:\n";
    demonstrateLocks();

    int size;
    cout << "\nРазмер системы для метода Гаусса (0 - 400): ";
    cin >> size;

    if (!cin || size <= 0)
    {
        cin.clear();
        size = 400;
    }

    vector<vector<double>> matrix;
    vector<double> right;
    vector<double> expected;
    createSystem(matrix, right, expected, size);

    double startTime = omp_get_wtime();
    vector<double> sequentialAnswer = gaussianSequential(matrix, right);
    double sequentialTime = omp_get_wtime() - startTime;

    cout << fixed << setprecision(6);
    cout << "Последовательный алгоритм: " << sequentialTime << " сек.\n";
    cout << scientific;
    cout << "Ошибка последовательного решения: "
         << findError(sequentialAnswer, expected) << '\n';

    int previousThreads = omp_get_max_threads();
    vector<int> threadCounts = { 1, 2, 4, previousThreads };
    vector<int> testedThreads;

    cout << "\nАнализ параллельного алгоритма:\n";
    cout << "Нити\tВремя\t\tУскорение\tЭффективность\tОшибка\n";

    for (int threads : threadCounts)
    {
        threads = min(threads, omp_get_num_procs());

        if (find(testedThreads.begin(), testedThreads.end(), threads) != testedThreads.end())
        {
            continue;
        }

        testedThreads.push_back(threads);
        omp_set_num_threads(threads);
        startTime = omp_get_wtime();
        vector<double> parallelAnswer = gaussianParallel(matrix, right);
        double parallelTime = omp_get_wtime() - startTime;
        double speedup = sequentialTime / parallelTime;
        double efficiency = speedup / threads;

        cout << fixed << setprecision(6);
        cout << threads << '\t' << parallelTime << '\t'
             << speedup << "\t\t" << efficiency << "\t\t";
        cout << scientific << findError(parallelAnswer, expected) << '\n';
    }

    omp_set_num_threads(previousThreads);
    cout << defaultfloat;
    cout << "Использован один parallel-регион и schedule(static): строки на каждом "
         << "шаге имеют близкую вычислительную стоимость.\n";
}
