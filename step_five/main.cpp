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


void assert_consistency_bilinear2d(at::Tensor t_input, int isize, int osize, bool align_corners=false) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({1, 3, isize, isize}, at::CPU(at::kFloat));
    }

    auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {osize, osize}, align_corners);
    auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {osize, osize}, align_corners);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        assert(false);
    }
}


void assert_consistency_linear1d(at::Tensor t_input, int isize, int osize, bool align_corners=false) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({4, 512, isize}, at::CPU(at::kFloat));
    }

    auto ref_out = at::native::upsample_linear1d_cpu(t_input, {osize, }, align_corners);
    auto out = ti_upsample_linear1d_kernel_impl(t_input, {osize, }, align_corners);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
        assert(false);
    }
}


void assert_consistency_trilinear3d(at::Tensor t_input, int isize, int osize, bool align_corners=false) {

    if (!t_input.defined()) {
        assert(isize > 0);
        t_input = at::rand({1, 3, 16, isize, isize}, at::CPU(at::kFloat));
    }

    int osize_2 = (float(t_input.size(3)) / osize > 1.0) ? t_input.size(2) / 2 : t_input.size(2) * 2;
    auto ref_out = at::native::upsample_trilinear3d_cpu(t_input, {osize_2, osize, osize}, align_corners);
    auto out = ti_upsample_trilinear3d_kernel_impl(t_input, {osize_2, osize, osize}, align_corners);

    if (!ref_out.allclose(out)){
        auto mse = (ref_out - out).pow(2.0).mean();
        auto max_e = (ref_out - out).view(-1).abs().max();
        std::cout << "Error: mse=" << mse << ", max e=" << max_e << std::endl;
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
    assert_consistency_bilinear2d(t_input, -1, up_osize);
    assert_consistency_bilinear2d(t_input, -1, up_osize, true);

    // Time benchmark
    {
        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {dn_osize, dn_osize}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - downsampling to " << dn_osize << "x" << dn_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {dn_osize, dn_osize}, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        std::cout << "\n- Bench upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {up_osize, up_osize}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_bilinear2d_cpu (" << n << " rounds) - upsampling to " << up_osize << "x" << up_osize << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {up_osize, up_osize}, false);
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
        std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << t_input.is_contiguous(at::MemoryFormat::ChannelsLast) << std::endl;
        std::cout << "Input is_contiguous : " << t_input.is_contiguous() << std::endl;

        assert_consistency_bilinear2d(t_input, -1, 128);
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
                auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {128, 128}, false);
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
        std::cout << "\nInput tensor: " << t_input.sizes() << std::endl;
        std::cout << "Input is_contiguous memory_format torch.channels_last: " << t_input.is_contiguous(at::MemoryFormat::ChannelsLast) << std::endl;
        std::cout << "Input is_contiguous : " << t_input.is_contiguous() << std::endl;

        assert_consistency_bilinear2d(t_input, -1, 128);
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
                auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {128, 128}, false);
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
    assert_consistency_linear1d(t_input, -1, 512);
    assert_consistency_linear1d(t_input, -1, 512, true);

    // Time benchmark
    {
        std::cout << "\n- Bench upsample_linear1d_cpu (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_linear1d_cpu(t_input, {256, }, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_linear1d_cpu (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_linear1d_kernel_impl(t_input, {256, }, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        std::cout << "\n- Bench upsample_linear1d_cpu (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_linear1d_cpu(t_input, {512, }, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_linear1d_cpu (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_linear1d_kernel_impl(t_input, {512, }, false);
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
    assert_consistency_trilinear3d(t_input, -1, 512);
    assert_consistency_trilinear3d(t_input, -1, 512, true);

    // Time benchmark
    {
        std::cout << "\n- Bench upsample_trilinear3d_cpu (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_trilinear3d_cpu(t_input, {8, 256, 256}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_trilinear3d_kernel_impl (" << n << " rounds) - downsampling to 256" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_trilinear3d_kernel_impl(t_input, {8, 256, 256 }, false);
            auto result = out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    if (!full_bench)
        return 1;

    {
        std::cout << "\n- Bench upsample_trilinear3d_cpu (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto ref_out = at::native::upsample_trilinear3d_cpu(t_input, {32, 512, 512}, false);
            auto result = ref_out.size(0);
        }
        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << "Elapsed time (ms): " << elapsed_seconds.count() / n * 1000 << std::endl;
    }

    {
        std::cout << "\n- Bench ti_upsample_trilinear3d_kernel_impl (" << n << " rounds) - upsampling to 512" << std::endl;
        auto start = std::chrono::steady_clock::now();
        for (int i=0; i<n; i++)
        {
            auto out = ti_upsample_trilinear3d_kernel_impl(t_input, {32, 512, 512}, false);
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