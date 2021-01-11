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
    std::cout << "Input tensor: " << t_input.sizes() << std::endl;
 
    at::set_num_threads(6);
    std::cout << "Num threads: " << at::get_num_threads() << std::endl;

    auto n = 5000;
    if (argc == 2)
    {        
        n = std::atoi(argv[1]);
    }

    // Check consistency:
    {
        std::cout << "\n- Check consistency (downsampling to 256x256): ";
        auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {256, 256}, false);
        auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {256, 256});

        if (!ref_out.allclose(out)){
            auto mse = (ref_out - out).pow(2.0).mean();
            auto max_e = (ref_out - out).view(-1).abs().max();
            std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
            return 1;
        }

        std::cout << "OK" << std::endl;
    }
    {
        std::cout << "\n- Check consistency (upsampling to 512x512): ";
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
        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - downsampling to 256x256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {256, 256}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - downsampling to 256x256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {256, 256});
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 1;
    TEMPORARY DISABLE ALL OTHER BENCHMARKS

    {
        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 512x512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {512, 512}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 512x512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {512, 512});
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    // ---- benchmark test size as in https://github.com/mingfeima/op_bench-py
    n = n / 10;

    {
        std::cout << "\n1 - Benchmark test size as in https://github.com/mingfeima/op_bench-py" << std::endl;
        t_input = at::rand({32, 64, 64, 128}, at::CPU(at::kFloat));
        t_input = t_input.permute({0, 3, 1, 2});
        std::cout << "Input tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << t_input.is_contiguous(at::MemoryFormat::ChannelsLast) << std::endl;
        std::cout << "Input is_contiguous : " << t_input.is_contiguous() << std::endl;

        // Check consistency:
        {
            std::cout << "\n- Check consistency (upsampling to 128x128): ";
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {128, 128}, false);
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {128, 128});

            if (!ref_out.allclose(out)){
                auto mse = (ref_out - out).pow(2.0).mean();
                auto max_e = (ref_out - out).view(-1).abs().max();
                std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
                return 1;
            }

            std::cout << "OK" << std::endl;
        }

        {    
            std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {128, 128}, false);
                auto result = ref_out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }

        {
            std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {128, 128});
                auto result = out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }
    }

    {
        std::cout << "\n2 - Benchmark test size as in https://github.com/mingfeima/op_bench-py" << std::endl;
        t_input = at::rand({32, 128, 64, 64}, at::CPU(at::kFloat));
        std::cout << "Input tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << t_input.is_contiguous(at::MemoryFormat::ChannelsLast) << std::endl;
        std::cout << "Input is_contiguous : " << t_input.is_contiguous() << std::endl;


        // Check consistency:
        {
            std::cout << "\n- Check consistency (upsampling to 128x128): ";
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {128, 128}, false);
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {128, 128});

            if (!ref_out.allclose(out)){
                auto mse = (ref_out - out).pow(2.0).mean();
                auto max_e = (ref_out - out).view(-1).abs().max();
                std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
                return 1;
            }

            std::cout << "OK" << std::endl;
        }

        {    
            std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {128, 128}, false);
                auto result = ref_out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }

        // auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {128, 128});

        {
            std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {128, 128});
                auto result = out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }
    }

    return 0;
}