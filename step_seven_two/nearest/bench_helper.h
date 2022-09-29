#pragma once

// STD
#include <iostream>
#include <chrono>

// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>
#include <ATen/native/UpSample.h>

// Local
#include "interpolate.h"


using namespace at;
using namespace at::indexing;
using namespace at::native;


inline void assert_consistency_nearest2d(
    Tensor t_input, int isize, int osize,
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

    auto ref_out = native::upsample_nearest2d(t_input, output_size, scale_factors);
    auto out = ti_upsample::upsample_nearest2d_cpu(t_input, output_size, scale_factors);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        std::cout << "Configuration: "
            << isize << " "
            << osize << " "
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


inline void assert_consistency_nearest1d(
    Tensor t_input, int isize, int osize,
    c10::optional<double> s_w = c10::nullopt
) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({4, 512, isize}, at::CPU(at::kFloat));
    }

    int64_t osizes[1] = {osize, };
    c10::optional<IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    double sfs[1] = {0.0, };
    if (s_w) {
        output_size = c10::nullopt;
        sfs[0] = *s_w;
        scale_factors = sfs;
    }

    auto ref_out = native::upsample_nearest1d(t_input, output_size, scale_factors);
    auto out = ti_upsample::upsample_nearest1d_cpu(t_input, output_size, scale_factors);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        std::cout << "Configuration: "
            << isize << " "
            << osize << " "
            << (s_w.has_value() ? *s_w : 0.0) << " "
            << t_input.dtype() << " "
            << std::endl;
        assert(false);
    }
}


inline void assert_consistency_nearest3d(
    Tensor t_input, int isize, int osize,
    c10::optional<double> s_d = c10::nullopt, c10::optional<double> s_h = c10::nullopt, c10::optional<double> s_w = c10::nullopt
) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({1, 3, 16, isize, isize}, at::CPU(at::kFloat));
    }

    int osize_2 = (float(t_input.size(3)) / osize > 1.0) ? t_input.size(2) / 2 : t_input.size(2) * 2;
    int64_t osizes[3] = {osize_2, osize, osize};
    c10::optional<IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    double sfs[3] = {0.0, 0.0, 0.0};
    if (s_d && s_h && s_w) {
        output_size = c10::nullopt;
        sfs[0] = *s_d; sfs[1] = *s_h; sfs[2] = *s_w;
        scale_factors = sfs;
    }

    auto ref_out = native::upsample_nearest3d_cpu(t_input, output_size, scale_factors);
    auto out = ti_upsample::upsample_nearest3d_cpu(t_input, output_size, scale_factors);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        std::cout << "Configuration: "
            << isize << " "
            << osize << " "
            << (s_d.has_value() ? *s_d : 0.0) << " "
            << (s_h.has_value() ? *s_h : 0.0) << " "
            << (s_w.has_value() ? *s_w : 0.0) << " "
            << t_input.dtype() << " "
            << std::endl;
        assert(false);
    }
}


inline void assert_consistency_2d(int isize=320, int dn_osize=256, int up_osize=512) {

    auto check = [](const at::Tensor & t, int osize1, int osize2){
        assert_consistency_nearest2d(t, -1, osize1);
        assert_consistency_nearest2d(t, -1, -1, 1.12, 1.23);
        assert_consistency_nearest2d(t, -1, osize2);
        assert_consistency_nearest2d(t, -1, -1, 0.77, 0.88);
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

    auto t_input = at::randint(0, 256, {1, 3, isize, isize}, CPU(kByte));
    check(t_input, dn_osize, up_osize);

    t_input = at::randint(0, 256, {1, isize, isize, 3}, CPU(kByte));
    t_input = t_input.permute({0, 3, 1, 2});
    check(t_input, dn_osize, up_osize);

    t_input = at::randint(0, 256, {1, 3, isize + 100, isize + 100}, CPU(kByte));
    t_input = t_input.index({"...", Slice(None, isize), Slice(None, isize)});
    check(t_input, dn_osize, up_osize);
}


inline void sub_bench_2d(int n, at::Tensor t_input, int dn_osize, int up_osize) {
    std::cout << "\nInput tensor: " << t_input.sizes() << " " << t_input.scalar_type() << std::endl;
    std::cout << "Input is_contiguous memory_format torch.channels_last: " << (t_input.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
    std::cout << "Input is_contiguous : " << (t_input.is_contiguous() ? "true" : "false") << std::endl;

    {
        int64_t osizes[2] = {dn_osize, dn_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_nearest2d (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = native::upsample_nearest2d(t_input, output_size, c10::nullopt);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[2] = {dn_osize, dn_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench TI upsample_nearest2d (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample::upsample_nearest2d_cpu(t_input, output_size);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (up_osize < 0)
    {
        return;
    }

    {
        int64_t osizes[2] = {up_osize, up_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_nearest2d (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = native::upsample_nearest2d(t_input, output_size, c10::nullopt);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[2] = {up_osize, up_osize};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench TI upsample_nearest2d (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample::upsample_nearest2d_cpu(t_input, output_size);
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


inline void sub_bench_2d_mingfeima_channel_last(int n) {
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

        assert_consistency_nearest2d(t_input, -1, 128);
        {
            std::cout << "\n- Bench upsample_nearest2d (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = native::upsample_nearest2d(t_input, output_size, c10::nullopt);
                auto result = ref_out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }

        {
            std::cout << "\n- Bench TI upsample_nearest2d (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto out = ti_upsample::upsample_nearest2d_cpu(t_input, output_size);
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

        assert_consistency_nearest2d(t_input, -1, 128);
        {
            std::cout << "\n- Bench upsample_nearest2d (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = native::upsample_nearest2d(t_input, output_size, c10::nullopt);
                auto result = ref_out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }

        {
            std::cout << "\n- Bench TI upsample_nearest2d (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto out = ti_upsample::upsample_nearest2d_cpu(t_input, output_size);
                auto result = out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }
    }
}


inline void sub_bench_2d_non_contiguous_channel_last(int n, int isize, int dn_osize, int up_osize, int n_channels=3) {
    auto t_input = at::rand({1, isize, isize, n_channels}, at::CPU(at::kFloat));
    t_input = t_input.permute({0, 3, 1, 2});
    sub_bench_2d(n, t_input, dn_osize, up_osize);
}


inline void bench_2d(int n, bool full_bench, int isize, int dn_osize, int up_osize) {

    assert_consistency_2d(isize, dn_osize, up_osize);

    sub_bench_2d_contiguous_channel_first(n, isize, dn_osize, up_osize);
    sub_bench_2d_non_contiguous_channel_last(n, isize, dn_osize, up_osize);
    sub_bench_2d_non_contiguous_channel_first(n, isize, dn_osize, up_osize);

    if (!full_bench)
        return;

    sub_bench_2d_mingfeima_channel_last(n);
}


inline void assert_consistency_1d() {
    for (auto dtype: {kFloat, kDouble}) {
        auto t_input = at::rand({4, 512, 320}, at::CPU(dtype));
        assert_consistency_nearest1d(t_input, -1, 256);
        assert_consistency_nearest1d(t_input, -1, -1, 1.12);
        assert_consistency_nearest1d(t_input, -1, 512);
        assert_consistency_nearest1d(t_input, -1, -1, 0.77);

        auto t_input_channel_last = at::rand({1, 320, 512}, at::CPU(dtype));
        t_input_channel_last = t_input_channel_last.permute({0, 2, 1});
        assert_consistency_nearest1d(t_input_channel_last, -1, 256);
        assert_consistency_nearest1d(t_input_channel_last, -1, -1, 0.77);
        assert_consistency_nearest1d(t_input_channel_last, -1, -1, 1.23);
    }

    auto t_input = at::randint(0, 256, {4, 512, 320}, at::CPU(kByte));
    assert_consistency_nearest1d(t_input, -1, 256);
    assert_consistency_nearest1d(t_input, -1, -1, 1.12);
    assert_consistency_nearest1d(t_input, -1, 512);
    assert_consistency_nearest1d(t_input, -1, -1, 0.77);

    auto t_input_channel_last = at::randint(0, 256, {1, 320, 512}, at::CPU(kByte));
    t_input_channel_last = t_input_channel_last.permute({0, 2, 1});
    assert_consistency_nearest1d(t_input_channel_last, -1, 256);
    assert_consistency_nearest1d(t_input_channel_last, -1, -1, 0.77);
    assert_consistency_nearest1d(t_input_channel_last, -1, -1, 1.23);
}


inline int bench_1d(int n, bool full_bench) {

    assert_consistency_1d();

    auto t_input = at::rand({4, 512, 320}, at::CPU(at::kFloat));
    std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
    std::cout << "Input is_contiguous memory_format torch.channels_last: " << (t_input.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
    std::cout << "Input is_contiguous : " << (t_input.is_contiguous() ? "true" : "false") << std::endl;

    // Time benchmark
    {
        int64_t osizes[1] = {256, };
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_nearest1d (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = native::upsample_nearest1d(t_input, output_size, c10::nullopt);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[1] = {256, };
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench TI upsample_nearest1d (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample::upsample_nearest1d_cpu(t_input, output_size);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        int64_t osizes[1] = {512, };
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_nearest1d (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = native::upsample_nearest1d(t_input, output_size, c10::nullopt);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[1] = {512, };
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench TI upsample_nearest1d (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample::upsample_nearest1d_cpu(t_input, output_size);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 0;
}


inline void assert_consistency_3d() {

    for (auto dtype: {kFloat, kDouble}) {
        auto t_input = at::rand({1, 3, 16, 320, 320}, at::CPU(dtype));
        assert_consistency_nearest3d(t_input, -1, 256);
        assert_consistency_nearest3d(t_input, -1, -1, 1.12, 1.12, 1.12);
        assert_consistency_nearest3d(t_input, -1, 512);
        assert_consistency_nearest3d(t_input, -1, -1, 0.77, 0.77, 0.77);

        auto t_input_channel_last = at::rand({1, 16, 320, 320, 3}, at::CPU(dtype));
        t_input_channel_last = t_input_channel_last.permute({0, 4, 1, 2, 3});
        assert_consistency_nearest3d(t_input_channel_last, -1, 256);
        assert_consistency_nearest3d(t_input_channel_last, -1, -1, 0.77, 0.77, 0.77);
        assert_consistency_nearest3d(t_input_channel_last, -1, -1, 1.23, 1.23, 1.23);
    }
    auto t_input = at::randint(0, 256, {1, 3, 16, 320, 320}, at::CPU(kByte));
    assert_consistency_nearest3d(t_input, -1, 256);
    assert_consistency_nearest3d(t_input, -1, -1, 1.12, 1.12, 1.12);
    assert_consistency_nearest3d(t_input, -1, 512);
    assert_consistency_nearest3d(t_input, -1, -1, 0.77, 0.77, 0.77);

    auto t_input_channel_last = at::randint(0, 256, {1, 16, 320, 320, 3}, at::CPU(kByte));
    t_input_channel_last = t_input_channel_last.permute({0, 4, 1, 2, 3});
    assert_consistency_nearest3d(t_input_channel_last, -1, 256);
    assert_consistency_nearest3d(t_input_channel_last, -1, -1, 0.77, 0.77, 0.77);
    assert_consistency_nearest3d(t_input_channel_last, -1, -1, 1.23, 1.23, 1.23);
}


inline int bench_3d(int n, bool full_bench) {

    assert_consistency_3d();

    auto t_input = at::rand({1, 3, 16, 320, 320}, at::CPU(at::kFloat));
    std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
    std::cout << "Input is_contiguous memory_format torch.channels_last: " << (t_input.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
    std::cout << "Input is_contiguous : " << (t_input.is_contiguous() ? "true" : "false") << std::endl;

    // Time benchmark
    {
        int64_t osizes[3] = {8, 256, 256};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_nearest3d (" << n << " rounds) - downsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = native::upsample_nearest3d_cpu(t_input, output_size, c10::nullopt);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[3] = {8, 256, 256};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench TI upsample_nearest3d_cpu (" << n << " rounds) - downsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample::upsample_nearest3d_cpu(t_input, output_size);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        int64_t osizes[3] = {32, 512, 512};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_nearest3d (" << n << " rounds) - upsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = native::upsample_nearest3d_cpu(t_input, output_size, c10::nullopt);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[3] = {32, 512, 512};
        IntArrayRef output_size(osizes);

        std::cout << "\n- Bench TI upsample_nearest3d_cpu (" << n << " rounds) - upsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample::upsample_nearest3d_cpu(t_input, output_size);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 0;
}
