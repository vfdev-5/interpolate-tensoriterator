// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

// Local
#include "interpolate.h"
#include "bench_helper.h"


#define INSPECT_ASSEMBLY_CODE
// #define BENCH_2D_C1_ONLY
// #define BENCH_3D_ONLY
// #define BENCH_2D_CF_MFMA_ONLY
// #define BENCH_2D_CL_ONLY
// #define USE_ALWAYS_INDEX64


using namespace at;
using namespace at::native;


int main(int argc, char** argv)
{

#ifdef INSPECT_ASSEMBLY_CODE

    // 3d case

    // auto input = at::rand({1, 3, 16, 320, 320});
    // int64_t osizes[3] = {8, 256, 256};
    // c10::optional<IntArrayRef> output_size = osizes;
    // c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    // auto out = ti_upsample::upsample_nearest3d_cpu(input, output_size, false, scale_factors);
    // return 0;


    // 2d (1,H,W) float
    auto input = at::rand({1, 1, 500, 500});
    int64_t osizes[2] = {128, 128};
    c10::optional<IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    auto out = ti_upsample::upsample_nearest2d_cpu(input, output_size, scale_factors);
    return out.sum().item<int>();

#endif

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

#if 0
    {
        std::cout << "--- Test very large sizes: 32x32 -> 2**28 x 16 ---" << std::endl;
        auto input = at::rand({1, 1, 32, 32});
        int64_t osizes[2] = {int64_t(pow(2, 28)), 16};
        c10::optional<IntArrayRef> output_size = osizes;
        c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;

        std::cout << "- TI upsample_nearest2d_kernel_impl" << std::endl;
        auto out = ti_upsample::upsample_nearest2d_cpu(input, output_size, false, scale_factors);
        std::cout << "- upsample_nearest2d_cpu" << std::endl;
        auto ref_out = native::upsample_nearest2d_cpu(input, output_size, false, scale_factors);

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


#ifdef BENCH_3D_ONLY
    std::cout << "\n\n---- Benchmark 3D ----" << std::endl;
    bench_3d(n / 10, full_bench);
    std::cout << "\n---- END Benchmark 3D ----" << std::endl;
    return 1;
#endif

#ifdef BENCH_2D_C1_ONLY
    std::cout << "\n\n---- Benchmark 2D Channel=1 ----" << std::endl;

    {
        auto t_input = at::randint(0, 256, {1, 1, 500, 500}, at::CPU(at::kFloat));
        sub_bench_2d(n, t_input, 120, -1);
    }
    {
        auto t_input = at::randint(0, 256, {1, 1, 500, 500}, at::CPU(at::kByte));
        sub_bench_2d(n, t_input, 120, -1);
    }

    std::cout << "\n---- END Benchmark 2D Channel=1 ----" << std::endl;
    return 1;
#endif

#ifdef BENCH_2D_CF_MFMA_ONLY
    std::cout << "\n\n---- Benchmark 2D MingfeiMa CF ----" << std::endl;

    {
        auto t_input = at::rand({32, 128, 64, 64}, at::CPU(at::kFloat));
        sub_bench_2d(n, t_input, 32, 128);
    }
    {
        auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
        sub_bench_2d(n, t_input, 256, 512);
    }

    std::cout << "\n---- END Benchmark 2D MingfeiMa CF ----" << std::endl;
    return 1;
#endif

#ifdef BENCH_2D_CL_ONLY
    std::cout << "\n\n---- Benchmark 2D CL ----" << std::endl;

    {
        auto t_input = at::rand({1, 320, 320, 3}, at::CPU(at::kFloat));
        t_input = t_input.permute({0, 3, 1, 2});
        sub_bench_2d(n, t_input, 256, 512);
    }

    std::cout << "\n---- END Benchmark 2D CL ----" << std::endl;
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
