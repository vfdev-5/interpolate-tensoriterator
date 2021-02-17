// STD
#include <iostream>
#include <chrono>

// Torch
#include <ATen/ATen.h>

#include <omp.h>

// Local
#include "interpolate.h"


int main(int argc, char** argv)
{
    auto t_input = at::rand({1, 3, 320, 320}, at::CPU(at::kFloat));
    std::cout << "Input tensor: "<< t_input.sizes() << std::endl;
    std::cout << "Num threads: " << at::get_num_threads() << std::endl;

    {
        std::cout << "\nRun : " << std::endl;
        auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {256, 256});
    }

    // {
    //     std::cout << "\n- Check consistency (upsampling): ";
    //     auto ref_out = at::native::upsample_bilinear2d_cpu(t_input, {512, 512}, false);
    //     auto out = ti_upsample_bilinear2d_kernel_impl(t_input, {512, 512});
    //     if (!ref_out.allclose(out)){
    //         std::cout << "Error" << std::endl;
    //         return 1;
    //     }
    //     std::cout << "OK" << std::endl;
    // }
    return 0;
}