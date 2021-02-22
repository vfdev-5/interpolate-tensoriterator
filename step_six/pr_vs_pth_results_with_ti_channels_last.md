

<details>
<summary>
Interpolation 2d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.3261 | 0.1239 | 2.6318
[1, 3, 320, 320] | [512, 512] | True | False | 1.2854 | 0.4086 | 3.1458
[1, 3, 320, 320] | [256, 256] | False | False | 0.3488 | 0.0777 | 4.4919
[1, 3, 320, 320] | [512, 512] | False | False | 1.3063 | 0.4084 | 3.1984
[1, 3, 320, 320] | [256, 256] | False | True | 1.0897 | 0.3289 | 3.3132
[1, 3, 320, 320] | [512, 512] | False | True | 4.2505 | 1.3841 | 3.0711
[32, 128, 64, 64] | [32, 32] | False | True | 2.2961 | 2.9314 | 0.7833
[32, 128, 64, 64] | [128, 128] | False | True | 35.9384 | 35.6293 | 1.0087
[32, 128, 64, 64] | [32, 32] | True | False | 3.6902 | 3.5451 | 1.0409
[32, 128, 64, 64] | [128, 128] | True | False | 86.7835 | 52.4501 | 1.6546
[1, 3, 500, 500] | [256, 256] | True | False | 0.3266 | 0.0785 | 4.1601
[1, 3, 500, 500] | [800, 800] | True | False | 3.1868 | 0.5580 | 5.7114
[1, 3, 500, 500] | [256, 256] | False | False | 0.3771 | 0.0793 | 4.7555
[1, 3, 500, 500] | [800, 800] | False | False | 3.2693 | 0.5610 | 5.8271


</details>

<details>
<summary>
Interpolation 1d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.2808 | 0.1044 | 2.6907
[4, 512, 320] | 512 | True | False | 0.5524 | 0.1887 | 2.9269


</details>

<details>
<summary>
Interpolation 3d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 4.4017 | 0.9578 | 4.5958
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 84.0302 | 24.0669 | 3.4915
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.6098 | 3.0288 | 4.4934
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 246.6380 | 64.3400 | 3.8334


</details>

<details>
<summary>
Interpolation 2d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.8967 | 0.4551 | 1.9703
[1, 3, 320, 320] | [512, 512] | True | False | 3.5399 | 1.7594 | 2.0120
[1, 3, 320, 320] | [256, 256] | False | False | 0.9760 | 0.3305 | 2.9531
[1, 3, 320, 320] | [512, 512] | False | False | 3.6266 | 1.7643 | 2.0555
[1, 3, 320, 320] | [256, 256] | False | True | 1.0093 | 1.6589 | 0.6084
[1, 3, 320, 320] | [512, 512] | False | True | 4.0231 | 7.1302 | 0.5642
[32, 128, 64, 64] | [32, 32] | False | True | 5.8736 | 9.6382 | 0.6094
[32, 128, 64, 64] | [128, 128] | False | True | 108.2541 | 117.1183 | 0.9243
[32, 128, 64, 64] | [32, 32] | True | False | 19.9122 | 14.0883 | 1.4134
[32, 128, 64, 64] | [128, 128] | True | False | 398.8196 | 205.5317 | 1.9404
[1, 3, 500, 500] | [256, 256] | True | False | 0.8944 | 0.3388 | 2.6404
[1, 3, 500, 500] | [800, 800] | True | False | 8.6327 | 2.9568 | 2.9196
[1, 3, 500, 500] | [256, 256] | False | False | 1.0921 | 0.3405 | 3.2076
[1, 3, 500, 500] | [800, 800] | False | False | 8.9394 | 2.9654 | 3.0145


</details>

<details>
<summary>
Interpolation 1d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 1.5233 | 0.5066 | 3.0071
[4, 512, 320] | 512 | True | False | 3.0312 | 0.9796 | 3.0943


</details>

<details>
<summary>
Interpolation 3d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 12.0408 | 4.8498 | 2.4827
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 222.8379 | 105.1315 | 2.1196
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.3036 | 17.2361 | 0.7718
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 245.9575 | 297.0317 | 0.8281


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
