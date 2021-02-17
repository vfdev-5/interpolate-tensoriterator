

<details>
<summary>
Interpolation 2d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.3146 | 0.0706 | 4.4559
[1, 3, 320, 320] | [512, 512] | True | False | 1.2432 | 0.2279 | 5.4551
[1, 3, 320, 320] | [256, 256] | False | False | 0.3402 | 0.0707 | 4.8141
[1, 3, 320, 320] | [512, 512] | False | False | 1.2668 | 0.2280 | 5.5564
[1, 3, 320, 320] | [256, 256] | False | True | 1.1038 | 1.1035 | 1.0003
[1, 3, 320, 320] | [512, 512] | False | True | 4.2852 | 4.2861 | 0.9998
[32, 128, 64, 64] | [128, 128] | False | True | 36.1045 | 36.1079 | 0.9999
[32, 128, 64, 64] | [128, 128] | True | False | 85.5408 | 51.5830 | 1.6583
[1, 3, 500, 500] | [256, 256] | True | False | 0.3155 | 0.0722 | 4.3665
[1, 3, 500, 500] | [800, 800] | True | False | 3.1574 | 0.5271 | 5.9906
[1, 3, 500, 500] | [256, 256] | False | False | 0.3857 | 0.0727 | 5.3033
[1, 3, 500, 500] | [800, 800] | False | False | 3.3122 | 0.5299 | 6.2511


</details>

<details>
<summary>
Interpolation 1d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.2830 | 0.1021 | 2.7715
[4, 512, 320] | 512 | True | False | 0.5616 | 0.1894 | 2.9648


</details>

<details>
<summary>
Interpolation 3d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 4.4820 | 2.0578 | 2.1781
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 85.2828 | 42.9003 | 1.9879


</details>

<details>
<summary>
Interpolation 2d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.8650 | 0.3117 | 2.7747
[1, 3, 320, 320] | [512, 512] | True | False | 3.4385 | 1.1730 | 2.9312
[1, 3, 320, 320] | [256, 256] | False | False | 0.9493 | 0.3149 | 3.0149
[1, 3, 320, 320] | [512, 512] | False | False | 3.5222 | 1.1760 | 2.9951
[1, 3, 320, 320] | [256, 256] | False | True | 1.0316 | 1.0246 | 1.0068
[1, 3, 320, 320] | [512, 512] | False | True | 4.0982 | 4.0623 | 1.0088
[32, 128, 64, 64] | [128, 128] | False | True | 109.2070 | 109.7230 | 0.9953
[32, 128, 64, 64] | [128, 128] | True | False | 392.0510 | 202.8110 | 1.9331
[1, 3, 500, 500] | [256, 256] | True | False | 0.8682 | 0.3219 | 2.6968
[1, 3, 500, 500] | [800, 800] | True | False | 8.3855 | 2.8295 | 2.9636
[1, 3, 500, 500] | [256, 256] | False | False | 1.1391 | 0.3253 | 3.5019
[1, 3, 500, 500] | [800, 800] | False | False | 8.7155 | 2.8346 | 3.0747


</details>

<details>
<summary>
Interpolation 1d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 1.5497 | 0.5147 | 3.0109
[4, 512, 320] | 512 | True | False | 3.0876 | 0.9926 | 3.1105


</details>

<details>
<summary>
Interpolation 3d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 12.3265 | 11.2472 | 1.0960
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 227.2940 | 208.5290 | 1.0900


</details>


<details>
<summary>
Versions and build configs
</summary>

PyTorch master: '1.8.0a0+8c5b024'
PyTorch master build setting:
```
BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.8.0, USE_CUDA=1, USE_CUDNN=1, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,
```

PR : '1.8.0a0+ee32a0c'
PR build setting:
```
BUILD_TYPE=Release, CUDA_VERSION=11.1, CUDNN_VERSION=8.0.5, CXX_COMPILER=/usr/lib/ccache/c++, CXX_FLAGS=-O3 -Wno-deprecated -fvisibility-inlines-hidden -DUSE_PTHREADPOOL -fopenmp -DNDEBUG -DUSE_KINETO -DUSE_PYTORCH_QNNPACK -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow, PERF_WITH_AVX=1, PERF_WITH_AVX2=1, PERF_WITH_AVX512=1, TORCH_VERSION=1.8.0, USE_CUDA=1, USE_CUDNN=ON, USE_EIGEN_FOR_BLAS=ON, USE_EXCEPTION_PTR=1, USE_GFLAGS=OFF, USE_GLOG=OFF, USE_MKL=OFF, USE_MKLDNN=OFF, USE_MPI=OFF, USE_NCCL=ON, USE_NNPACK=0, USE_OPENMP=ON,
```
</details>
