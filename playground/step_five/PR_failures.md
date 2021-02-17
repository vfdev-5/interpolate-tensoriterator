# Test failures : https://github.com/pytorch/pytorch/pull/51653

1) https://app.circleci.com/pipelines/github/pytorch/pytorch/268872/workflows/685ced97-77b0-4988-a600-dcddb93a0f72/jobs/10640812

```bash
python test/run_test.py -vi test_nn -- -k "test_interpolate"
```
Output
```
ERROR: test_interpolate (__main__.TestNN)
----------------------------------------------------------------------
Traceback (most recent call last):
  File "test_nn.py", line 8813, in test_interpolate
  ...
RuntimeError: strides[1] == 0 INTERNAL ASSERT FAILED at "aten/src/ATen/native/cpu/UpSampleMoreKernel.cpp.AVX2.cpp":531, please report a bug to PyTorch. 8
```

2) https://app.circleci.com/pipelines/github/pytorch/pytorch/268872/workflows/685ced97-77b0-4988-a600-dcddb93a0f72/jobs/10639987

Can not build on Windows
```
aten\src\ATen\native\cpu\UpSampleMoreKernel.cpp.DEFAULT.cpp(746): note: see reference to function template instantiation 'void at::native::`anonymous-namespace'::ti_cpu_upsample_linear<scalar_t,index_t,1>(at::TensorIterator &)' being compiled
        with
        [
            index_t=int32_t
        ]
aten\src\ATen\native\cpu\UpSampleMoreKernel.cpp.DEFAULT.cpp(779): note: see reference to function template instantiation 'at::Tensor at::native::`anonymous-namespace'::ti_upsample_linearNd_kernel_impl<int32_t,1,at::native::`anonymous-namespace'::scale_t>(at::Tensor &,const at::Tensor &,bool,const scale_type &)' being compiled
        with
        [
            scale_type=at::native::`anonymous-namespace'::scale_t
        ]
aten\src\ATen\native\cpu\UpSampleMoreKernel.cpp.DEFAULT.cpp(591): error C2133: 'constval_idx_ptrs': unknown size
```


3) https://app.circleci.com/pipelines/github/pytorch/pytorch/268872/workflows/685ced97-77b0-4988-a600-dcddb93a0f72/jobs/10640662

```bash
python test/run_test.py -vi test_nn -- -k "test_upsamplingBilinear2d_spatial_invariance"
```
