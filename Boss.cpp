#include <iostream>
#include <string>

int main(){
    //std::cout<<"[";
    for(int i=1; i<9; ++i) {
        //std::cout<<"("<<i<<",";
        std::string command = "OMP_NUM_THREADS="+std::to_string(i)+" ./a.out";
        std::system(command.c_str());//run
        //std::cout<<"),";
    }
    //std::cout<<"]\n";
}

//compile g++ Boss.cpp -o boss