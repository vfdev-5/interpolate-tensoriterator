// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

// Local
#include "bench_helper.h"


int main(int argc, char** argv)
{

    int n = 7500;
    bool full_bench = false;
    bool test_all_dims = false;
    int num_threads = 1;

    manual_seed(10);

    if (argc >= 2)
    {
        n = std::atoi(argv[1]);
    }
    if (argc >= 3) {
        full_bench = (bool) (std::atoi(argv[2]));
    }
    if (argc >= 4) {
        test_all_dims = (bool) (std::atoi(argv[3]));
    }
    if (argc >= 5) {
        num_threads =  std::atoi(argv[4]);
    }

    std::cout << "Torch config: " << show_config() << std::endl;

    set_num_threads(num_threads);
    std::cout << "Num threads: " << get_num_threads() << std::endl;

    std::cout << "\n\n---- Benchmark 2D ----" << std::endl;
    bench_2d(n, full_bench, 320, 256, 512);
    if (full_bench) {
        bench_2d(n, false, 500, 256, 800);
    }
    std::cout << "\n---- END Benchmark 2D ----" << std::endl;

    if (!test_all_dims) return 0;

    std::cout << "\n\n---- Benchmark 1D ----" << std::endl;
    bench_1d(n, full_bench);
    std::cout << "\n---- END Benchmark 1D ----" << std::endl;

    std::cout << "\n\n---- Benchmark 3D ----" << std::endl;
    bench_3d(n / 10, full_bench);
    std::cout << "\n---- END Benchmark 3D ----" << std::endl;

    return 0;
}
