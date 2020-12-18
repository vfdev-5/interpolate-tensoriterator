// STD
#include <iostream>
#include <chrono>

// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

// Local
#include "interpolate.h"


int main(int argc, char** argv)
{
    auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
    std::cout << "Input tensor: "<< t_input.sizes() << std::endl;
 
    at::set_num_threads(6);
    std::cout << "Num threads: " << at::get_num_threads() << std::endl;

    auto n = 5000;
    if (argc == 2)
    {        
        n = std::atoi(argv[1]);
    }

    // Check consistency:
    {
        std::cout << "\n- Check consistency (downsampling): ";
        auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {256, 256}, false);
        auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {256, 256});

        if (!ref_out.allclose(out)){
            std::cout << "Error" << std::endl;
            return 1;
        }

        std::cout << "OK" << std::endl;
    }
    {
        std::cout << "\n- Check consistency (upsampling): ";
        auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {512, 512}, false);
        auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {512, 512});

        if (!ref_out.allclose(out)){
            std::cout << "Error" << std::endl;
            return 1;
        }

        std::cout << "OK" << std::endl;
    }


    // Time benchmark
    {
        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - downsampling" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {256, 256}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time: " << elapsed_seconds.count() / n << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - downsampling" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {256, 256});
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time: " << elapsed_seconds.count() / n << std::endl;
    }

    {
        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {512, 512}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time: " << elapsed_seconds.count() / n << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - upsampling" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {512, 512});
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time: " << elapsed_seconds.count() / n << std::endl;
    }    
    return 0;
}