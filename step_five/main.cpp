// STD
#include <iostream>
#include <chrono>

// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

#ifdef WITH_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#endif

// Local
#include "interpolate.h"


#define NUM_THREADS 6


void assert_consistency_bilinear2d(
    at::Tensor t_input, int isize, int osize, bool align_corners=false,
    c10::optional<double> s_h = c10::nullopt, c10::optional<double> s_w = c10::nullopt
) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({1, 3, isize, isize}, at::CPU(at::kFloat));
    }

    int64_t osizes[2] = {osize, osize};
    c10::optional<at::IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    double sfs[2] = {0.0, 0.0};
    if (s_h && s_w) {
        output_size = c10::nullopt;
        sfs[0] = *s_h; sfs[1] = *s_w;
        scale_factors = sfs;
    }

    auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, output_size, align_corners, scale_factors);
    auto out = ti_upsample_bilinear2d_kernel_impl(t_input, output_size, align_corners, scale_factors);

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


void assert_consistency_linear1d(
    at::Tensor t_input, int isize, int osize, bool align_corners=false,
    c10::optional<double> s_w = c10::nullopt
) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({4, 512, isize}, at::CPU(at::kFloat));
    }

    int64_t osizes[1] = {osize, };
    c10::optional<at::IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    double sfs[1] = {0.0, };
    if (s_w) {
        output_size = c10::nullopt;
        sfs[0] = *s_w;
        scale_factors = sfs;
    }

    auto ref_out = at::native::upsample_linear1d_cpu(t_input, output_size, align_corners, scale_factors);
    auto out = ti_upsample_linear1d_kernel_impl(t_input, output_size, align_corners, scale_factors);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        std::cout << "Configuration: "
            << isize << " "
            << osize << " "
            << align_corners << " "
            << (s_w.has_value() ? *s_w : 0.0) << " "
            << std::endl;
        assert(false);
    }
}


void assert_consistency_trilinear3d(
    at::Tensor t_input, int isize, int osize, bool align_corners=false,
    c10::optional<double> s_d = c10::nullopt, c10::optional<double> s_h = c10::nullopt, c10::optional<double> s_w = c10::nullopt
) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({1, 3, 16, isize, isize}, at::CPU(at::kFloat));
    }

    int osize_2 = (float(t_input.size(3)) / osize > 1.0) ? t_input.size(2) / 2 : t_input.size(2) * 2;
    int64_t osizes[3] = {osize_2, osize, osize};
    c10::optional<at::IntArrayRef> output_size = osizes;
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt;
    double sfs[3] = {0.0, 0.0, 0.0};
    if (s_d && s_h && s_w) {
        output_size = c10::nullopt;
        sfs[0] = *s_d; sfs[1] = *s_h; sfs[2] = *s_w;
        scale_factors = sfs;
    }

    auto ref_out = at::native::upsample_trilinear3d_cpu(t_input, output_size, align_corners, scale_factors);
    auto out = ti_upsample_trilinear3d_kernel_impl(t_input, output_size, align_corners, scale_factors);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        std::cout << "Configuration: "
            << isize << " "
            << osize << " "
            << align_corners << " "
            << (s_d.has_value() ? *s_d : 0.0) << " "
            << (s_h.has_value() ? *s_h : 0.0) << " "
            << (s_w.has_value() ? *s_w : 0.0) << " "
            << std::endl;
        assert(false);
    }
}


int bench_2d(int n, bool full_bench, int isize=320, int dn_osize=256, int up_osize=512) {

    auto t_input = at::rand({1, 3, isize, isize}, at::CPU(at::kFloat));
    std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
 
    at::set_num_threads(NUM_THREADS);
    std::cout << "Num threads: " << at::get_num_threads() << std::endl;

    assert_consistency_bilinear2d(t_input, -1, dn_osize);
    assert_consistency_bilinear2d(t_input, -1, dn_osize, true);
    // BELOW FAILS WITH PRECISION ERROR:
    // Error: mse=5.45085e-12, max e=2.14428e-05
    // assert_consistency_bilinear2d(t_input, -1, dn_osize, false, 1.12, 1.23);
    assert_consistency_bilinear2d(t_input, -1, dn_osize, true, 1.12, 1.23);
    assert_consistency_bilinear2d(t_input, -1, up_osize);
    assert_consistency_bilinear2d(t_input, -1, up_osize, true);
    // BELOW FAILS WITH PRECISION ERROR:
    // Error: mse=5.32594e-12, max e=2.05934e-05
    // assert_consistency_bilinear2d(t_input, -1, up_osize, false, 0.77, 0.88);
    assert_consistency_bilinear2d(t_input, -1, up_osize, true, 0.77, 0.88);

    auto t_input_channel_last = at::rand({1, 3, isize, isize}, at::CPU(at::kFloat));
    assert_consistency_bilinear2d(t_input, -1, up_osize, true, 0.77, 0.88);

    // Time benchmark
    {
        int64_t osizes[2] = {dn_osize, dn_osize};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[2] = {dn_osize, dn_osize};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, output_size, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        int64_t osizes[2] = {up_osize, up_osize};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[2] = {up_osize, up_osize};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, output_size, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    // ---- benchmark test size as in https://github.com/mingfeima/op_bench-py
    n = n / 10;

    {
        int64_t osizes[2] = {128, 128};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n1 - Benchmark test size as in https://github.com/mingfeima/op_bench-py" << std::endl;
        t_input = at::rand({32, 64, 64, 128}, at::CPU(at::kFloat));
        t_input = t_input.permute({0, 3, 1, 2});
        std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << t_input.is_contiguous(at::MemoryFormat::ChannelsLast) << std::endl;
        std::cout << "Input is_contiguous : " << t_input.is_contiguous() << std::endl;

        assert_consistency_bilinear2d(t_input, -1, 128);
        {    
            std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, output_size, false);
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
                auto out = ti_upsample_bilinear2d_kernel_impl(t_input, output_size, false);
                auto result = out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }
    }

    {
        int64_t osizes[2] = {128, 128};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n2 - Benchmark test size as in https://github.com/mingfeima/op_bench-py" << std::endl;
        t_input = at::rand({32, 128, 64, 64}, at::CPU(at::kFloat));
        std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << t_input.is_contiguous(at::MemoryFormat::ChannelsLast) << std::endl;
        std::cout << "Input is_contiguous : " << t_input.is_contiguous() << std::endl;

        assert_consistency_bilinear2d(t_input, -1, 128);
        {    
            std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to 128x128" << std::endl;
            auto start = std::chrono::steady_clock::now();
            for (int i=0; i<n; i++)
            {
                auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, output_size, false);
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
                auto out = ti_upsample_bilinear2d_kernel_impl(t_input, output_size, false);
                auto result = out.size(0);
            }
            auto end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed_seconds = end - start;
            std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
        }
    }
    return 0;
}


int bench_1d(int n, bool full_bench) {

    auto t_input = at::rand({4, 512, 320}, at::CPU(at::kFloat));
    std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
 
    at::set_num_threads(NUM_THREADS);
    std::cout << "Num threads: " << at::get_num_threads() << std::endl;

    assert_consistency_linear1d(t_input, -1, 256);
    assert_consistency_linear1d(t_input, -1, 256, true);
    // Error: mse=4.15286e-12, max e=1.49012e-05
    // assert_consistency_linear1d(t_input, -1, 256, false, 1.12);
    assert_consistency_linear1d(t_input, -1, 256, true, 1.12);
    assert_consistency_linear1d(t_input, -1, 512);
    assert_consistency_linear1d(t_input, -1, 512, true);
    // Error: mse=3.9745e-12, max e=1.46627e-05
    // assert_consistency_linear1d(t_input, -1, 512, false, 0.77);
    assert_consistency_linear1d(t_input, -1, 512, true, 0.77);

    // Time benchmark
    {
        int64_t osizes[1] = {256, };
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_linear1d_cpu (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_linear1d_cpu(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[1] = {256, };
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_linear1d_cpu (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_linear1d_kernel_impl(t_input, output_size, false);
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
        at::IntArrayRef output_size(osizes);
        
        std::cout << "\n- Bench upsample_linear1d_cpu (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_linear1d_cpu(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[1] = {512, };
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_linear1d_cpu (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_linear1d_kernel_impl(t_input, output_size, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 0;
}


int bench_3d(int n, bool full_bench) {
    auto t_input = at::rand({1, 3, 16, 320, 320}, at::CPU(at::kFloat));
    std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
 
    at::set_num_threads(NUM_THREADS);
    std::cout << "Num threads: " << at::get_num_threads() << std::endl;

    assert_consistency_trilinear3d(t_input, -1, 256);
    assert_consistency_trilinear3d(t_input, -1, 256, true);
    assert_consistency_trilinear3d(t_input, -1, 256, false, 1.12);
    assert_consistency_trilinear3d(t_input, -1, 256, true, 1.12);
    assert_consistency_trilinear3d(t_input, -1, 512);
    assert_consistency_trilinear3d(t_input, -1, 512, true);
    assert_consistency_trilinear3d(t_input, -1, 512, false, 0.77);
    assert_consistency_trilinear3d(t_input, -1, 512, true, 0.77);

    // Time benchmark
    {
        int64_t osizes[3] = {8, 256, 256};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_trilinear3d_cpu (" << n << " rounds) - downsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_trilinear3d_cpu(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[3] = {8, 256, 256};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_trilinear3d_kernel_impl (" << n << " rounds) - downsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_trilinear3d_kernel_impl(t_input, output_size, false);
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
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench upsample_trilinear3d_cpu (" << n << " rounds) - upsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_trilinear3d_cpu(t_input, output_size, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        int64_t osizes[3] = {32, 512, 512};
        at::IntArrayRef output_size(osizes);

        std::cout << "\n- Bench ti_upsample_trilinear3d_kernel_impl (" << n << " rounds) - upsampling to " << output_size << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_trilinear3d_kernel_impl(t_input, output_size, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 0;
}

#ifdef WITH_OPENCV

int bench_opencv_2d(int n, bool full_bench, int isize=320, int dn_osize=256, int up_osize=512) {

    cv::Mat t_input(isize, isize, CV_32FC3);
    cv::randu(t_input, 0.0, 1.0);
    std::cout << "\nInput tensor: " << t_input.size() << std::endl;
 
    // Time benchmark
    {
        std::cout << "\n- Bench cv::resize (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        cv::Mat dst;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            cv::resize(t_input, dst, cv::Size(dn_osize, dn_osize), 0.0, 0.0, cv::INTER_LINEAR);
            auto result = dst.size[0];
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        std::cout << "\n- Bench cv::resize (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        cv::Mat dst;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            cv::resize(t_input, dst, cv::Size(up_osize, up_osize), 0.0, 0.0, cv::INTER_LINEAR);
            auto result = dst.size[0];
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 0;
}

int bench_opencv_2d_uint8(int n, bool full_bench, int isize=320, int dn_osize=256, int up_osize=512) {

    cv::Mat t_input(isize, isize, CV_8UC3);
    cv::randu(t_input, 0, 256);
    std::cout << "\nInput tensor: " << t_input.size() << std::endl;
 
    // Time benchmark
    {
        std::cout << "\n- Bench cv::resize (" << n << " rounds) - uint8 downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        cv::Mat dst;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            cv::resize(t_input, dst, cv::Size(dn_osize, dn_osize), 0.0, 0.0, cv::INTER_LINEAR);
            auto result = dst.size[0];
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        std::cout << "\n- Bench cv::resize (" << n << " rounds) - uint8 upsampling to " << up_osize << "x" << up_osize << std::endl;
        cv::Mat dst;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            cv::resize(t_input, dst, cv::Size(up_osize, up_osize), 0.0, 0.0, cv::INTER_LINEAR);
            auto result = dst.size[0];
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    return 0;
}

#endif


int main(int argc, char** argv)
{
    auto n = 7500;
    bool full_bench = false;
    bool test_all_dims = false;

    at::manual_seed(10);

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

    std::cout << "Torch config: " << at::show_config() << std::endl;

#ifdef WITH_OPENCV
    cv::setNumThreads(NUM_THREADS);
    auto cv_build_info = cv::getBuildInformation();
    std::cout << cv_build_info.substr(0, 2280) << std::endl;
#endif
    
    std::cout << "\n\n---- Benchmark 2D ----" << std::endl;
    bench_2d(n, full_bench, 320, 256);
    bench_2d(n, full_bench, 1024, 512);

    // Attempt to check index overflow
    // assert_consistency_bilinear2d(at::Tensor(), 28000, 1200);

#ifdef WITH_OPENCV
    std::cout << "\n\n---- Benchmark OpenCV 2D ----" << std::endl;
    bench_opencv_2d(n, full_bench, 320, 256);
    bench_opencv_2d_uint8(n, full_bench, 320, 256);
    bench_opencv_2d(n, full_bench, 1024, 512);
    bench_opencv_2d_uint8(n, full_bench, 1024, 512);

#endif

    if (!test_all_dims) return 0;

    std::cout << "\n\n---- Benchmark 1D ----" << std::endl;
    bench_1d(n, full_bench);

    std::cout << "\n\n---- Benchmark 3D ----" << std::endl;
    bench_3d(n / 10, full_bench);

    return 0;
}