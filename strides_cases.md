### List all possible cases

Different 1D cases:

- 1D, NCL, L > 1 | {1, 4, 32} -> {1, 4, 16}

```
n=16
dst strides[0] = 4
src strides[1] = 0
i d1 strides[2] = 4
w d1 strides[3] = 4
```

- 1D, NCL, L == 1 | {1, 4, 32} -> {1, 4, 1}

```
n=1
dst strides[0] = 4
src strides[1] = 128
i d1 strides[2] = 0
w d1 strides[3] = 0
```



Different 2D cases:

- 2D, NCHW, channel first, H, W > 1 | {1, 3, 4, 32} -> {1, 3, 2, 16}

```
n=16
dst strides[0] = 4
src strides[1] = 0
i d1 strides[2] = 0
w d1 strides[3] = 0
i d2 strides[6] = 4
w d2 strides[7] = 4
```


- 2D, NCHW, channel first, H > 1, W == 1 | {1, 3, 8, 8} -> {1, 3, 4, 1}

```
n=4
dst strides[0] = 4
src strides[1] = 0
i d1 strides[2] = 4
w d1 strides[3] = 4
i d2 strides[6] = 0
w d2 strides[7] = 0
```


- 2D, NCHW, channel first, H == 1, W == 1 | {1, 3, 8, 8} -> {1, 3, 1, 1}

```
n=3
dst strides[0] = 4
src strides[1] = 256
i d1 strides[2] = 0
w d1 strides[3] = 0
i d2 strides[6] = 0
w d2 strides[7] = 0
```


- 2D, NCHW, channel last, H, W > 1 | {1, 3, 4, 32} -> {1, 3, 2, 16}
- 2D, NCHW, channel last, H > 1, W == 1 | {1, 3, 8, 8} -> {1, 3, 4, 1}
- 2D, NCHW, channel last, H == 1, W == 1 | {1, 3, 8, 8} -> {1, 3, 1, 1}

```
n=3
dst strides[0] = 4
src strides[1] = 4
i d1 strides[2] = 0
w d1 strides[3] = 0
i d2 strides[6] = 0
w d2 strides[7] = 0
```

Different 3D cases:

- 3D, NCKHW, channel first, K, H, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 4, 4, 4}
- 3D, NCKHW, channel first, K == 1, H == 1, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 1, 1, 4}
- 3D, NCKHW, channel first, K == 1, H > 1, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 1, 4, 4}
- 3D, NCKHW, channel first, K > 1, H == 1, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 4, 1, 4}

```
n=4
dst strides[0] = 4
src strides[1] = 0
i d1 strides[2] = 0
w d1 strides[3] = 0
i d2 strides[6] = 0
w d2 strides[7] = 0
i d3 strides[10] = 4
w d3 strides[11] = 4
```

- 3D, NCKHW, channel first, K, H > 1, W == 1 | {1, 3, 8, 8, 8} -> {1, 3, 4, 4, 1}
- 3D, NCKHW, channel first, K > 1, H == 1, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 1, 4, 1}

```
n=4
dst strides[0] = 4
src strides[1] = 0
i d1 strides[2] = 0
w d1 strides[3] = 0
i d2 strides[6] = 4
w d2 strides[7] = 4
i d3 strides[10] = 0
w d3 strides[11] = 0
```

- 3D, NCKHW, channel first, K, H == 1, W == 1 | {1, 3, 8, 8, 8} -> {1, 3, 4, 1, 1}

```
n=4
dst strides[0] = 4
src strides[1] = 0
i d1 strides[2] = 4
w d1 strides[3] = 4
i d2 strides[6] = 0
w d2 strides[7] = 0
i d3 strides[10] = 0
w d3 strides[11] = 0
```

- 3D, NCKHW, channel first, K, H == 1, W == 1 | {1, 3, 8, 8, 8} -> {1, 3, 1, 1, 1}

```
n=3
dst strides[0] = 4
src strides[1] = 2048
i d1 strides[2] = 0
w d1 strides[3] = 0
i d2 strides[6] = 0
w d2 strides[7] = 0
i d3 strides[10] = 0
w d3 strides[11] = 0
```

- 3D, NCKHW, channel last, K, H, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 4, 4, 4}
- 3D, NCKHW, channel last, K == 1, H, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 1, 4, 4}
- 3D, NCKHW, channel last, K == 1, H == 1, W > 1 | {1, 3, 8, 8, 8} -> {1, 3, 1, 1, 4}
- ...

```
n=3
dst strides[0] = 4
src strides[1] = 4
i d1 strides[2] = 0
w d1 strides[3] = 0
i d2 strides[6] = 0
w d2 strides[7] = 0
i d3 strides[10] = 0
w d3 strides[11] = 0
```




## Code to reproduce

```c++
#include <vector>
#include <ATen/native/TensorIterator.h>
#include <ATen/TypeDefault.h>
#include <ATen/native/IndexingUtils.h>
#include <iostream>
#include <ATen/ATen.h>
#include <ATen/native/UpSample.h>


using namespace at;
using namespace at::native;

template <typename scalar_t, typename index_t, int out_ndims>
inline void assert_strides_linear(const int64_t* strides) {
  for (int i=0; i<out_ndims; i++) {
    // Assert strides for indices 0, 1 and weights 0, 1
    TORCH_INTERNAL_ASSERT(
      strides[4 * i + 0 + 2] == strides[4 * i + 2 + 2], strides[4 * i + 0 + 2], strides[4 * i + 2 + 2]        
    );
    TORCH_INTERNAL_ASSERT(
      strides[4 * i + 1 + 2] == strides[4 * i + 3 + 2], strides[4 * i + 1 + 2], strides[4 * i + 3 + 2]
    );
  }

  // Assert zero stride for indices and weights on dims -2, -3, ...
  for (int i=0; i<out_ndims - 1; i++) {
    TORCH_INTERNAL_ASSERT(strides[4 * i + 0 + 2] == 0, strides[4 * i + 0 + 2]);
    TORCH_INTERNAL_ASSERT(strides[4 * i + 1 + 2] == 0, strides[4 * i + 1 + 2]);
  }

  // Assert zero stride for the source
  TORCH_INTERNAL_ASSERT(strides[1] == 0, strides[1]);

  // Assert stride for the output
  TORCH_INTERNAL_ASSERT(strides[0] == sizeof(scalar_t), strides[0], sizeof(scalar_t));

  // Assert non-zero stride for indices and weights on dim -1
  int i = out_ndims - 1;
  TORCH_INTERNAL_ASSERT(strides[4 * i + 0 + 2] == sizeof(index_t), strides[4 * i + 0 + 2], sizeof(index_t));
  TORCH_INTERNAL_ASSERT(strides[4 * i + 1 + 2] == sizeof(scalar_t), strides[4 * i + 1 + 2], sizeof(scalar_t));
}


template <typename scalar_t, typename index_t, int out_ndims>
void ti_cpu_upsample_linear(TensorIterator& iter) {

  auto loop = [&](char** data, const int64_t* strides, int64_t n) {

    scalar_t * dst = (scalar_t *) data[0];
    scalar_t * src = (scalar_t *) data[1];

    std::cout << "n=" << n << std::endl;
    std::cout << "dst strides[0] = " << strides[0] << std::endl;
    std::cout << "src strides[1] = " << strides[1] << std::endl;
    std::cout << "i d1 strides[2] = " << strides[2] << std::endl;
    std::cout << "w d1 strides[3] = " << strides[3] << std::endl;
    if (out_ndims > 1) {
        std::cout << "i d2 strides[6] = " << strides[6] << std::endl;
        std::cout << "w d2 strides[7] = " << strides[7] << std::endl;    
    }
    if (out_ndims > 2) {
        std::cout << "i d3 strides[10] = " << strides[10] << std::endl;
        std::cout << "w d3 strides[11] = " << strides[11] << std::endl;
    }

//     std::cout << "src= ";
//     for (int64_t i=0; i<std::min(n, (int64_t) 10); i++) {
//         std::cout << *src << " ";
//         src += strides[1];
//     }
//     std::cout << std::endl;

//     assert_strides_linear<scalar_t, index_t, out_ndims>(strides);

  };

  iter.for_each(loop);
}

template<typename index_t, typename scalar_t>
std::vector<Tensor> ti_compute_indices_weights_linear(
  int64_t input_size, int64_t output_size, int64_t stride, int64_t ndims, int64_t reshape_dim, 
  bool align_corners, const c10::optional<double> opt_scale
) {

  scalar_t scale = area_pixel_compute_scale<scalar_t>(input_size, output_size, align_corners, opt_scale);

  std::vector<Tensor> output;
  auto new_shape = std::vector<int64_t>(ndims, 1);
  new_shape[reshape_dim] = output_size;

  output.emplace_back(empty(new_shape, CPU(c10::CppTypeToScalarType<index_t>())));
  output.emplace_back(empty(new_shape, CPU(c10::CppTypeToScalarType<scalar_t>())));  
  output.emplace_back(empty(new_shape, CPU(c10::CppTypeToScalarType<index_t>())));
  output.emplace_back(empty(new_shape, CPU(c10::CppTypeToScalarType<scalar_t>())));

  auto input_index0_ptr = output[0].data_ptr<index_t>();
  auto lambda0_ptr = output[1].data_ptr<scalar_t>();
  auto input_index1_ptr = output[2].data_ptr<index_t>();
  auto lambda1_ptr = output[3].data_ptr<scalar_t>();

  double xd;
  int64_t xl;
  
  for (int64_t i=0; i<output_size; i++) {

    compute_source_index_and_lambda<scalar_t, index_t>(
      input_index0_ptr[i], input_index1_ptr[i],
      lambda0_ptr[i], lambda1_ptr[i],
      scale, i, input_size, output_size, align_corners
    );
    input_index0_ptr[i] *= stride;
    input_index1_ptr[i] *= stride;
  }
  return output;
}


template <typename index_t, int out_ndims, typename scale_type>
Tensor ti_kernel(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    const scale_type& scales) {

  // input can be NCHW, NCL or NCKHW
  auto shape = input.sizes().vec();
  auto strides = input.strides().vec();
  auto oshape = output.sizes();

  for (int i=0; i<out_ndims; i++) {
    shape[i + 2] = oshape[i + 2];
    strides[i + 2] = 0;
  }
  auto restrided_input = input.as_strided(shape, strides);

  std::vector<std::vector<Tensor>> indices_weights;
  AT_DISPATCH_FLOATING_TYPES(
    input.scalar_type(), "compute_indices_weights_linear", [&] {
      for (int i=0; i<out_ndims; i++) {
        indices_weights.emplace_back(
          ti_compute_indices_weights_linear<index_t, scalar_t>(
            input.size(i + 2), oshape[i + 2], input.stride(i + 2), input.dim(), i + 2, align_corners, scales[i])
        );
      }
    }
  );

  TensorIteratorConfig config;
  config.check_all_same_dtype(false)
    .declare_static_dtype_and_device(input.scalar_type(), input.device())
    .add_output(output)
    .add_input(restrided_input);
  
  for (auto iter=indices_weights.begin(); iter!=indices_weights.end(); iter++) { 
    for (auto& tensor : *iter) {
      config.add_input(tensor);
    }
  }

  auto iter = config.build();

  AT_DISPATCH_FLOATING_TYPES(
      iter.dtype(), "upsample_linearNd", [&] {
      ti_cpu_upsample_linear<scalar_t, index_t, out_ndims>(iter);
  });

  return iter.output();
}


{
  // - 1D, NCL, L > 1
  auto input = at::ones({1, 4, 32}, at::CPU(at::kFloat));
  auto output = at::ones({1, 4, 16}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 1, scale_t>(output, input, false, {c10::nullopt});
}
{
  // - 1D, NCL, L == 1
  auto input = at::ones({1, 4, 32}, at::CPU(at::kFloat));
  auto output = at::ones({1, 4, 1}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 1, scale_t>(output, input, false, {c10::nullopt});
}
{
  // 2D, NCHW, channel first, H, W > 1
  auto input = at::ones({1, 3, 4, 32}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 2, 16}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 2, scale_t>(output, input, false, {c10::nullopt});
}
{
  // 2D, NCHW, channel first, H > 1, W == 1
  auto input = at::ones({1, 3, 8, 8}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 4, 1}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 2, scale_t>(output, input, false, {c10::nullopt});
}
{
  // 2D, NCHW, channel first, H == 1, W == 1
  auto input = at::ones({1, 3, 8, 8}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 1, 1}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 2, scale_t>(output, input, false, {c10::nullopt});
}
{
  // 2D, NCHW, channel last, H, W > 1
  auto input = at::ones({1, 4, 32, 3}, at::CPU(at::kFloat)).permute({0, 3, 1, 2});
  auto output = at::ones({1, 2, 16, 3}, at::CPU(at::kFloat)).permute({0, 3, 1, 2});

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 2, scale_t>(output, input, false, {c10::nullopt});
}
{
  // 2D, NCHW, channel last, H > 1, W == 1
  auto input = at::ones({1, 8, 8, 3}, at::CPU(at::kFloat)).permute({0, 3, 1, 2});
  auto output = at::ones({1, 1, 1, 3}, at::CPU(at::kFloat)).permute({0, 3, 1, 2});

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 2, scale_t>(output, input, false, {c10::nullopt});
}
{
  // - 3D, NCKHW, channel first, K, H, W > 1
  auto input = at::ones({1, 3, 8, 8, 8}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 4, 4, 4}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 3, scale_t>(output, input, false, {c10::nullopt});
}
{
  // - 3D, NCKHW, channel first, K, H == 1, W > 1
  auto input = at::ones({1, 3, 8, 8, 8}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 4, 1, 4}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 3, scale_t>(output, input, false, {c10::nullopt});
}
{
  // - 3D, NCKHW, channel first, K, H == 1, W == 1
  auto input = at::ones({1, 3, 8, 8, 8}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 4, 1, 1}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 3, scale_t>(output, input, false, {c10::nullopt});
}
{
  // - 3D, NCKHW, channel first, K > 1, H == 1, W == 1
  auto input = at::ones({1, 3, 8, 8, 8}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 4, 1, 4}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 3, scale_t>(output, input, false, {c10::nullopt});
}
{
  // - 3D, NCKHW, channel first, K == 1, H == 1, W == 1
  auto input = at::ones({1, 3, 8, 8, 8}, at::CPU(at::kFloat));
  auto output = at::ones({1, 3, 1, 1, 1}, at::CPU(at::kFloat));

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 3, scale_t>(output, input, false, {c10::nullopt});
}
{
  // - 3D, NCKHW, channel last, K > 1, H > 1, W > 1
  auto input = at::ones({1, 8, 8, 8, 3}, at::CPU(at::kFloat)).permute({0, 4, 1, 2, 3});
  auto output = at::ones({1, 4, 4, 4, 3}, at::CPU(at::kFloat)).permute({0, 4, 1, 2, 3});

  using scale_t = std::vector<c10::optional<double>>;
  auto out = ti_kernel<int32_t, 3, scale_t>(output, input, false, {c10::nullopt});
}
```
