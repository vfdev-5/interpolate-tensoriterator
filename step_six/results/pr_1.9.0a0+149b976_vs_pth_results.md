

<details>
<summary>
Interpolation 2d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.3261 | 0.0782 | 4.1711
[1, 3, 320, 320] | [512, 512] | True | False | 1.2854 | 0.4083 | 3.1480
[1, 3, 320, 320] | [256, 256] | False | False | 0.3488 | 0.0770 | 4.5277
[1, 3, 320, 320] | [512, 512] | False | False | 1.3063 | 0.4120 | 3.1707
[1, 3, 320, 320] | [256, 256] | False | True | 1.0897 | 0.3201 | 3.4040
[1, 3, 320, 320] | [512, 512] | False | True | 4.2505 | 1.3706 | 3.1012
[32, 128, 64, 64] | [32, 32] | False | True | 2.2961 | 2.9161 | 0.7874
[32, 128, 64, 64] | [128, 128] | False | True | 35.9384 | 35.5796 | 1.0101
[32, 128, 64, 64] | [32, 32] | True | False | 3.6902 | 3.4111 | 1.0818
[32, 128, 64, 64] | [128, 128] | True | False | 86.7835 | 52.4608 | 1.6543
[1, 3, 500, 500] | [256, 256] | True | False | 0.3266 | 0.0775 | 4.2136
[1, 3, 500, 500] | [800, 800] | True | False | 3.1868 | 0.5574 | 5.7176
[1, 3, 500, 500] | [256, 256] | False | False | 0.3771 | 0.0790 | 4.7761
[1, 3, 500, 500] | [800, 800] | False | False | 3.2693 | 0.5602 | 5.8360


</details>

<details>
<summary>
Interpolation 1d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.2808 | 0.1029 | 2.7281
[4, 512, 320] | 512 | True | False | 0.5524 | 0.1877 | 2.9420


</details>

<details>
<summary>
Interpolation 3d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 4.4017 | 0.9523 | 4.6223
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 84.0302 | 23.9800 | 3.5042
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.6098 | 3.0330 | 4.4873
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 246.6380 | 57.3959 | 4.2971


</details>

<details>
<summary>
Interpolation 2d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.8967 | 0.4548 | 1.9717
[1, 3, 320, 320] | [512, 512] | True | False | 3.5399 | 1.7646 | 2.0061
[1, 3, 320, 320] | [256, 256] | False | False | 0.9760 | 0.3304 | 2.9544
[1, 3, 320, 320] | [512, 512] | False | False | 3.6266 | 1.7685 | 2.0506
[1, 3, 320, 320] | [256, 256] | False | True | 1.0093 | 1.6634 | 0.6068
[1, 3, 320, 320] | [512, 512] | False | True | 4.0231 | 7.1345 | 0.5639
[32, 128, 64, 64] | [32, 32] | False | True | 5.8736 | 9.7192 | 0.6043
[32, 128, 64, 64] | [128, 128] | False | True | 108.2541 | 117.5123 | 0.9212
[32, 128, 64, 64] | [32, 32] | True | False | 19.9122 | 14.1282 | 1.4094
[32, 128, 64, 64] | [128, 128] | True | False | 398.8196 | 205.1126 | 1.9444
[1, 3, 500, 500] | [256, 256] | True | False | 0.8944 | 0.3389 | 2.6389
[1, 3, 500, 500] | [800, 800] | True | False | 8.6327 | 2.9608 | 2.9156
[1, 3, 500, 500] | [256, 256] | False | False | 1.0921 | 0.3431 | 3.1833
[1, 3, 500, 500] | [800, 800] | False | False | 8.9394 | 2.9665 | 3.0135


</details>

<details>
<summary>
Interpolation 1d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 1.5233 | 0.5057 | 3.0121
[4, 512, 320] | 512 | True | False | 3.0312 | 0.9725 | 3.1171


</details>

<details>
<summary>
Interpolation 3d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 12.0408 | 4.8766 | 2.4691
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 222.8379 | 105.6527 | 2.1092
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.3036 | 19.2686 | 0.6904
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 245.9575 | 341.4843 | 0.7203


</details>


<details>
<summary>
Versions and build configs
</summary>

PyTorch master: 1.8.0.dev20210208+cu110
PyTorch master build setting:
```
BLAS_INFO=mkl, BUILD_TYPE=Release, CUDA_VERSION=11.0, CUDNN_VERSION=8.0.5, CXX_COMPILER=/opt/rh/devtoolset-7/root/usr/bin/c++, CXX_FLAGS= -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_FBGEMM -DUSE_QNNPACK -DUSE_PYTORCH_QNNPACK -DUSE_XNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Wno-stringop-overflow, LAPACK_INFO=mkl, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.8.0, USE_CUDA=ON, USE_CUDNN=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=ON, USE_MKLDNN=ON, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=ON, USE_OPENMP=ON,
```

PR : 1.9.0a0+149b976
PR build setting:
```
BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/bin/g++-7, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.9.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,
```
</details>
