# Prototype Torch Interpolate with TensorIterator

FMassa's code : https://github.com/fmassa/vision-1/commit/407e0430e14ca688b2fb6f03ec1122ba46527553

## TODO

### Goals

- 2D downsampling with TensorIterator
- 2D upsampling with TensorIterator
- Benchmark implementations vs original pytorch ones
- Improve previous algorithms

### Step 1

- [x] Create an 2D implementation
- [x] Check correctness vs torch interpolate in float32
- [x] Benchmark

### Step 2

- [x] Create an 2D implementation
- [x] Check correctness vs torch interpolate in float32
- [x] Benchmark
- [x] Benchmarking on other test sizes
    - [x] Compare with https://github.com/mingfeima/op_bench-py

### Step 3

- [x] Refactor implementation like in Advanced Indexing codebase
- [x] Apply vectorization


### Step 2.1

- [x] Reproduce fmassa's results

### Step 4: 1d, 2d, 3d

- [x] Clean up the code
- [x] Generalize Step 2.1 code to 1d, 2d and 3d cases
- [x] Verify if using `int64_t` brings a slowdown, if not, just use it instead of int32_t
  - Checking https://github.com/pytorch/pytorch/pull/41923 and potentially using `canUse32BitIndexMath` if we decide to keep int32 as well
- [x] Add a documentation in the code
  - put all hypothesis on strides and indices
  - how we access pointer while iteration a single dimension
  - strides are already added to indices
  - Mention that output is constructed inside the function and contiguous
- [x] Change ti_compute_indices_weights_faster to output one or two TensorLists (to define depending on generalization)
- [x] Merge reshape (ti_reshape_indices_weights) into ti_compute_indices_weights_faster by passing new arg : dim
- [ ] ~~Optional: revert order of added indices and weights -> x, y, z, ...~~

### Step 5: Nd simplified

- [x] Fix issue #3
- [x] Fix issue #4 -> inspect the sources of precision error -> fixed by using compute_source_index_and_lambda/area_pixel_compute_scale
- [x] Check if we can have 32-bit indices overflow with large input in `ti_compute_indices_weights_linear`
- [x] Reuse compute_source_index_and_lambda and area_pixel_compute_scale
- [x] Dispatch indices_weights creation with AT_DISPATCH_FLOATING_TYPES
- [ ] Check index overflow with large output and upsampling
- [x] Fix code with hard-coded float weights

### Ideas for the future

- Add a structure to store (idx_ptrs, weights_ptrs, src_offset) to refactor the code and simplify things:
  - interp<...>(..., structure); structure.next()
- Make the code generic about nearest/linear/cubic interpolation by templating the mode by a number: e.g. 1, 2, 3

### Step 6: Back to basics (#5 by Francisco)

- [x] Test the code with older compiler like gcc 5.4
- [x] Inspect assembly code
- [ ] Specialization tricks: https://github.com/pytorch/pytorch/blob/9cec8ae146c0b95b0f5dcd1c62ea4e83ee32f90c/aten/src/ATen/native/cpu/Loops.h#L387
- [x] Explore C10_RESTRICT : https://en.wikipedia.org/wiki/Restrict
  -  https://wiki.sei.cmu.edu/confluence/display/c/EXP43-C.+Avoid+undefined+behavior+when+using+restrict-qualified+pointers


## Questions

- ~~Force parallelization on output size only (do not take into account input shape)~~
- ~~Usage of a buffer to accelerate the processing~~

## Development

<details>

<summary>
Click here for details
</summary>


```bash
docker run --rm -it \
    --name=tv-interpolate \
    -v $PWD:/interpolate-tensoriterator \
    -v $PWD/../:/workspace \
    -w /interpolate-tensoriterator \
    -v /home/user/Documents/ml/pytorch/:/pytorch \
    --network=host --security-opt seccomp:unconfined --privileged --shm-size 16G \
    nvidia/cuda:11.1-cudnn8-devel-ubuntu20.04 \
    /bin/bash
```
```
apt-get update && ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime && \
    apt-get install -y tzdata && \
    dpkg-reconfigure --frontend noninteractive tzdata && \
    apt-get install -y git cmake python3 python3-pip numactl && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    ln -s /usr/bin/pip3 /usr/bin/pip && \
    pip install numpy typing_extensions
```

</details>

### Step 1

<details>

<summary>
Click here for details
</summary>


```bash
cd step_one && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench

Input tensor: [4, 3, 320, 320]

- Check consistency (downsampling): OK

- Check consistency (upsampling): OK

- Bench upsample_bilinear2d_cpu (5000 rounds) - downsampling
Elapsed time: 0.000647936

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - downsampling
Elapsed time: 0.000641677

- Bench upsample_bilinear2d_cpu (5000 rounds) - upsampling
Elapsed time: 0.00267952

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - upsampling
Elapsed time: 0.00224466
```

</details>

### Step 2

<details>

<summary>
Click here for details
</summary>


```bash
cd step_two && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```


```bash
make && ./bench
```

#### Result 1:
```
Input tensor: [4, 3, 320, 320]

- Check consistency (downsampling): OK

- Check consistency (upsampling): OK

- Bench upsample_bilinear2d_cpu (5000 rounds) - downsampling
Elapsed time: 0.000644288

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - downsampling
Elapsed time: 0.000619151

- Bench upsample_bilinear2d_cpu (5000 rounds) - upsampling
Elapsed time: 0.00268625

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - upsampling
Elapsed time: 0.00227043
```


#### Result 2:
```
Input tensor: [1, 3, 320, 320]

- Check consistency (downsampling): OK

- Check consistency (upsampling): OK

- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling
Elapsed time: 0.000320938

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling
Elapsed time: 0.000202762

- Bench upsample_bilinear2d_cpu (7500 rounds) - upsampling
Elapsed time: 0.00129764

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - upsampling
Elapsed time: 0.000595403
```


#### Result 3:
```
Input tensor: [1, 3, 320, 320]
Num threads: 1

- Check consistency (downsampling): OK

- Check consistency (upsampling): OK

- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling
Elapsed time: 0.000872627

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling
Elapsed time: 0.000789011

- Bench upsample_bilinear2d_cpu (7500 rounds) - upsampling
Elapsed time: 0.00356224

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - upsampling
Elapsed time: 0.00291151
```


#### Result 4:
```
Input tensor: [1, 3, 320, 320]
Num threads: 6

- Check consistency (downsampling to 256x256): OK

- Check consistency (upsampling to 512x512): OK

- Bench upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time: 0.000331258

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time: 0.000191895

- Bench upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time: 0.001274

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time: 0.000576962

1 - Benchmark test size as in https://github.com/mingfeima/op_bench-py
Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: 1
Input is_contiguous : 0

- Bench upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time: 0.0374804

- Bench ti_upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time: 0.101596

2 - Benchmark test size as in https://github.com/mingfeima/op_bench-py
Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: 0
Input is_contiguous : 1

- Bench upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time: 0.088325

- Bench ti_upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time: 0.0788824
```

</details>

### Step 3

**Important:** to activate AVX we need to add the following to CMakeLists.txt:
```
set(CMAKE_CXX_FLAGS "${CMAKE_CUDA_FLAGS} -O3 -mavx")
add_definitions("-DCPU_CAPABILITY_AVX")
```

  - Rewritten `ti_compute_indices_weights` as a single loop
  - Skip multiplications on zero stride (e.g. src, y-indices and y-weights (under certain conditions))
  - Integrate index strides directly into tensors
  - Replaced manual buffer expansion with for-loop
  - Casted once indices to int64_t
  - Casted once weights to float
  - Simplified setup_vec_src

```bash
cd step_three && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```

<details>

<summary>
Results
</summary>


#### Result 1

```
Input tensor: [1, 3, 320, 320]
Num threads: 6

- Check consistency (downsampling to 256x256): OK

- Check consistency (upsampling to 512x512): OK

- Bench upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time: 0.00032115

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time: 0.000191068

- Bench upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time: 0.00128171

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time: 0.00065351

1 - Benchmark test size as in https://github.com/mingfeima/op_bench-py
Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: 1
Input is_contiguous : 0

- Bench upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time: 0.0309762

- Bench ti_upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Segmentation fault (core dumped)
```

#### Result 2
```
Input tensor: [1, 3, 320, 320]
Num threads: 6

- Check consistency (downsampling to 256x256): OK

- Check consistency (upsampling to 512x512): OK

- Bench upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time: 0.000320324

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time: 0.000144485

- Bench upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time: 0.00128517

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time: 0.000500686

1 - Benchmark test size as in https://github.com/mingfeima/op_bench-py
Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: 1
Input is_contiguous : 0

- Bench upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time: 0.0303925

- Bench ti_upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Segmentation fault (core dumped)
```

#### Result 3

```
Input tensor: [1, 3, 320, 320]
Num threads: 6

- Check consistency (downsampling to 256x256): OK

- Check consistency (upsampling to 512x512): OK

- Bench upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time (ms): 0.317664

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time (ms): 0.080967

- Bench upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time (ms): 1.27172

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time (ms): 0.25715

1 - Benchmark test size as in https://github.com/mingfeima/op_bench-py
Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: 1
Input is_contiguous : 0

- Bench upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time (ms): 30.2949

- Bench ti_upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Segmentation fault (core dumped)
```

</details>

### Step 2.1

```bash
cd step_two_dot_one && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```

<details>

<summary>
Results
</summary>


#### Result 1:

```
Input tensor: [1, 3, 320, 320]
Num threads: 6

- Check consistency (downsampling to 256x256): OK

- Check consistency (upsampling to 512x512): OK

- Bench upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time (ms): 0.322771

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - downsampling to 256x256
Elapsed time (ms): 0.0721024

- Bench upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time (ms): 1.26993

- Bench ti_upsample_bilinear2d_cpu (5000 rounds) - upsampling to 512x512
Elapsed time (ms): 0.199163

1 - Benchmark test size as in https://github.com/mingfeima/op_bench-py
Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: 1
Input is_contiguous : 0

- Check consistency (upsampling to 128x128): OK

- Bench upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time (ms): 35.68

- Bench ti_upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time (ms): 61.7569

2 - Benchmark test size as in https://github.com/mingfeima/op_bench-py
Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: 0
Input is_contiguous : 1

- Check consistency (upsampling to 128x128): OK

- Bench upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time (ms): 88.6877

- Bench ti_upsample_bilinear2d_cpu (500 rounds) - upsampling to 128x128
Elapsed time (ms): 48.9633
```

</details>


### Step 4: 1d, 2d, 3d

```bash
cd step_four_1d_2d_3d && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```

<details>

<summary>
Results
</summary>


#### Result 1:

```
Torch config: PyTorch built with:
  - GCC 9.3
  - C++ Version: 201402
  - OpenMP 201511 (a.k.a. OpenMP 4.5)
  - CPU capability usage: AVX2
  - Build settings: BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-h
idden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-l
imits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-ty
pedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -
Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=for
mat -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.8.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_
FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,

                                                                    
                            
---- Benchmark 2D ----
Input tensor: [1, 3, 320, 320]
Num threads: 6        
                                  
- Check consistency (downsampling to 256x256): OK

- Check consistency (upsampling to 512x512): OK

- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 0.316959
                                                                   
- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 0.0636201

---- Benchmark 1D ----
Input tensor: [4, 512, 320]
Num threads: 6

- Check consistency (downsampling to 256): OK

- Check consistency (upsampling to 512): OK

- Bench upsample_linear1d_cpu (7500 rounds) - downsampling to 256
Elapsed time (ms): 0.288682

- Bench ti_upsample_linear1d_cpu (7500 rounds) - downsampling to 256
Elapsed time (ms): 0.0954624


---- Benchmark 3D ----
Input tensor: [1, 3, 16, 320, 320]
Num threads: 6

- Check consistency (downsampling to 256): OK

- Check consistency (upsampling to 512): OK

- Bench upsample_trilinear3d_cpu (750 rounds) - downsampling to 256
Elapsed time (ms): 4.48405

- Bench ti_upsample_trilinear3d_kernel_impl (750 rounds) - downsampling to 256
Elapsed time (ms): 0.743611
```


#### Result 2 (single loop):

```
Torch config: PyTorch built with:                                      
  - GCC 9.3                
  - C++ Version: 201402
  - OpenMP 201511 (a.k.a. OpenMP 4.5)                                     
  - CPU capability usage: AVX2
  - Build settings: BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPO
OL -fopenmp -DNDEBUG -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragm
as -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=dep
recated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-se
t-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX51
2=1, TORCH_VERSION=1.8.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNP
ACK=0, USE_OPENMP=ON,



---- Benchmark 2D ----
                      
Input tensor: [1, 3, 320, 320]
Num threads: 6

- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 0.325516

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 0.0668158

Input tensor: [1, 3, 1024, 1024]
Num threads: 6

- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling to 512x512
Elapsed time (ms): 1.29353

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling to 512x512
Elapsed time (ms): 0.232354


---- Benchmark 1D ----

Input tensor: [4, 512, 320]
Num threads: 6

- Bench upsample_linear1d_cpu (7500 rounds) - downsampling to 256
Elapsed time (ms): 0.289353

- Bench ti_upsample_linear1d_cpu (7500 rounds) - downsampling to 256
Elapsed time (ms): 0.109785


---- Benchmark 3D ----

Input tensor: [1, 3, 16, 320, 320]
Num threads: 6

- Check consistency (upsampling to 512): 
- Bench upsample_trilinear3d_cpu (750 rounds) - downsampling to 256
Elapsed time (ms): 4.49102

- Bench ti_upsample_trilinear3d_kernel_impl (750 rounds) - downsampling to 256
Elapsed time (ms): 0.978064
```

</details>


#### Benchmark with OpenCV

Install OpenCV libs
```
apt-get install -y libopencv-core-dev libopencv-imgproc-dev 

ls /usr/include/opencv4/opencv2/core/
```

Build with OpenCV
```bash
cd step_four_1d_2d_3d && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH -DOPENCV_INCLUDE_DIR=/usr/include/opencv4/ -DOPENCV_LIB_DIR=/usr/lib/x86_64-linux-gnu/ ..
make
```

```bash
make && ./bench
```


### Step 5

```bash
cd step_five && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```

<details>

<summary>
Results
</summary>


#### Result:

```
Torch config: PyTorch built with:
  - GCC 9.3                       
  - C++ Version: 201402
  - OpenMP 201511 (a.k.a. OpenMP 4.5)
  - CPU capability usage: AVX2                                               
  - Build settings: BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fope
nmp -DNDEBUG -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-com
pare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wn
o-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitiali
zed -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.8.0, USE_CUDA=1, US
E_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,


---- Benchmark 2D ----    

Input tensor: [1, 3, 320, 320]                                             
Num threads: 6             

- Bench upsample_bilinear2d_cpu (10000 rounds) - downsampling to 256x256
Elapsed time (ms): 0.319714

- Bench ti_upsample_bilinear2d_cpu (10000 rounds) - downsampling to 256x256
Elapsed time (ms): 0.0637279

Input tensor: [1, 3, 1024, 1024]                                  
Num threads: 6             

- Bench upsample_bilinear2d_cpu (10000 rounds) - downsampling to 512x512
Elapsed time (ms): 1.28923 

- Bench ti_upsample_bilinear2d_cpu (10000 rounds) - downsampling to 512x512
Elapsed time (ms): 0.231808


---- Benchmark 1D ----

Input tensor: [4, 512, 320]
Num threads: 6

- Bench upsample_linear1d_cpu (10000 rounds) - downsampling to 256
Elapsed time (ms): 0.289338

- Bench ti_upsample_linear1d_cpu (10000 rounds) - downsampling to 256
Elapsed time (ms): 0.108713


---- Benchmark 3D ----

Input tensor: [1, 3, 16, 320, 320]
Num threads: 6

- Bench upsample_trilinear3d_cpu (1000 rounds) - downsampling to [8, 256, 256]
Elapsed time (ms): 4.70384

- Bench ti_upsample_trilinear3d_kernel_impl (1000 rounds) - downsampling to [8, 256, 256]
Elapsed time (ms): 1.10881
```

</details>



### Step 6


```bash
cd step_six && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench 20000
```

#### With GCC 5.4

- Use libtorch

```bash
apt-get update && apt-get install -y wget p7zip-full
cd /tmp
wget https://download.pytorch.org/libtorch/nightly/cpu/libtorch-cxx11-abi-shared-with-deps-latest.zip
7z x libtorch-cxx11-abi-shared-with-deps-latest.zip
```

- Install GCC 5.4

```bash
echo "deb http://archive.ubuntu.com/ubuntu/ xenial main universe\n" >> /etc/apt/sources.list
apt-get update

apt-get install -y g++-5 gcc-5
```

- Build

```bash
cd step_six && mkdir -p build_gcc_54 && cd $_
export CC=/usr/bin/gcc-5
export CXX=/usr/bin/g++-5
export TORCH_PATH=/tmp/libtorch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```


#### Assembly code generation

```bash
g++ -O3 -mavx -mfma -mavx2 -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store -S -fverbose-asm -fopt-info-vec-missed -fopt-info-vec -g ../main.cpp -I.. -I${TORCH_PATH}/include -I${TORCH_PATH}/include/torch/csrc/api/include -L${TORCH_PATH}/lib/ -ltorch -ltorch_cpu -lc10 -o main.s

as -alhnd main.s > main.lst
```



## Upsampling code inspection

<details>

<summary>
Click here for details
</summary>

```
Native functions: upsample_bilinear2d, pytorch/aten/src/ATen/native/native_functions.yaml:8889
|
\-> dispatch to upsample_bilinear2d_cpu, pytorch/aten/src/ATen/native/UpSampleBilinear2d.cpp:106
|
\-> upsample_bilinear2d_out_cpu_template, pytorch/aten/src/ATen/native/UpSampleBilinear2d.cpp:113
|
\-> upsample_bilinear2d_kernel, pytorch/aten/src/ATen/native/UpSampleBilinear2d.cpp:48
|   DEFINE_DISPATCH(upsample_bilinear2d_kernel) , DECLARE_DISPATCH(upsampling_bilinear2d, upsample_bilinear2d_kernel)
|
\-> REGISTER_DISPATCH(upsample_bilinear2d_kernel, &upsample_bilinear2d_kernel_impl), 
|   pytorch/aten/src/ATen/native/cpu/UpSampleMoreKernel.cpp:569
|
\-> upsample_bilinear2d_kernel_impl
|   AT_DISPATCH_FLOATING_TYPES(input.scalar_type(), "upsample_bilinear2d", [&] {
|      cpu_upsample_linear<scalar_t, scale_t>(output, input, align_corners, {scales_h, scales_w});
|   })
|   pytorch/aten/src/ATen/native/cpu/UpSampleMoreKernel.cpp:510
|
\-> cpu_upsample_linear, pytorch/aten/src/ATen/native/cpu/UpSampleMoreKernel.cpp:41
|
\-> at::parallel_for(0, channels, at::internal::GRAIN_SIZE / output_slice_size / 4, loop2d);
```

</details>

## Repro op_bench-py

<details>

<summary>
Click here for details
</summary>

```
docker run --rm -it \
    --name=op_bench-py \
    -v $PWD:/op_bench-py \
    -w /op_bench-py \
    --network=host --security-opt seccomp:unconfined --privileged --shm-size 16G \
    pytorch/pytorch:1.7.0-cuda11.0-cudnn8-devel \
    /bin/bash
```

Setup
```
apt-get install -y numactl
```
Run
```
cd /workspace/op_bench-py/
PYTHONPATH=/pytorch:$PYTHONPATH ./run.sh upsample_linear.py
```


### Result on pytorch/pytorch:1.7.0-cuda11.0-cudnn8-devel

```
./run.sh upsample_linear.py

### using KMP_AFFINITY=granularity=fine,compact,1,0
### using KMP_BLOCKTIME=1

### using OMP_NUM_THREADS=6
### using numactl --physcpubind=0-5 --membind=0
/opt/conda/lib/python3.8/site-packages/torch/nn/functional.py:3060: UserWarning: Default upsampling behavior when mode=bilinear is changed to align_corners=False sinc
e 0.4.0. Please specify align_corners=True if the old behavior is desired. See the documentation of nn.Upsample for details.
  warnings.warn("Default upsampling behavior when mode={} is changed "

upsample_bilinear: memory format: nchw, input size:  [32, 128, 64, 64]
input.is_contiguous(memory_format=torch.channels_last):  False
input.is_contiguous():  True
forward time per iteration: 87.659 ms
upsample_bilinear_cl: memory format: nhwc, input size:  [32, 64, 64, 128]
input.is_contiguous(memory_format=torch.channels_last):  True
input.is_contiguous():  False
forward time per iteration: 34.082 ms

### using OMP_NUM_THREADS=1
### using numactl --physcpubind=0 --membind=0

/opt/conda/lib/python3.8/site-packages/torch/nn/functional.py:3060: UserWarning: Default upsampling behavior when mode=bilinear is changed to align_corners=False sinc
e 0.4.0. Please specify align_corners=True if the old behavior is desired. See the documentation of nn.Upsample for details.
  warnings.warn("Default upsampling behavior when mode={} is changed "

upsample_bilinear: memory format: nchw, input size:  [32, 128, 64, 64]
input.is_contiguous(memory_format=torch.channels_last):  False
input.is_contiguous():  True
forward time per iteration: 413.920 ms
upsample_bilinear_cl: memory format: nhwc, input size:  [32, 64, 64, 128]
input.is_contiguous(memory_format=torch.channels_last):  True
input.is_contiguous():  False
forward time per iteration: 106.310 ms
```

### Result on master (1a92802bde28286f850b415bfd985515565740c3)
```
__version__ = '1.8.0a0+1a92802'
debug = False
cuda = '11.1'
git_version = '1a92802bde28286f850b415bfd985515565740c3'
hip = None

PyTorch built with:
  - GCC 9.3
  - C++ Version: 201402
  - OpenMP 201511 (a.k.a. OpenMP 4.5)
  - CPU capability usage: AVX2
  - CUDA Runtime 11.1
  - NVCC architecture flags: -gencode;arch=compute_61,code=sm_61
  - CuDNN 8.0.5
  - Build settings: BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.8.0, USE_CUDA=1, USE_CUDNN=ON, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON, 
```

```
PYTHONPATH=/pytorch:$PYTHONPATH ./run.sh upsample_linear.py

### using KMP_AFFINITY=granularity=fine,compact,1,0
### using KMP_BLOCKTIME=1


### using OMP_NUM_THREADS=6
### using numactl --physcpubind=0-5 --membind=0

/pytorch/torch/nn/functional.py:3032: UserWarning: Default upsampling behavior when mode=bilinear is changed to align_corners=False since 0.4.0. Please specify align_corners=True if the old behavior is desired. See the documentation of nn.Upsample for details.  warnings.warn("Default upsampling behavior when mode={} is changed "
upsample_bilinear: memory format: nchw, input size:  [32, 128, 64, 64]
input.is_contiguous(memory_format=torch.channels_last):  False
input.is_contiguous():  True
forward time per iteration: 86.019 ms
upsample_bilinear_cl: memory format: nhwc, input size:  [32, 64, 64, 128]
input.is_contiguous(memory_format=torch.channels_last):  True
input.is_contiguous():  False
forward time per iteration: 34.567 ms

### using OMP_NUM_THREADS=1
### using numactl --physcpubind=0 --membind=0

/pytorch/torch/nn/functional.py:3032: UserWarning: Default upsampling behavior when mode=bilinear is changed to align_corners=False since 0.4.0. Please specify align_corners=True if the old behavior is desired. See the documentation of nn.Upsample for details.
  warnings.warn("Default upsampling behavior when mode={} is changed "
upsample_bilinear: memory format: nchw, input size:  [32, 128, 64, 64]
input.is_contiguous(memory_format=torch.channels_last):  False
input.is_contiguous():  True
forward time per iteration: 406.644 ms
upsample_bilinear_cl: memory format: nhwc, input size:  [32, 64, 64, 128]
input.is_contiguous(memory_format=torch.channels_last):  True
input.is_contiguous():  False
forward time per iteration: 108.430 ms
```

</details>


## OpenCV Resize function inspection

<details>

<summary>
Click here for details
</summary>


```
// https://github.com/opencv/opencv/blob/7d7ab462d6bcf39e453b47e95641ede41d3ef8bd/modules/imgproc/src/resize.cpp#L4044
|-- void cv::resize( InputArray _src, OutputArray _dst, Size dsize,
|                    double inv_scale_x, double inv_scale_y, int interpolation )
|                 
|
// https://github.com/opencv/opencv/blob/7d7ab462d6bcf39e453b47e95641ede41d3ef8bd/modules/imgproc/src/resize.cpp#L3669 
|-- hal::resize(...)
|
|
|--> CALL_HAL(resize, cv_hal_resize, ...) 
|--> CV_IPP_RUN_FAST(ipp_resize(...)
|
|
|
|
|
|

```

</details>


## [ASAN](https://github.com/google/sanitizers/wiki/AddressSanitizer)

```bash
docker run --rm -it \
    --name=tv-interpolate-asan \
    -v $PWD:/interpolate-tensoriterator \
    -v $PWD/../:/workspace \
    -w /interpolate-tensoriterator \
    -v /home/user/Documents/ml/pytorch/:/pytorch \
    --network=host --security-opt seccomp:unconfined --privileged --shm-size 16G \
    nvidia/cuda:11.1-cudnn8-devel-ubuntu20.04 \
    /bin/bash
```
```bash
apt-get update && ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime && \
    apt-get install -y tzdata && \
    dpkg-reconfigure --frontend noninteractive tzdata && \
    apt-get install -y git cmake python3 python3-pip numactl && \
    ln -s /usr/bin/python3 /usr/bin/python && \
    ln -s /usr/bin/pip3 /usr/bin/pip && \
    pip install numpy typing_extensions
```

### Step 5

```bash
cd step_five && mkdir -p asan_build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH -DWITH_ASAN=yes ..
make && ./bench
```

### Step 6

```bash
cd step_six && mkdir -p asan_build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH -DWITH_ASAN=yes ..
make && ./bench
```

## Install linux perf

On Ubuntu 20.04 docker image, linux kernel is still 4.15, but perf and other tools are for the kernel 5.4.

```bash
echo "deb http://archive.ubuntu.com/ubuntu/ bionic main universe\n" >> /etc/apt/sources.list
apt-get update
apt-get install -y linux-tools-4.15.0-20-generic linux-tools-4.15.0-20 linux-tools-4.15.0-20-lowlatency 

rm -rf /usr/bin/perf
ln -s /usr/lib/linux-tools-4.15.0-20/perf /usr/bin/perf
```

## References:

- https://github.com/opencv/opencv/blob/7d7ab462d6bcf39e453b47e95641ede41d3ef8bd/modules/imgproc/src/resize.cpp#L457

- https://github.com/pytorch/pytorch/blob/master/aten/src/ATen/native/cpu/GridSamplerKernel.cpp
- https://github.com/pytorch/pytorch/blob/ec8e9d31cf3a9c2bc5f31d500389b48a5024917d/aten/src/ATen/native/TensorAdvancedIndexing.cpp#L238-L256
- https://github.com/pytorch/pytorch/pull/34864

- https://dirtyhandscoding.wordpress.com/2017/08/02/vectorizing-stdmerge-with-vpermd-from-avx2-and-lookup-table/

- https://chryswoods.com/vector_c++/

- http://hpac.cs.umu.se/teaching/sem-accg-16/slides/08.Schmitz-GGC_Autovec.pdf

- https://indico.cern.ch/event/771113/contributions/3203712/attachments/1746730/3022094/PracticalVectorization.pres.pdf

- https://software.intel.com/sites/default/files/m/4/8/8/2/a/31848-CompilerAutovectorizationGuide.pdf