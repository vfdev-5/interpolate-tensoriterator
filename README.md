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

- [ ] Refactor implementation like in Advanced Indexing codebase
- [ ] Apply vectorization

## Questions

- ~~Force parallelization on output size only (do not take into accound input shape)~~
- Usage of a buffer to accelerate the processing

## Development

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

### Step 1

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


### Step 2

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



## Upsampling code inspection

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

## Repro op_bench-py

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


## OpenCV Resize function inspection

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


## References:

- https://github.com/opencv/opencv/blob/7d7ab462d6bcf39e453b47e95641ede41d3ef8bd/modules/imgproc/src/resize.cpp#L457

- https://github.com/pytorch/pytorch/blob/master/aten/src/ATen/native/cpu/GridSamplerKernel.cpp
- https://github.com/pytorch/pytorch/blob/ec8e9d31cf3a9c2bc5f31d500389b48a5024917d/aten/src/ATen/native/TensorAdvancedIndexing.cpp#L238-L256
- https://github.com/pytorch/pytorch/pull/34864

- https://dirtyhandscoding.wordpress.com/2017/08/02/vectorizing-stdmerge-with-vpermd-from-avx2-and-lookup-table/

- https://chryswoods.com/vector_c++/
