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

See [playground](playground) for details step by step.

### Step 6 - Linear interpolation


```bash
cd step_six && mkdir -p build && cd $_
export TORCH_PATH=/pytorch/torch
cmake -DTORCH_DIR=$TORCH_PATH ..
make
```

```bash
make && ./bench 20000
```

#### How to produce benchmarks:

Configure TORCH_PATH for PyTorch master build and PR_TORCH_PATH for PR PyTorch build inside `run_pr_bench.sh`:
```
export TORCH_PATH=/pytorch/torch
export PR_TORCH_PATH=/workspace/pth-linear-interp/torch
```

and run:
```
sh run_pr_bench.sh
python make_results_table.py output.md pth_vs_this_full_results.log.save PR_vs_this_full_results.log.save
```

#### Channels last times

2D only ()
```
App build flags: -mavx -mfma -mavx2 -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store -Wno-deprecated -fvisibility-inlines-hidden -fopenmp -DNDEBUG -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow


Torch config: PyTorch built with:
  - GCC 9.3
  - C++ Version: 201402                                                
  - OpenMP 201511 (a.k.a. OpenMP 4.5)
  - CPU capability usage: AVX2
  - Build settings: BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREAD
POOL -fopenmp -DNDEBUG -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-p
ragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-err
or=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unuse
d-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_
WITH_AVX512=1, TORCH_VERSION=1.8.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL
=ON, USE_NNPACK=0, USE_OPENMP=ON,                                       
                          
Num threads: 6


---- Benchmark 2D ----     

Input tensor: [1, 3, 320, 320]                                         
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false
                                                                          
- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 1.11414
                                                                     
- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 0.36291

- Bench upsample_bilinear2d_cpu (7500 rounds) - upsampling to 512x512
Elapsed time (ms): 4.33236

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - upsampling to 512x512
Elapsed time (ms): 1.40062

Input tensor: [1, 12, 320, 320]
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false

- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 1.34784

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 0.430209

- Bench upsample_bilinear2d_cpu (7500 rounds) - upsampling to 512x512
Elapsed time (ms): 5.28019

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - upsampling to 512x512
Elapsed time (ms): 1.69565

Input tensor: [1, 24, 320, 320]
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false

- Bench upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 1.13603

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - downsampling to 256x256
Elapsed time (ms): 0.30654

- Bench upsample_bilinear2d_cpu (7500 rounds) - upsampling to 512x512
Elapsed time (ms): 4.41721

- Bench ti_upsample_bilinear2d_cpu (7500 rounds) - upsampling to 512x512
Elapsed time (ms): 1.50753


1 - Test size as in https://github.com/mingfeima/op_bench-py
                                                                        
Input tensor: [32, 128, 64, 64]                                      
Input is_contiguous memory_format torch.channels_last: true
Input is_contiguous : false    
                                                                        
- Bench upsample_bilinear2d_cpu (750 rounds) - upsampling to 128x128
Elapsed time (ms): 35.5334
                                                                       
- Bench ti_upsample_bilinear2d_cpu (750 rounds) - upsampling to 128x128
Elapsed time (ms): 32.8065
                                                                          
2 - Test size as in https://github.com/mingfeima/op_bench-py

Input tensor: [32, 128, 64, 64]
Input is_contiguous memory_format torch.channels_last: false
Input is_contiguous : true

- Bench upsample_bilinear2d_cpu (750 rounds) - upsampling to 128x128
Elapsed time (ms): 84.203

- Bench ti_upsample_bilinear2d_cpu (750 rounds) - upsampling to 128x128
Elapsed time (ms): 50.5433

---- END Benchmark 2D ----
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
