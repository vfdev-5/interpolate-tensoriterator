// STD
#include <iostream>
#include <chrono>

// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>
#include <ATen/native/UpSample.h>


#define NUM_THREADS 6

using namespace at;
using namespace at::native;
using namespace at::indexing;


int bench_3d(int n, bool full_bench, c10::ScalarType dtype) {

    auto t_input = at::rand({1, 3, 16, 320, 320}, at::CPU(dtype));
    std::cout << "\nInput tensor: " << t_input.sizes() << " " << t_input.dtype() << std::endl;
 
    set_num_threads(NUM_THREADS);
    std::cout << "Num threads: " << get_num_threads() << std::endl;

    // Time benchmark
    {
        int64_t osizes[3] = {8, 256, 256};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_trilinear3d_cpu (" << n << " rounds) - downsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = upsample_trilinear3d_cpu(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[3] = {8, 256, 256};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench my_internal_upsample::pth_ti_upsample_trilinear3d_kernel_impl (" << n << " rounds) - downsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = my_internal_upsample::pth_ti_upsample_trilinear3d_kernel_impl(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 0;

}


int main(int argc, char** argv)
{

    auto n = 7500;
    bool full_bench = false;
    bool test_all_dims = false;

    manual_seed(10);

    if (argc >= 2)
    {        
        n = std::atoi(argv[1]);
    }
    if (argc >= 3) {
        full_bench = (bool) (std::atoi(argv[2]));
    }
    if (argc == 4) {
        test_all_dims = (bool) (std::atoi(argv[3]));
    }

    std::cout << "Torch config: " << show_config() << std::endl;

    // at::IntArrayRef b = {1, 2, 12, 34};
    // at::IntArrayRef c = {10, 12};
    // at::native::upsample::compute_output_size(b, c, c10::nullopt);

    std::cout << "\n\n---- Benchmark 3D ----" << std::endl;
    bench_3d(n / 10, full_bench, at::kFloat);
    bench_3d(n / 10, full_bench, at::kDouble);

    return 0;
}
