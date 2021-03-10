

<details>
<summary>
Interpolation 2d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.3261 | 0.0808 | 4.0377
[1, 3, 320, 320] | [512, 512] | True | False | 1.2854 | 0.4111 | 3.1267
[1, 3, 320, 320] | [256, 256] | False | False | 0.3488 | 0.0777 | 4.4898
[1, 3, 320, 320] | [512, 512] | False | False | 1.3063 | 0.4100 | 3.1860
[1, 3, 320, 320] | [256, 256] | False | True | 1.0897 | 1.0875 | 1.0021
[1, 3, 320, 320] | [512, 512] | False | True | 4.2505 | 4.2442 | 1.0015
[32, 128, 64, 64] | [32, 32] | False | True | 2.2961 | 2.2834 | 1.0056
[32, 128, 64, 64] | [128, 128] | False | True | 35.9384 | 35.9576 | 0.9995
[32, 128, 64, 64] | [32, 32] | True | False | 3.6902 | 2.3240 | 1.5879
[32, 128, 64, 64] | [128, 128] | True | False | 86.7835 | 52.3617 | 1.6574
[1, 3, 500, 500] | [256, 256] | True | False | 0.3266 | 0.0800 | 4.0838
[1, 3, 500, 500] | [800, 800] | True | False | 3.1868 | 0.5588 | 5.7032
[1, 3, 500, 500] | [256, 256] | False | False | 0.3771 | 0.0794 | 4.7483
[1, 3, 500, 500] | [800, 800] | False | False | 3.2693 | 0.5631 | 5.8061


</details>

<details>
<summary>
Interpolation 1d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 0.2808 | 0.1037 | 2.7086
[4, 512, 320] | 512 | True | False | 0.5524 | 0.1923 | 2.8731


</details>

<details>
<summary>
Interpolation 3d - 6 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 4.4017 | 2.0998 | 2.0962
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 84.0302 | 42.7260 | 1.9667
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.6098 | 13.6228 | 0.9990
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 246.6380 | 246.4457 | 1.0008


</details>

<details>
<summary>
Interpolation 2d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 320, 320] | [256, 256] | True | False | 0.8967 | 0.4564 | 1.9645
[1, 3, 320, 320] | [512, 512] | True | False | 3.5399 | 1.7642 | 2.0065
[1, 3, 320, 320] | [256, 256] | False | False | 0.9760 | 0.3310 | 2.9482
[1, 3, 320, 320] | [512, 512] | False | False | 3.6266 | 1.7698 | 2.0492
[1, 3, 320, 320] | [256, 256] | False | True | 1.0093 | 1.0078 | 1.0015
[1, 3, 320, 320] | [512, 512] | False | True | 4.0231 | 4.0177 | 1.0013
[32, 128, 64, 64] | [32, 32] | False | True | 5.8736 | 5.9210 | 0.9920
[32, 128, 64, 64] | [128, 128] | False | True | 108.2541 | 106.9406 | 1.0123
[32, 128, 64, 64] | [32, 32] | True | False | 19.9122 | 10.0178 | 1.9877
[32, 128, 64, 64] | [128, 128] | True | False | 398.8196 | 203.9546 | 1.9554
[1, 3, 500, 500] | [256, 256] | True | False | 0.8944 | 0.3402 | 2.6294
[1, 3, 500, 500] | [800, 800] | True | False | 8.6327 | 2.9565 | 2.9200
[1, 3, 500, 500] | [256, 256] | False | False | 1.0921 | 0.3422 | 3.1916
[1, 3, 500, 500] | [800, 800] | False | False | 8.9394 | 2.9606 | 3.0194


</details>

<details>
<summary>
Interpolation 1d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[4, 512, 320] | 256 | True | False | 1.5233 | 0.5074 | 3.0022
[4, 512, 320] | 512 | True | False | 3.0312 | 0.9812 | 3.0894


</details>

<details>
<summary>
Interpolation 3d - 1 thread(s)
</summary>


In | Out | Is contigous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
[1, 3, 16, 320, 320] | [8, 256, 256] | True | False | 12.0408 | 11.3195 | 1.0637
[1, 3, 16, 320, 320] | [32, 512, 512] | True | False | 222.8379 | 209.5616 | 1.0634
[1, 3, 16, 320, 320] | [8, 256, 256] | False | True | 13.3036 | 13.3289 | 0.9981
[1, 3, 16, 320, 320] | [32, 512, 512] | False | True | 245.9575 | 245.7994 | 1.0006


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
