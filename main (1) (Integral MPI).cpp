#include <iostream>
#include <mpi.h>
#include <cmath>
#include <chrono>


#define PORTO_START 0 //integrate from, double
#define PORTO_FINISH 2 //integrate to, double
#define PORTO_COUNT 10000000000l //count of dots

//PORTO is name of my project, nothing more

class Timer {
public:
    Timer() {
        begin = std::chrono::steady_clock::now();
    }

    ~Timer() {
        auto end = std::chrono::steady_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
        std::cout << "Time is " << elapsed_ms.count() << " ns\n";
    }

private:
    std::chrono::time_point<std::chrono::_V2::steady_clock, std::chrono::duration<long int, std::ratio<1, 1000000000>>>
    begin;

};

class MPI_unit
{
    const int Tag = 0;
    const int root = 0;
    int rank = 0, commSize = 0;
    double fun(double start, double finish, long long count);
public:
    //int getrank() const {return rank;}
    //int getcommSize() const {return  commSize;}
    MPI_unit(int argc, char *argv[]){
        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &commSize);
    }
    ~MPI_unit() {MPI_Finalize();}
    void run();
};

double integrated_fun(double x){
    return sqrt(4-x*x);
}

double MPI_unit::fun(double start, double finish, long long count)
{
    double diff = (finish - start) / count;
    double res = 0;
    for (long long i = 0; i < count; ++i){
        res += integrated_fun(start + diff*(0.5 + i))*diff;
    }
    return res;
}

void MPI_unit::run()
{
    //fprintf(stderr, "run working, rank = %d\n", rank);

    if (rank == root)
    {
        Timer timer;
        //MPI root
        //fprintf(stderr, "I'm root, commSize is %d\n", commSize);//print config information
        MPI_Status status;
        double result = 0;
        long long count = PORTO_COUNT;
        double start = PORTO_START, finish = PORTO_FINISH;

        long long partSize = count/commSize;//count_of_dots to each task
        long long shift = count%commSize;//remaining dots for distribution
        long long *msg = new long long[2* commSize];//distribution massive//0 - number of start, 1 - count
        /*massive with start index on 2*i positions and
        count of lines on 2*i+1 positions. 0 is root,
        i = 1..commSize-1 are clients*/
        for (int i = root; i < shift; ++i) {
            msg[2*i] = (partSize + 1) * i;
            msg[2*i + 1] = partSize + 1;
        } //clients with count_of_dots partSize+1 to distribute remainig dots
        for (int i = shift; i < commSize; ++i) {
            msg[2*i] = partSize * i + shift;
            msg[2*i + 1] = partSize;
        } //clients with count_of_dots partSize
        for (int i = root+1; i < commSize; ++i)
        {
            //fprintf(stderr, "I'm root, send to %d start %d count %d\n", i, msg[2*i], msg[2*i+1]);//printing configs
            MPI_Send(msg + 2*i, 2, MPI_LONG_LONG, i, Tag, MPI_COMM_WORLD);
            //sending tasks
        }
        //INTEGRATE
        result += fun((finish - start)*msg[0]/count + start,(finish - start)*(msg[0]+msg[1])/count + start,msg[1]);//the only using Trace Machine

        //fprintf(stderr, "I'm %d, local res is %f\n", rank, result);

        for (int i = root+1; i < commSize; ++i)
        {
            double res;
            MPI_Recv(&res, 1, MPI_DOUBLE, i, Tag, MPI_COMM_WORLD, &status);
            result += res;
            //receiving results
        }

        delete[] msg;
        printf("#####\nCommSize is %d\nresult is %f \nDotcount is %ld\n", commSize, result, PORTO_COUNT);
    }
    else
    {
        //MPI client
        MPI_Status status;
        long long msg[2];//two ints to receive task
        //usleep(1000 + 100*rank);
        MPI_Recv(msg, 2, MPI_LONG_LONG, root, Tag, MPI_COMM_WORLD, &status);
        //usleep(1000 + 100*rank);
        //fprintf(stderr, "I'm %d, start is %d, count is %d\n", rank, msg[0], msg[1]);

        long long count = PORTO_COUNT;
        double start = PORTO_START, finish = PORTO_FINISH;
        //INTEGRATE
        double res = fun((finish - start)*msg[0]/count + start,(finish - start)*(msg[0]+msg[1])/count + start,msg[1]);//the only using Trace Machine

        //fprintf(stderr, "I'm %d, local res is %f\n", rank, res);
        MPI_Send(&res, 1, MPI_DOUBLE, root, Tag, MPI_COMM_WORLD);//sending results
    }

}

int main(int argc, char *argv[])
{
    MPI_unit boss(argc, argv);
    boss.run();
}

//compile:  mpic++ main.cpp
//run:      mpirun -np 8 ./a.out
//to know time run: time mpirun -np 8 ./a.out

