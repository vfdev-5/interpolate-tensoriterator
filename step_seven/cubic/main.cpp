// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

// Local
#include "bench_helper.h"
#include "interpolate.h"

// #define INSPECT_ASSEMBLY_CODE

using namespace at;
using namespace at::native;


int main(int argc, char** argv)
{

#ifdef INSPECT_ASSEMBLY_CODE
    
    auto input = at::rand({1, 3, 16, 320, 320});
    int64_t osizes[3] = {8, 256, 256};
    c10::optional<IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

    auto out = ti_upsample_tricubic3d_kernel_impl(input, output_size, false, scale_factors);    

    return 0;

#endif

    int n = 7500;
    bool full_bench = false;
    int num_threads = 6;

    manual_seed(10);

    if (argc >= 2)
    {        
        n = std::atoi(argv[1]);
    }
    if (argc >= 3) {
        full_bench = (bool) (std::atoi(argv[2]));
    }
    if (argc >= 4) {
        num_threads =  std::atoi(argv[4]);
    }

    std::cout << "Torch config: " << show_config() << std::endl;

    set_num_threads(num_threads);
    std::cout << "Num threads: " << get_num_threads() << std::endl;

#ifdef WITH_ASAN

    std::cout << "- Assert consistency 2D" << std::endl;
    assert_consistency_2d(320, 256, 512);
    assert_consistency_2d(500, 256, 800);
    std::cout << "- Assert consistency 1D" << std::endl;
    assert_consistency_1d();
    std::cout << "- Assert consistency 3D" << std::endl;
    assert_consistency_3d();

#endif

    std::cout << "\n\n---- Benchmark 2D ----" << std::endl;
    bench_2d(n, full_bench, 320, 256, 512);
    bench_2d(n, false, 500, 256, 800);
    std::cout << "\n---- END Benchmark 2D ----" << std::endl;

    return 0;
}
