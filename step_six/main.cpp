// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

// Local
#include "bench_helper.h"
#include "interpolate.h"


// #define INSPECT_ASSEMBLY_CODE
// #define BENCH_3D_ONLY
// #define BENCH_2D_SLOWDOWN_CASE_ONLY
#define BENCH_2D_SLOWDOWN_CL_CASE_ONLY
// #define INSPECT_2D_SLOWDOWN_CASE_ONLY
// #define INSPECT_2D_SLOWDOWN_CASE2_ONLY
// #define INSPECT_3D_CASE_ONLY
// #define PERF_INSPECT_2D_SLOWDOWN_CASE2_ONLY
// How to execute it with valgrind:
// valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bench 0 0 0 1
// callgrind_annotate callgrind.out > callgrind.out.log
// valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bench 0 0 0 1
// 
// How to execute it with perf:
// perf record -g ./bench 0 0 0 1
// perf report
//
// (https://www.slideshare.net/emBO_Conference/profiling-your-applications-using-the-linux-perf-tools)
// perf record --call-graph dwarf -F 40 -- ./bench 0 0 0 1
// perf record --call-graph dwarf -F 40 -- ./bench 1 0 0 1
// perf report


using namespace at;
using namespace at::native;
using namespace at::native::ti_upsample;


int main(int argc, char** argv)
{

#ifdef INSPECT_ASSEMBLY_CODE

    auto input = at::rand({1, 3, 16, 320, 320});
    int64_t osizes[3] = {8, 256, 256};
    c10::optional<IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

    auto out = ti_upsample_trilinear3d_cpu(input, output_size, false, scale_factors);    

    return 0;

#endif

    int n = 7500;
    bool full_bench = false;
    bool test_all_dims = false;
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
        test_all_dims = (bool) (std::atoi(argv[3]));
    }
    if (argc >= 5) {
        num_threads =  std::atoi(argv[4]);
    }

    std::cout << "Torch config: " << show_config() << std::endl;

    set_num_threads(num_threads);
    std::cout << "Num threads: " << get_num_threads() << std::endl;

#if 0
    {
        std::cout << "--- Test very large sizes: 32x32 -> 2**28 x 16 ---" << std::endl;
        auto input = at::rand({1, 1, 32, 32});
        int64_t osizes[2] = {int64_t(pow(2, 28)), 16};
        c10::optional<IntArrayRef> output_size = osizes;
        c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

        std::cout << "- ti_upsample_bilinear2d_kernel_impl" << std::endl;
        auto out = ti_upsample_bilinear2d_kernel_impl(input, output_size, false, scale_factors);
        std::cout << "- upsample_bilinear2d_cpu" << std::endl;
        auto ref_out = upsample_bilinear2d_cpu(input, output_size, false, scale_factors);

        if (!ref_out.allclose(out)){
            auto mse = (ref_out - out).pow(2.0).mean();
            auto max_e = (ref_out - out).view(-1).abs().max();
            std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
            assert(false);
        }
        return 1;
    }
#endif


#ifdef WITH_ASAN

    std::cout << "- Assert consistency 2D" << std::endl;
    assert_consistency_2d(320, 256, 512);
    assert_consistency_2d(500, 256, 800);
    std::cout << "- Assert consistency 1D" << std::endl;
    assert_consistency_1d();
    std::cout << "- Assert consistency 3D" << std::endl;
    assert_consistency_3d();

#endif


#ifdef INSPECT_3D_CASE_ONLY

    {
        auto t_input = at::rand({1, 16, 320, 320, 3}, at::CPU(at::kFloat));
        t_input = t_input.permute({0, 4, 1, 2, 3});
        int64_t osizes[3] = {8, 256, 256};
        c10::optional<IntArrayRef> output_size = osizes;
        c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
        
        std::cout << "\n- upsample_trilinear3d on channels last" << std::endl;
        native::upsample_trilinear3d(t_input, output_size, false, scale_factors);

        std::cout << "\n- ti_upsample_trilinear3d_cpu on channels last" << std::endl;
        ti_upsample::ti_upsample_trilinear3d_cpu(t_input, output_size, false, scale_factors);
    }

    return 1;
#endif

#ifdef BENCH_3D_ONLY
    std::cout << "\n\n---- Benchmark 3D ----" << std::endl;
    // bench_3d(n / 10, full_bench);
    auto t_input = at::rand({1, 16, 320, 320, 3}, at::CPU(at::kFloat));
    t_input = t_input.permute({0, 4, 1, 2, 3});
    sub_bench_3d(n / 10, t_input, 256, 512);
    std::cout << "\n---- END Benchmark 3D ----" << std::endl;
    return 1;
#endif


#ifdef INSPECT_2D_SLOWDOWN_CASE_ONLY

    {
        auto t_input = at::rand({2, 64, 64, 128}, at::CPU(at::kFloat));
        t_input = t_input.permute({0, 3, 1, 2});
        int64_t osizes[2] = {256, 256};
        c10::optional<IntArrayRef> output_size = osizes;
        c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
        
        std::cout << "\n- upsample_bilinear2d on channels last" << std::endl;
        native::upsample_bilinear2d(t_input, output_size, false, scale_factors);

        std::cout << "\n- ti_upsample_bilinear2d_cpu on channels last" << std::endl;
        ti_upsample::ti_upsample_bilinear2d_cpu(t_input, output_size, false, scale_factors);
    }

    {
        auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
        int64_t osizes[2] = {256, 256};
        c10::optional<IntArrayRef> output_size = osizes;
        c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

        std::cout << "\n- upsample_bilinear2d on channels first" << std::endl;
        native::upsample_bilinear2d(t_input, output_size, false, scale_factors);
        std::cout << "\n- ti_upsample_bilinear2d_cpu on channels first" << std::endl;
        ti_upsample::ti_upsample_bilinear2d_cpu(t_input, output_size, false, scale_factors);
    }

    return 1;
#endif


#ifdef INSPECT_2D_SLOWDOWN_CASE2_ONLY

    {
        auto t_input = at::rand({1, 320, 320, 3}, at::CPU(at::kFloat));
        t_input = t_input.permute({0, 3, 1, 2});
        int64_t osizes[2] = {256, 256};
        c10::optional<IntArrayRef> output_size = osizes;
        c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

        std::cout << "\n- upsample_bilinear2d on channels last" << std::endl;
        native::upsample_bilinear2d(t_input, output_size, false, scale_factors);

        std::cout << "\n- ti_upsample_bilinear2d_cpu on channels last" << std::endl;
        ti_upsample::ti_upsample_bilinear2d_cpu(t_input, output_size, false, scale_factors);
    }

    {
        auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
        int64_t osizes[2] = {256, 256};
        c10::optional<IntArrayRef> output_size = osizes;
        c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

        std::cout << "\n- upsample_bilinear2d on channels first" << std::endl;
        native::upsample_bilinear2d(t_input, output_size, false, scale_factors);
        std::cout << "\n- ti_upsample_bilinear2d_cpu on channels first" << std::endl;
        ti_upsample::ti_upsample_bilinear2d_cpu(t_input, output_size, false, scale_factors);
    }

    return 1;
#endif


#ifdef PERF_INSPECT_2D_SLOWDOWN_CASE2_ONLY

    auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
    int64_t osizes[2] = {256, 256};
    c10::optional<IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

    if (n == 0) {
        std::cout << "\n- upsample_bilinear2d on channels first" << std::endl;
        for (int i=0; i<100; i++) {
            native::upsample_bilinear2d(t_input, output_size, false, scale_factors);
        }
    } else {
        std::cout << "\n- ti_upsample_bilinear2d_cpu on channels last" << std::endl;
        for (int i=0; i<100; i++) {
            ti_upsample::ti_upsample_bilinear2d_cpu(t_input, output_size, false, scale_factors);
        }
    }
    return 0;
#endif


#ifdef BENCH_2D_SLOWDOWN_CASE_ONLY

    auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
    sub_bench_2d(n, t_input, 256, 512);

    // t_input = at::rand({2, 64, 64, 128}, at::CPU(at::kFloat));
    // t_input = t_input.permute({0, 3, 1, 2});
    // sub_bench_2d(n / 10, t_input, 32, 128);

    // t_input = at::rand({32, 64, 64, 128}, at::CPU(at::kFloat));
    // t_input = t_input.permute({0, 3, 1, 2});
    // sub_bench_2d(n / 10, t_input, 32, 128);

    return 1;
#endif


#ifdef BENCH_2D_SLOWDOWN_CL_CASE_ONLY

    // sub_bench_2d_non_contiguous_channel_last(n, 320, 256, 512);
    sub_bench_2d_mingfeima_channel_last(n);

    return 1;
#endif

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
