// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

// Local
#include "bench_helper.h"
#include "interpolate.h"

// #define INSPECT_ASSEMBLY_CODE
#define INSPECT_2D_CASE_ONLY

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

    auto out = ti_upsample_tricubic3d_kernel_impl(input, output_size, false, scale_factors);    

    return 0;

#endif

    int n = 2500;
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


#ifdef INSPECT_2D_CASE_ONLY

    std::cout << "\n\n---- Benchmark 2D ----" << std::endl;

    auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
    sub_bench_2d(n, t_input, 256, 512);

    std::cout << "\n---- END Benchmark 2D ----" << std::endl;

    return 1;
#endif


    std::cout << "\n\n---- Benchmark 2D ----" << std::endl;
    bench_2d(n, full_bench, 320, 256, 512);
    if (full_bench) {
        bench_2d(n, false, 500, 256, 800);
    }        
    std::cout << "\n---- END Benchmark 2D ----" << std::endl;

    return 0;
}



// Check Consistency error

    // int64_t osizes[2] = {16 * 5, 16 * 5};
    // c10::optional<IntArrayRef> output_size = osizes;
    // auto nch = 3;
    // auto sx = 32 * 5;
    // auto sy = 32 * 5;
    // auto t = 1024 * at::arange(1, 1 + 1 * nch * sx * sy, at::CPU(at::kFloat)).reshape({1, nch, sy, sx});

    // // std::cout << "Input:\n";
    // // std::cout << t << std::endl;

    // auto ref_out = at::native::upsample_bicubic2d(t, output_size, false, c10::nullopt);
    // // std::cout << "Ref output:\n";
    // // std::cout << ref_out << std::endl;

    // auto out = ti_upsample_bicubic2d_cpu(t, output_size, false, c10::nullopt);
    // // std::cout << "Output:\n";
    // // std::cout << out << std::endl;

    // if (!ref_out.allclose(out))
    // {
    //     auto mse = (ref_out - out).pow(2.0).mean();
    //     auto max_e = (ref_out - out).view(-1).abs().max();
    //     std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
    //     auto mask = ref_out != out;
    //     int n_err_vals = mask.sum().item<int>();
    //     std::cout << "Number of different vals: " << n_err_vals << std::endl;
    //     std::cout << "Ref output vs TI output (value and diff):" << std::endl;
    //     auto r1 = ref_out.masked_select(mask);
    //     auto diff = (ref_out - out).masked_select(mask);
    //     auto argmax = diff.argmax();        
    //     for (int i=0; i<int(std::min(5, n_err_vals)); i++) {
    //         std::cout << "\t" << r1[i].item<double>() << ", diff=" << diff[i].item<double>() << std::endl;
    //     }
    //     std::cout << "\t" << r1[argmax].item<double>() << ", diff=" << diff[argmax].item<double>() << std::endl;
    // }
