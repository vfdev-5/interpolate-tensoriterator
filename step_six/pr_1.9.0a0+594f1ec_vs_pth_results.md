

<details>
<summary>
Interpolation 2d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.3261 | 0.0789 | 4.1354
[1, 3, 320, 320] | [512, 512] | True | False | 1.2854 | 0.4093 | 3.1405
[1, 3, 320, 320] | [256, 256] | False | False | 0.3488 | 0.0776 | 4.4972
[1, 3, 320, 320] | [512, 512] | False | False | 1.3063 | 0.4087 | 3.1959
[1, 3, 320, 320] | [256, 256] | False | True | 1.0897 | 1.0897 | 1.0000
[1, 3, 320, 320] | [512, 512] | False | True | 4.2505 | 4.2387 | 1.0028
[32, 128, 64, 64] | [32, 32] | False | True | 2.2961 | 2.2847 | 1.0050
[32, 128, 64, 64] | [128, 128] | False | True | 35.9384 | 35.9696 | 0.9991
[32, 128, 64, 64] | [32, 32] | True | False | 3.6902 | 2.4377 | 1.5138
[32, 128, 64, 64] | [128, 128] | True | False | 86.7835 | 52.4367 | 1.6550
[1, 3, 500, 500] | [256, 256] | True | False | 0.3266 | 0.0782 | 4.1752
[1, 3, 500, 500] | [800, 800] | True | False | 3.1868 | 0.5605 | 5.6860
[1, 3, 500, 500] | [256, 256] | False | False | 0.3771 | 0.0793 | 4.7574
[1, 3, 500, 500] | [800, 800] | False | False | 3.2693 | 0.5631 | 5.8061


</details>

<details>
<summary>
Interpolation 1d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.2808 | 0.1036 | 2.7112
[4, 512, 320] | 512 | True | False | 0.5524 | 0.1884 | 2.9319


</details>

<details>
<summary>
Interpolation 3d - 6 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 4.4017 | 2.0948 | 2.1012
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 84.0302 | 42.6498 | 1.9702
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.6098 | 13.6576 | 0.9965
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 246.6380 | 246.2547 | 1.0016


</details>

<details>
<summary>
Interpolation 2d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.8967 | 0.3298 | 2.7190
[1, 3, 320, 320] | [512, 512] | True | False | 3.5399 | 1.7587 | 2.0128
[1, 3, 320, 320] | [256, 256] | False | False | 0.9760 | 0.3296 | 2.9614
[1, 3, 320, 320] | [512, 512] | False | False | 3.6266 | 1.7632 | 2.0568
[1, 3, 320, 320] | [256, 256] | False | True | 1.0093 | 1.0075 | 1.0018
[1, 3, 320, 320] | [512, 512] | False | True | 4.0231 | 4.0089 | 1.0035
[32, 128, 64, 64] | [32, 32] | False | True | 5.8736 | 5.8861 | 0.9979
[32, 128, 64, 64] | [128, 128] | False | True | 108.2541 | 107.5471 | 1.0066
[32, 128, 64, 64] | [32, 32] | True | False | 19.9122 | 10.0503 | 1.9813
[32, 128, 64, 64] | [128, 128] | True | False | 398.8196 | 204.6447 | 1.9488
[1, 3, 500, 500] | [256, 256] | True | False | 0.8944 | 0.3358 | 2.6638
[1, 3, 500, 500] | [800, 800] | True | False | 8.6327 | 2.9512 | 2.9252
[1, 3, 500, 500] | [256, 256] | False | False | 1.0921 | 0.3404 | 3.2080
[1, 3, 500, 500] | [800, 800] | False | False | 8.9394 | 2.9619 | 3.0181


</details>

<details>
<summary>
Interpolation 1d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 1.5233 | 0.5065 | 3.0076
[4, 512, 320] | 512 | True | False | 3.0312 | 0.9801 | 3.0929


</details>

<details>
<summary>
Interpolation 3d - 1 thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 12.0408 | 11.3036 | 1.0652
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 222.8379 | 209.6775 | 1.0628
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.3036 | 13.3229 | 0.9986
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 245.9575 | 245.5839 | 1.0015


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

PR : 1.9.0a0+594f1ec
PR build setting:
```
BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/bin/g++-7, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.9.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,
```
</details>
