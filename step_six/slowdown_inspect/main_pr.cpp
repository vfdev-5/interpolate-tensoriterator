// STD
#include <iostream>

// Torch
#include <ATen/ATen.h>
#include <ATen/Parallel.h>

#define NUM_THREADS 6

using namespace at;
using namespace at::native;
using namespace at::indexing;


int main(int argc, char** argv) {

    set_num_threads(NUM_THREADS);

    auto t_input = at::rand({1, 3, 16, 320, 320}, at::CPU(at::kFloat));
    int64_t osizes[3] = {8, 256, 256};
    IntArrayRef output_size(osizes);

    auto out = upsample_trilinear3d_cpu(t_input, output_size, false);
    int result = out.size(0);
    return result;
}