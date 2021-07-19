# Prototype Torch Interpolate with TensorIterator

FMassa's code : https://github.com/fmassa/vision-1/commit/407e0430e14ca688b2fb6f03ec1122ba46527553

## Goals

- ND downsampling/upsampling with TensorIterator, mode: linear, nearest, cubic
- Benchmark implementations vs original pytorch ones
- Improve previous algorithms

### Step 6: Back to basics (#5 by Francisco)

- [x] Test the code with older compiler like gcc 5.4
- [x] Inspect assembly code
- [ ] Specialization tricks: https://github.com/pytorch/pytorch/blob/9cec8ae146c0b95b0f5dcd1c62ea4e83ee32f90c/aten/src/ATen/native/cpu/Loops.h#L387
- [x] Explore C10_RESTRICT : https://en.wikipedia.org/wiki/Restrict
  -  https://wiki.sei.cmu.edu/confluence/display/c/EXP43-C.+Avoid+undefined+behavior+when+using+restrict-qualified+pointers

### Step 7: Generic implementation

Following [results 16/03/2021](step_seven/results/custom_pr_1.9.0a0+git2c06596_vs_pth_1.9.0a0+gite8e570e_results.1.md)
- [ ] Improve case: `upsample_nearest2d channels_first contiguous [32, 128, 64, 64] -> (128, 128)`
  - 6 threads `[32, 128, 64, 64] -> (128, 128)  |        50420.0       |        53869.1`
  - 1 thread  `[32, 128, 64, 64] -> (128, 128)  |       195835.9       |       219061.2`
- [ ] Improve case: `upsample_trilinear3d channels_first contiguous`:
  - 1 thread  `[1, 3, 16, 320, 320] -> [8, 256, 256]   |          5.4         |         11.5`
  - 1 thread  `[1, 3, 16, 320, 320] -> [32, 512, 512]  |        114.5         |        210.6`
  - 6 threads `[1, 3, 16, 320, 320] -> [8, 256, 256]   |          1.0         |          2.1`
  - 6 threads `[1, 3, 16, 320, 320] -> [32, 512, 512]  |         25.6         |         43.7`

## Development

<details>

<summary>
Click here for details
</summary>


```bash
docker run -it \
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
    pip install numpy typing_extensions
```

</details>

See [playground](playground) for details step by step.

### Step 6 - Linear interpolation

- Install GCC 7.3 as PyTorch Nightly

```bash
echo "deb http://archive.ubuntu.com/ubuntu/ bionic main\n" >> /etc/apt/sources.list
echo "deb http://archive.ubuntu.com/ubuntu/ bionic universe\n" >> /etc/apt/sources.list
apt-get update
apt-get install g++-7=7.3.0-16ubuntu3 gcc-7-base=7.3.0-16ubuntu3 gcc-7=7.3.0-16ubuntu3 cpp-7=7.3.0-16ubuntu3 libgcc-7-dev=7.3.0-16ubuntu3 libstdc++-7-dev=7.3.0-16ubuntu3 libasan4=7.3.0-16ubuntu3 libubsan0=7.3.0-16ubuntu3 libcilkrts5=7.3.0-16ubuntu3
```

- Build

```bash
cd step_six && mkdir -p build && cd $_
export CC=/usr/bin/gcc-7
export CXX=/usr/bin/g++-7
export TORCH_PATH=/tmp/libtorch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench 20000
```

#### How to produce benchmarks:

Configure PR_TORCH_PATH for PR PyTorch build inside `run_python_pr_bench.sh`:
```
export PR_TORCH_PATH=/workspace/pth-linear-interp/
```

and run:
```
sh run_python_pr_bench.sh
> pr_vs_pth_results.md
```



### Step 7 - Cubic/Nearest/Linear interpolations


#### Cubic interpolation

```bash
cd step_seven/cubic && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```



<details>

<summary>

Cubic 2d prelimiary results

</summary>

```
Torch config: PyTorch built with:
  - GCC 9.3
  - C++ Version: 201402
  - OpenMP 201511 (a.k.a. OpenMP 4.5)
  - CPU capability usage: AVX2
  - Build settings: BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS= -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopen
mp -DNDEBUG -DUSE_KINETO -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas
 -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-
declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wn
o-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1
.9.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,

Num threads: 6


---- Benchmark 2D ----

Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous : true

- Bench upsample_bicubic2d (750 rounds) - downsampling to 256x256
Elapsed time (ms): 6.5751

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - downsampling to 256x256
Elapsed time (ms): 0.415758

- Bench upsample_bicubic2d (750 rounds) - upsampling to 512x512
Elapsed time (ms): 25.2327

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - upsampling to 512x512
Elapsed time (ms): 1.57621

Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous : false

- Bench upsample_bicubic2d (750 rounds) - downsampling to 256x256
Elapsed time (ms): 6.54954

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - downsampling to 256x256
Elapsed time (ms): 0.413038

- Bench upsample_bicubic2d (750 rounds) - upsampling to 512x512
Elapsed time (ms): 25.2994

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - upsampling to 512x512
Elapsed time (ms): 1.50504

Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false

- Bench upsample_bicubic2d (750 rounds) - downsampling to 256x256
Elapsed time (ms): 6.58091

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - downsampling to 256x256
Elapsed time (ms): 0.752833

- Bench upsample_bicubic2d (750 rounds) - upsampling to 512x512
Elapsed time (ms): 25.3467

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - upsampling to 512x512
Elapsed time (ms): 2.94774

1 - Test size as in https://github.com/mingfeima/op_bench-py

Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false

- Bench upsample_bicubic2d (75 rounds) - upsampling to 128x128
Elapsed time (ms): 7296.32

- Bench ti_upsample_bicubic2d_cpu (75 rounds) - upsampling to 128x128
Elapsed time (ms): 158.019

2 - Test size as in https://github.com/mingfeima/op_bench-py

Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous : true

- Bench upsample_bicubic2d (75 rounds) - upsampling to 128x128
Elapsed time (ms): 7249.08

- Bench ti_upsample_bicubic2d_cpu (75 rounds) - upsampling to 128x128
Elapsed time (ms): 158.135

Input tensor: [1, 3, 500, 500]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous : true

- Bench upsample_bicubic2d (750 rounds) - downsampling to 256x256
Elapsed time (ms): 6.51921

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - downsampling to 256x256
Elapsed time (ms): 0.414213

- Bench upsample_bicubic2d (750 rounds) - upsampling to 800x800
Elapsed time (ms): 61.1398

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - upsampling to 800x800
Elapsed time (ms): 3.62011

Input tensor: [1, 3, 500, 500]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous : false

- Bench upsample_bicubic2d (750 rounds) - downsampling to 256x256
Elapsed time (ms): 6.6466

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - downsampling to 256x256
Elapsed time (ms): 0.420774

- Bench upsample_bicubic2d (750 rounds) - upsampling to 800x800
Elapsed time (ms): 61.3422

- Bench ti_upsample_bicubic2d_cpu (750 rounds) - upsampling to 800x800
Elapsed time (ms): 3.62022

---- END Benchmark 2D ----
```

</details>


#### Result 1

[PyTorch nightly (66f07c0) vs This Prototype](step_seven/pth_vs_this_full_results.log.save)


#### Any mode / Nd implementation results

[results](step_seven/results)

#### Notes

<details>

<summary>

17/03/2021

</summary>

```
- ti_upsample_bilinear2d_cpu on channels first

Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous memory_format torch.channels_last_3d: false
Input is_contiguous : true

Output tensor: [1, 3, 256, 256]
Output is_contiguous memory_format torch.channels_last: false
Output is_contiguous memory_format torch.channels_last_3d: false
Output is_contiguous : true
TI_SHOW: N=256
TI_SHOW_STRIDES: 4 0 | 0 0 0 0 | 8 4 8 4 |
TI_BASIC_LOOP -> CHANNELS_FIRST
```
and
```
- Bench ti_upsample_nearest2d (1 rounds) - upsampling to 512x512

Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous memory_format torch.channels_last_3d: false
Input is_contiguous : true

Output tensor: [1, 3, 512, 512]
Output is_contiguous memory_format torch.channels_last: false
Output is_contiguous memory_format torch.channels_last_3d: false
Output is_contiguous : true
TI_SHOW: N=512
TI_SHOW_STRIDES: 4 0 | 0 0 | 8 4 |
TI_BASIC_LOOP -> CHANNELS_FIRST
Elapsed time (ms): 1.41033
```
and
```
- Bench ti_upsample_bicubic2d_cpu (1 rounds) - upsampling to 512x512

Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous memory_format torch.channels_last_3d: false
Input is_contiguous : true

Output tensor: [1, 3, 512, 512]
Output is_contiguous memory_format torch.channels_last: false
Output is_contiguous memory_format torch.channels_last_3d: false
Output is_contiguous : true
TI_SHOW: N=512
TI_SHOW_STRIDES: 4 0 | 0 0 0 0 0 0 0 0 | 8 4 8 4 8 4 8 4 |
TI_BASIC_LOOP -> CHANNELS_FIRST
Elapsed time (ms): 10.8974
```

</details>


<details>

<summary>

18/03/2021 - loop1d vs loop2d

</summary>

```
# LOOP1D
Num threads: 1
Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false

- Bench ti_upsample_bilinear2d (1000 rounds) - downsampling to 256x256
Elapsed time (ms): 1.28927

- Bench upsample_bilinear2d (1000 rounds) - downsampling to 256x256
Elapsed time (ms): 1.01537

- Bench ti_upsample_bilinear2d (1000 rounds) - upsampling to 512x512
Elapsed time (ms): 5.06349

- Bench upsample_bilinear2d (1000 rounds) - upsampling to 512x512
Elapsed time (ms): 4.03706
```
vs
```
# LOOP2D

Num threads: 1

Input tensor: [1, 3, 320, 320]
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false

- Bench ti_upsample_bilinear2d (1000 rounds) - downsampling to 256x256
Elapsed time (ms): 1.14841

- Bench upsample_bilinear2d (1000 rounds) - downsampling to 256x256
Elapsed time (ms): 1.01576

- Bench ti_upsample_bilinear2d (1000 rounds) - upsampling to 512x512
Elapsed time (ms): 4.35938

- Bench upsample_bilinear2d (1000 rounds) - upsampling to 512x512
Elapsed time (ms): 4.03187
```

```
Output tensor: [1, 3, 512, 512]
Output is_contiguous memory_format torch.channels_last: true
Output is_contiguous memory_format torch.channels_last_3d: false
Output is_contiguous : false
TI_SHOW: size0=3
TI_SHOW: size1=512
TI_SHOW_STRIDES: 4 4 | 0 0 0 0 | 0 0 0 0 |
 - strides= 4 4 0 0 0 0 0 0 0 0
 - outer_strides= 12 0 0 0 0 0 8 4 8 4
TI_BASIC_LOOP -> CHANNELS_LAST
```

```
Output tensor: [1, 3, 512, 512]
Output is_contiguous memory_format torch.channels_last: false
Output is_contiguous memory_format torch.channels_last_3d: false
Output is_contiguous : true
TI_SHOW: size0=512
TI_SHOW: size1=512
TI_SHOW_STRIDES: 4 0 | 0 0 0 0 | 8 4 8 4 |
 - strides= 4 0 0 0 0 0 8 4 8 4
 - outer_strides= 2048 0 8 4 8 4 0 0 0 0
```

```
Output tensor: [1, 3, 8, 256, 256]
Output is_contiguous memory_format torch.channels_last: false
Output is_contiguous memory_format torch.channels_last_3d: true
Output is_contiguous : false
TI_SHOW: size0=3
TI_SHOW: size1=256
TI_SHOW_STRIDES: 4 4 | 0 0 0 0 | 0 0 0 0 | 0 0 0 0 |
 - strides= 4 4 0 0 0 0 0 0 0 0 0 0 0 0
 - outer_strides= 12 0 0 0 0 0 0 0 0 0 8 4 8 4
TI_BASIC_LOOP -> CHANNELS_LAST
```

```
Output tensor: [1, 3, 8, 256, 256]
Output is_contiguous memory_format torch.channels_last: false
Output is_contiguous memory_format torch.channels_last_3d: false
Output is_contiguous : true
TI_SHOW: size0=256
TI_SHOW: size1=256
TI_SHOW_STRIDES: 4 0 | 0 0 0 0 | 0 0 0 0 | 8 4 8 4 |
 - strides= 4 0 0 0 0 0 0 0 0 0 8 4 8 4
 - outer_strides= 1024 0 0 0 0 0 8 4 8 4 0 0 0 0
TI_BASIC_LOOP -> CHANNELS_FIRST
```


</details>


### Step 8 - Backward with TI for Cubic/Nearest/Linear interpolations

/!\ Under development /!\

- an implementation with implicit 2d kernel and possible race condition with ChannelsLast mem format.

- "Separable implementation" can possibly solve race condition.

#### Linear interpolation

```bash
cd step_eight_backward/linear && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench
```

<details>

<summary>

Early results with linear backward using TensorIterator

</summary>

```
Torch config: PyTorch built with:  - GCC 9.3  - C++ Version: 201402  - OpenMP 201511 (a.k.a. OpenMP 4.5)
  - CPU capability usage: AVX2  - Build settings: BUILD_TYPE=Release, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS= -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DLIBKINETO_NOCUPTI -DUSE_PYTORCH_QNNPACK -DSYMBOLICATE_MOBILE_DEBUG_HANDLE -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type-Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.10.0, USE_CUDA=0, USE_CUDNN=OFF, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=OFF, USE_NNPACK=0, USE_OPENMP=ON,

Num threads: 1


---- Benchmark 2D ----

Grad Output tensor: [1, 3, 320, 320]
Grad Output is_contiguous memory_format torch.channels_last: false
Grad Output is_contiguous : true

- Bench ti_upsample_bilinear2d_cpu_backward (7500 rounds) - upsampling from 256x256
Elapsed time (ms): 0.781853

- Bench upsample_bilinear2d_backward (7500 rounds) - upsampling from 256x256
Elapsed time (ms): 1.71152

- Bench ti_upsample_bilinear2d_cpu_backward (7500 rounds) - downsampling from 512x512
Elapsed time (ms): 0.809508

- Bench upsample_bilinear2d_backward (7500 rounds) - downsampling from 512x512
Elapsed time (ms): 1.80017

---- END Benchmark 2D ----
```

</details>


