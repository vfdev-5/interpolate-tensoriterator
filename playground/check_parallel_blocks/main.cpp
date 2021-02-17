// STD
#include <iostream>
#include <chrono>
#include <thread>

// Torch
#include <ATen/ATen.h>

// Local
#include "ti_check_blocks.h"
// #include <omp.h>


using namespace std::chrono_literals;


void test_f(int64_t begin, int64_t end)
{
    std::this_thread::sleep_for(1ms * begin);
    std::cout << begin << " --> " << end << std::endl;
}


inline int64_t divup(int64_t x, int64_t y) {
  return (x + y - 1) / y;
}


int main(int argc, char** argv)
{

    at::parallel_for(0, 100, 10, test_f);

    // int64_t begin = 0;
    // int64_t end = 100;
    // int64_t grain_size = 10;

    // #pragma omp parallel if (omp_get_max_threads() > 1 && !omp_in_parallel() && ((end - begin) > grain_size))
    // {
    //     // choose number of tasks based on grain size and number of threads
    //     // can't use num_threads clause due to bugs in GOMP's thread pool (See #32008)
    //     int64_t num_threads = omp_get_num_threads();
    //     if (grain_size > 0) {
    //         num_threads = std::min(num_threads, divup((end - begin), grain_size));
    //     }

    //     int64_t tid = omp_get_thread_num();
    //     int64_t chunk_size = divup((end - begin), num_threads);
    //     int64_t begin_tid = begin + tid * chunk_size;
    //     if (begin_tid < end) {
    //         test_f(begin_tid, std::min(end, chunk_size + begin_tid));
    //     }
    // }


    // auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
    // std::cout << "Input tensor: "<< t_input.sizes() << std::endl;
 
    // {
    //     std::cout << "\n- Run : ";
    //     auto out = ti_check_blocks(t_input);
    //     std::cout << "OK" << std::endl;
    // }

    return 0;
}