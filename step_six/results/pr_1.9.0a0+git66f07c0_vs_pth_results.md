

<details>
<summary>
Interpolation 2d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.0766 | 0.1203 | 0.6368
[1, 3, 320, 320] | [512, 512] | True | False | 0.4054 | 0.4003 | 1.0126
[1, 3, 320, 320] | [256, 256] | False | False | 0.0760 | 0.0747 | 1.0170
[1, 3, 320, 320] | [512, 512] | False | False | 0.4068 | 0.4001 | 1.0168
[1, 3, 320, 320] | [256, 256] | False | True | 1.2060 | 0.0745 | 16.1887
[1, 3, 320, 320] | [512, 512] | False | True | 4.8605 | 0.3992 | 12.1752
[32, 128, 64, 64] | [32, 32] | False | True | 6.4047 | 8.9005 | 0.7196
[32, 128, 64, 64] | [128, 128] | False | True | 165.7918 | 68.1139 | 2.4340
[32, 128, 64, 64] | [32, 32] | True | False | 2.3176 | 3.3407 | 0.6937
[32, 128, 64, 64] | [128, 128] | True | False | 52.3114 | 51.7710 | 1.0104
[1, 3, 500, 500] | [256, 256] | True | False | 0.0768 | 0.0744 | 1.0332
[1, 3, 500, 500] | [800, 800] | True | False | 0.5596 | 0.5433 | 1.0300
[1, 3, 500, 500] | [256, 256] | False | False | 0.0776 | 0.0755 | 1.0270
[1, 3, 500, 500] | [800, 800] | False | False | 0.5621 | 0.5469 | 1.0277


</details>

<details>
<summary>
Interpolation 1d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.1023 | 0.1051 | 0.9737
[4, 512, 320] | 512 | True | False | 0.1876 | 0.1943 | 0.9655


</details>

<details>
<summary>
Interpolation 3d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 2.1925 | 0.9994 | 2.1938
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 43.0123 | 24.8409 | 1.7315
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 15.5746 | 1.2676 | 12.2867
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 272.3312 | 25.1733 | 10.8182


</details>

<details>
<summary>
Interpolation 2d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.4525 | 0.4048 | 1.1178
[1, 3, 320, 320] | [512, 512] | True | False | 1.7609 | 1.3484 | 1.3059
[1, 3, 320, 320] | [256, 256] | False | False | 0.3288 | 0.3185 | 1.0322
[1, 3, 320, 320] | [512, 512] | False | False | 1.7659 | 1.1868 | 1.4879
[1, 3, 320, 320] | [256, 256] | False | True | 1.5450 | 0.3221 | 4.7966
[1, 3, 320, 320] | [512, 512] | False | True | 6.1403 | 1.1891 | 5.1638
[32, 128, 64, 64] | [32, 32] | False | True | 27.6198 | 44.4654 | 0.6212
[32, 128, 64, 64] | [128, 128] | False | True | 469.8933 | 284.5639 | 1.6513
[32, 128, 64, 64] | [32, 32] | True | False | 10.0300 | 12.9194 | 0.7764
[32, 128, 64, 64] | [128, 128] | True | False | 204.3304 | 204.5542 | 0.9989
[1, 3, 500, 500] | [256, 256] | True | False | 0.3381 | 0.3279 | 1.0310
[1, 3, 500, 500] | [800, 800] | True | False | 2.9525 | 2.8614 | 1.0318
[1, 3, 500, 500] | [256, 256] | False | False | 0.3399 | 0.3295 | 1.0315
[1, 3, 500, 500] | [800, 800] | False | False | 2.9591 | 2.8704 | 1.0309


</details>

<details>
<summary>
Interpolation 1d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.5028 | 0.5177 | 0.9712
[4, 512, 320] | 512 | True | False | 0.9689 | 1.0026 | 0.9664


</details>

<details>
<summary>
Interpolation 3d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 11.2675 | 5.2029 | 2.1656
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 209.8922 | 110.0209 | 1.9077
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 18.0122 | 5.8879 | 3.0592
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 381.7173 | 111.9734 | 3.4090


</details>


<details>
<summary>
Versions and build configs
</summary>

PyTorch master: 1.9.0.dev20210302+cpu
PyTorch master build setting:
```
BLAS_INFO=mkl, BUILD_TYPE=Release, CXX_COMPILER=/opt/rh/devtoolset-7/root/usr/bin/c++, CXX_FLAGS= -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_FBGEMM -DUSE_QNNPACK -DUSE_PYTORCH_QNNPACK -DUSE_XNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Wno-stringop-overflow, LAPACK_INFO=mkl, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.9.0, USE_CUDA=0, USE_CUDNN=OFF, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=ON, USE_MKLDNN=ON, USE_MPI=OFF, USE_NCCL=OFF, USE_NNPACK=ON, USE_OPENMP=ON,
```

PR : 1.9.0a0+git66f07c0
PR build setting:
```
BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS= -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.9.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,
```
</details>
