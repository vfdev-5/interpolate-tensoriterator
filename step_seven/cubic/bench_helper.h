#pragma once

// STD
#include <iostream>
#include <chrono>

// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>
// Fails with original pytorch as UpSample.h is missing #pragma once
// #include <ATen/native/UpSample.h>

// Local
#include "interpolate.h"


using namespace at;
using namespace at::native;
using namespace at::indexing;


inline void assert_consistency_bicubic2d(
    Tensor t_input, int isize, int osize, bool align_corners=false,
    c10::optional<double> s_h = c10::nullopt, c10::optional<double> s_w = c10::nullopt
) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({1, 3, isize, isize}, at::CPU(at::kFloat));
    }

    int64_t osizes[2] = {osize, osize};
    c10::optional<IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    double sfs[2] = {0.0, 0.0};
    if (s_h && s_w) {
        output_size = c10::nullopt;
        sfs[0] = *s_h; sfs[1] = *s_w;
        scale_factors = sfs;
    }

    auto ref_out = at::native::upsample_bicubic2d(t_input, output_size, align_corners, scale_factors);
    auto out = ti_upsample_bicubic2d_cpu(t_input, output_size, align_corners, scale_factors);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        std::cout << "Configuration: "
            << isize << " "
            << osize << " "
            << align_corners << " "
            << (s_h.has_value() ? *s_h : 0.0) << " "
            << (s_w.has_value() ? *s_w : 0.0) << " "
            << t_input.dtype() << " "
            << std::endl;
        auto mask = ref_out != out;
        int n_err_vals = mask.sum().item<int>();
        std::cout << "Number of different vals: " << n_err_vals << std::endl;
        std::cout << "Ref output vs TI output (value and diff):" << std::endl;
        auto r1 = ref_out.masked_select(mask);
        auto diff = (ref_out - out).masked_select(mask);
        auto argmax = diff.argmax();        
        for (int i=0; i<int(std::min(5, n_err_vals)); i++) {
            std::cout << "\t" << r1[i].item<double>() << ", diff=" << diff[i].item<double>() << std::endl;
        }
        std::cout << "\t" << r1[argmax].item<double>() << ", diff=" << diff[argmax].item<double>() << std::endl;
        assert(false);
    }
}


inline void assert_consistency_2d(int isize=320, int dn_osize=256, int up_osize=512) {

    auto check = [](const at::Tensor & t, int osize1, int osize2){
        assert_consistency_bicubic2d(t, -1, osize1);
        assert_consistency_bicubic2d(t, -1, osize1, true);
        assert_consistency_bicubic2d(t, -1, -1, false, 1.12, 1.23);
        assert_consistency_bicubic2d(t, -1, -1, true, 1.12, 1.23);
        assert_consistency_bicubic2d(t, -1, osize2);
        assert_consistency_bicubic2d(t, -1, osize2, true);
        assert_consistency_bicubic2d(t, -1, -1, false, 0.77, 0.88);
        assert_consistency_bicubic2d(t, -1, -1, true, 0.77, 0.88);        
    };

    for (auto dtype: {kFloat, kDouble}) {
        auto t_input = at::rand({1, 3, isize, isize}, CPU(dtype));
        check(t_input, dn_osize, up_osize);

        t_input = at::rand({1, isize, isize, 3}, CPU(dtype));
        t_input = t_input.permute({0, 3, 1, 2});
        check(t_input, dn_osize, up_osize);

        t_input = at::rand({1, 3, isize + 100, isize + 100}, CPU(dtype));
        t_input = t_input.index({"...", Slice(None, isize), Slice(None, isize)});
        check(t_input, dn_osize, up_osize);
    }
}


inline void sub_bench_2d(int n, at::Tensor t_input, int dn_osize, int up_osize) {
    std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
    std::cout << "Input is_contiguous memory_format torch.channels_last: " << (t_input.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
    std::cout << "Input is_contiguous : " << (t_input.is_contiguous() ? "true" : "false") << std::endl;

    {
        int64_t osizes[2] = {dn_osize, dn_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_bicubic2d (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = upsample_bicubic2d(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[2] = {dn_osize, dn_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_bicubic2d_cpu (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bicubic2d_cpu(t_input, output_size, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[2] = {up_osize, up_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_bicubic2d (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = upsample_bicubic2d(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[2] = {up_osize, up_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_bicubic2d_cpu (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bicubic2d_cpu(t_input, output_size, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }
}


inline void sub_bench_2d_non_contiguous_channel_first(int n, int isize, int dn_osize, int up_osize) {
    // Non-contiguous case:
    auto t_input = at::rand({1, 3, isize + 100, isize + 100}, at::CPU(at::kFloat));
    t_input = t_input.index({"...", Slice(None, isize), Slice(None, isize)});
    sub_bench_2d(n, t_input, dn_osize, up_osize);
}


inline void sub_bench_2d_contiguous_channel_first(int n, int isize, int dn_osize, int up_osize) {
    auto t_input = at::rand({1, 3, isize, isize}, at::CPU(at::kFloat));
    sub_bench_2d(n, t_input, dn_osize, up_osize);
}


inline void sub_bench_2d_mingfeima_channel_last(int n, int isize, int dn_osize, int up_osize) {
    // ---- Test size as in https://github.com/mingfeima/op_bench-py
    n = n / 10;

    {
        int64_t osizes[2] = {128, 128};
        IntArrayRef output_size(osizes);

        std::cout << "\n1 - Test size as in https://github.com/mingfeima/op_bench-py" << std::endl;
        auto t_input = at::rand({32, 64, 64, 128}, at::CPU(at::kFloat));
        t_input = t_input.permute({0, 3, 1, 2});
        std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << (t_input.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
        std::cout << "Input is_contiguous : " << (t_input.is_contiguous() ? "true" : "false") << std::endl;

        assert_consistency_bicubic2d(t_input, -1, 128);
        {    
            std::cout << "\n- Bench upsample_bicubic2d (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = upsample_bicubic2d(t_input, output_size, false);
                auto result = ref_out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }

        {
            std::cout << "\n- Bench ti_upsample_bicubic2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto out = ti_upsample_bicubic2d_cpu(t_input, output_size, false);
                auto result = out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }
    }

    {
        int64_t osizes[2] = {128, 128};
        IntArrayRef output_size(osizes);

        std::cout << "\n2 - Test size as in https://github.com/mingfeima/op_bench-py" << std::endl;
        auto t_input = at::rand({32, 128, 64, 64}, at::CPU(at::kFloat));
        std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << (t_input.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
        std::cout << "Input is_contiguous : " << (t_input.is_contiguous() ? "true" : "false") << std::endl;

        assert_consistency_bicubic2d(t_input, -1, 128);
        {    
            std::cout << "\n- Bench upsample_bicubic2d (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = upsample_bicubic2d(t_input, output_size, false);
                auto result = ref_out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }

        {
            std::cout << "\n- Bench ti_upsample_bicubic2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto out = ti_upsample_bicubic2d_cpu(t_input, output_size, false);
                auto result = out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }
    }
} 


inline void sub_bench_2d_non_contiguous_channel_last(int n, int isize, int dn_osize, int up_osize) {
    auto t_input = at::rand({1, isize, isize, 3}, at::CPU(at::kFloat));
    t_input = t_input.permute({0, 3, 1, 2});  
    sub_bench_2d(n, t_input, dn_osize, up_osize);
} 


inline void bench_2d(int n, bool full_bench, int isize, int dn_osize, int up_osize) {

    assert_consistency_2d(isize, dn_osize, up_osize);

    sub_bench_2d_contiguous_channel_first(n, isize, dn_osize, up_osize);
    sub_bench_2d_non_contiguous_channel_first(n, isize, dn_osize, up_osize);

    if (!full_bench)
        return;

    sub_bench_2d_non_contiguous_channel_last(n, isize, dn_osize, up_osize);

    sub_bench_2d_mingfeima_channel_last(n, isize, dn_osize, up_osize);
}
