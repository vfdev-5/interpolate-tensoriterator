

<details>
<summary>
Interpolation 2d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.3938 | 0.0782 | 5.0339
[1, 3, 320, 320] | [512, 512] | True | False | 1.5585 | 0.4105 | 3.7965
[1, 3, 320, 320] | [256, 256] | False | False | 0.3481 | 0.0760 | 4.5780
[1, 3, 320, 320] | [512, 512] | False | False | 1.5848 | 0.4091 | 3.8734
[1, 3, 320, 320] | [256, 256] | False | True | 1.2058 | 1.2034 | 1.0020
[1, 3, 320, 320] | [512, 512] | False | True | 4.8691 | 4.8537 | 1.0032
[32, 128, 64, 64] | [32, 32] | False | True | 6.3915 | 6.4041 | 0.9980
[32, 128, 64, 64] | [128, 128] | False | True | 166.1769 | 164.5621 | 1.0098
[32, 128, 64, 64] | [32, 32] | True | False | 3.7194 | 2.4720 | 1.5046
[32, 128, 64, 64] | [128, 128] | True | False | 86.6704 | 52.3754 | 1.6548
[1, 3, 500, 500] | [256, 256] | True | False | 0.3270 | 0.0792 | 4.1307
[1, 3, 500, 500] | [800, 800] | True | False | 3.3116 | 0.5567 | 5.9482
[1, 3, 500, 500] | [256, 256] | False | False | 0.3763 | 0.0773 | 4.8700
[1, 3, 500, 500] | [800, 800] | False | False | 3.2577 | 0.5590 | 5.8279


</details>

<details>
<summary>
Interpolation 1d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.2795 | 0.1032 | 2.7089
[4, 512, 320] | 512 | True | False | 0.5533 | 0.1888 | 2.9303


</details>

<details>
<summary>
Interpolation 3d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 4.4105 | 2.1236 | 2.0769
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 83.9426 | 42.6641 | 1.9675
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 15.5736 | 15.5758 | 0.9999
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 272.4795 | 273.2745 | 0.9971


</details>

<details>
<summary>
Interpolation 2d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 1.0240 | 0.4145 | 2.4705
[1, 3, 320, 320] | [512, 512] | True | False | 4.0771 | 1.3836 | 2.9467
[1, 3, 320, 320] | [256, 256] | False | False | 0.9771 | 0.3270 | 2.9878
[1, 3, 320, 320] | [512, 512] | False | False | 4.1732 | 1.2209 | 3.4180
[1, 3, 320, 320] | [256, 256] | False | True | 1.5466 | 1.5363 | 1.0067
[1, 3, 320, 320] | [512, 512] | False | True | 6.1555 | 6.1199 | 1.0058
[32, 128, 64, 64] | [32, 32] | False | True | 27.6362 | 27.5901 | 1.0017
[32, 128, 64, 64] | [128, 128] | False | True | 468.6442 | 465.5163 | 1.0067
[32, 128, 64, 64] | [32, 32] | True | False | 20.1495 | 10.0694 | 2.0011
[32, 128, 64, 64] | [128, 128] | True | False | 400.0401 | 204.0662 | 1.9603
[1, 3, 500, 500] | [256, 256] | True | False | 0.8956 | 0.3366 | 2.6606
[1, 3, 500, 500] | [800, 800] | True | False | 8.6554 | 2.9530 | 2.9310
[1, 3, 500, 500] | [256, 256] | False | False | 1.0921 | 0.3385 | 3.2263
[1, 3, 500, 500] | [800, 800] | False | False | 8.9594 | 2.9627 | 3.0241


</details>

<details>
<summary>
Interpolation 1d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 1.5233 | 0.5027 | 3.0301
[4, 512, 320] | 512 | True | False | 3.0302 | 0.9735 | 3.1128


</details>

<details>
<summary>
Interpolation 3d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 12.0477 | 11.3196 | 1.0643
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 222.8618 | 209.9955 | 1.0613
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 17.9883 | 17.9937 | 0.9997
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 380.7244 | 380.1916 | 1.0014


</details>


<details>
<summary>
Versions and build configs
</summary>

PyTorch master: 1.9.0.dev20210223
PyTorch master build setting:
```
BLAS_INFO=mkl, BUILD_TYPE=Release, CUDA_VERSION=10.2, CUDNN_VERSION=7.6.5, CXX_COMPILER=/opt/rh/devtoolset-7/root/usr/bin/c++, CXX_FLAGS= -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_FBGEMM -DUSE_QNNPACK -DUSE_PYTORCH_QNNPACK -DUSE_XNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Wno-stringop-overflow, LAPACK_INFO=mkl, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.9.0, USE_CUDA=ON, USE_CUDNN=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=ON, USE_MKLDNN=ON, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=ON, USE_OPENMP=ON,
```

PR : 1.9.0a0+74b172b
PR build setting:
```
BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/bin/g++-7, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.9.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,
```
</details>
