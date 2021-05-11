#include <cstdio>
#include <cmath>
#include <omp.h>


#define PORTO_START 0 //integrate from, double
#define PORTO_FINISH 2 //integrate to, double
#define PORTO_COUNT 1000000000l //count of dots

//PORTO is name of my project, nothing more


double integrated_fun(double x){
    return sqrt(4-x*x);
}

int main(int argc, char **argv)
{
    double time_start, time_end;
    time_start = omp_get_wtime();//time measure section

    double res = 0;
    long long count = PORTO_COUNT;
    double start = PORTO_START, finish = PORTO_FINISH;
    double diff = (finish - start) / count;

#pragma omp parallel reduction(+: res)
    {
        int commSize = omp_get_num_threads();
        int rank = omp_get_thread_num();
        //printf("%d\n", omp_get_num_threads());
        for (long long i = rank; i < count; i+=commSize){
            res += integrated_fun(start + diff*(0.5 + i))*diff;
        }
    }

    printf("%f\n", res);
    time_end = omp_get_wtime();
    printf("%f\n", time_end - time_start);

    return 0;
}