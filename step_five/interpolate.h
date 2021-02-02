#include <math.h>
#include <vector>
#include <ATen/TypeDefault.h>
#include <ATen/native/TensorIterator.h>
#include <ATen/native/IndexingUtils.h>
#include <ATen/native/UpSample.h>


// Copied from aten/src/ATen/native/cpu/UpSampleMoreKernel.cpp
// for reuse if goes as a PR in PyTorch
template<typename scalar_t, typename index_t>
static inline void compute_source_index_and_lambda(
    index_t& input_index0,
    index_t& input_index1,
    scalar_t& lambda0,
    scalar_t& lambda1,
    scalar_t ratio,
    int64_t output_index,
    int64_t input_size,
    int64_t output_size,
    bool align_corners) {
  if (output_size == input_size) {
    // scale_factor = 1, simply copy
    input_index0 = static_cast<index_t>(output_index);
    input_index1 = static_cast<index_t>(output_index);
    lambda0 = static_cast<scalar_t>(1);
    lambda1 = static_cast<scalar_t>(0);
  } else {
    const scalar_t real_input_index = at::native::area_pixel_compute_source_index<scalar_t>(
        ratio, output_index, align_corners, /*cubic=*/false);
    input_index0 = static_cast<index_t>(real_input_index);
    index_t offset = (input_index0 < input_size - 1) ? 1 : 0;
    input_index1 = input_index0 + offset;
    lambda1 = real_input_index - static_cast<scalar_t>(input_index0);
    lambda0 = static_cast<scalar_t>(1.) - lambda1;
  }
}
// End of copied ...

template <typename scalar_t, typename index_t, int step>
inline void load(scalar_t* dst, scalar_t *src, index_t * index) {
  for (int k = 0; k < step; k++) {
    *dst = *(src + *index);
    dst++;
    index++;
  }
}


template <typename scalar_t, int step>
inline void compute_linear(scalar_t* dst, scalar_t * src1, scalar_t* src2, scalar_t* w0, scalar_t* w1) {
  for (int k = 0; k < step; k++) {
    *dst = *src1 * *w0 + *src2 * *w1;
    dst++;
    src1++;
    src2++;
    w0++;
    w1++;
  }
}


template <typename scalar_t, int step>
inline void compute_linear(scalar_t* dst, scalar_t * src1, scalar_t* src2, scalar_t w0, scalar_t w1) {
  for (int k = 0; k < step; k++) {
    *dst = *src1 * w0 + *src2 * w1;
    dst++;
    src1++;
    src2++;
  }
}


// Interpolation type structure to compute output value in n-dimensional case.
// - use buffers (buf) to prefetch source data.
// - recursively compute interpolated output for each dimension
//
// for example for 2d:
// 
// source[0, 0] -> buffer[0]
// source[0, 1] -> buffer[1]
// interpolate(buffer[0], weight00, buffer[1], weight01) -> buffer[2]
//
// source[1, 0] -> buffer[0]
// source[1, 1] -> buffer[1]
// interpolate(buffer[0], weight00, buffer[1], weight01) -> buffer[3]
// 
// interpolate(buffer[2], weight10, buffer[3], weight11) -> output
//
template <int n, typename scalar_t, typename index_t, int step>
struct Interp {
    static inline void eval(scalar_t* out, scalar_t* buf, scalar_t* src[], index_t* idx[], scalar_t* w[]) {
        constexpr int i = 2 * (n - 1);
        constexpr int j = 2 * (n - 1) + 1;
        constexpr int is = i * step;
        constexpr int js = j * step;
        constexpr int half = 1 << (n - 2);
        Interp<n - 1, scalar_t, index_t, step>::eval(&buf[is], buf, &src[0], idx, &w[2]);        
        Interp<n - 1, scalar_t, index_t, step>::eval(&buf[js], buf, &src[half], idx, &w[2]); 
        compute_linear<scalar_t, step>(out, &buf[is], &buf[js], w[0][0], w[1][0]);
    }
};


template <typename scalar_t, typename index_t, int step>
struct Interp<1, scalar_t, index_t, step> {
    static inline void eval(scalar_t* out, scalar_t* buf, scalar_t* src[], index_t* idx[], scalar_t* w[]) {
      load<scalar_t, index_t, step>(&buf[0], src[0], idx[0]);
      load<scalar_t, index_t, step>(&buf[step], src[0], idx[1]);
      compute_linear<scalar_t, step>(out, &buf[0], &buf[step], w[0], w[1]);
    }
};


template <int n, typename scalar_t, typename index_t, int step>
static inline void interp(scalar_t* out, scalar_t* buf, scalar_t* src[], index_t* idx[], scalar_t* w[]) {
  Interp<n, scalar_t, index_t, step>::eval(out, buf, src, idx, w);
}


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


// Linear upsampling dispatched computation method using TensorIterator.
// 
// Assumptions:
// - input and output are of shape (B, C, D1, D2, D3, ..., DN) and
// - upsampling is computed on D_i axes.
// - zero strides for construced indices and weights on dims D1, D2, ..., DN-1
// - zero stride for input source (as it is restrided)
// - non-zero stride for indices and weights on DN dimension
// 
// Using these assumptions we iterate over DN dimension and compute the output 
// using the following tricks for optimizations:
// - indices are already containing strides
// - src pointer is advanced once by the constant offset for D1, D2, ..., DN-1
// - using buffers to prefetch src data before the computations
// 
// Single loop function for 1d, 2d and 3d cases.
// For N dimensions, output value up to Di dimension can be computed as
///
// output_i[a] = linear_interp(output_{i+1}[a], w_{i+1}[a], output_{i+1}[a+1], w_{i+1}[a+1])
// with
// output_DN[a] = linear_interp(input_DN[a], w_DN[a], input_DN[a+1], w_DN[a+1])
//
// This recursive call is implemented with Interp struct using template for 
// the loop unrolling on compile time.
// 
template <typename scalar_t, typename index_t, int out_ndims>
void ti_cpu_upsample_linear(at::TensorIterator& iter) {

  auto loop = [&](char** data, const int64_t* strides, int64_t n) {

    scalar_t * dst = (scalar_t *) data[0];
    scalar_t * src = (scalar_t *) data[1];

    assert_strides_linear<scalar_t, index_t, out_ndims>(strides);

    constexpr int step = 4;
    constexpr int p2_size = 1 << (out_ndims - 1);

    // temporary buffer for src values
    scalar_t buffer[p2_size * step];

    // placeholder for pointers on indices for iterated dimension (e.g. -1)
    index_t * idx_ptrs[2];
    // placeholder for pointers on all weights: w0 and w1 for each dimension
    scalar_t * weights_ptrs[2 * out_ndims];
    // placeholder src pointer with all possible constant offsets added
    scalar_t * src_offset[p2_size];
    {
      index_t * constval_idx_ptrs[2 * (out_ndims - 1)];
      int i = 0;
      for (; i<out_ndims - 1; i++) {
        constval_idx_ptrs[2 * i + 0] = (index_t *) data[4 * i + 0 + 2];
        weights_ptrs[2 * i + 0] = (scalar_t *) data[4 * i + 1 + 2];
        constval_idx_ptrs[2 * i + 1] = (index_t *) data[4 * i + 2 + 2];
        weights_ptrs[2 * i + 1] = (scalar_t *) data[4 * i + 3 + 2];
      }
      idx_ptrs[0] = (index_t *) data[4 * i + 0 + 2];
      weights_ptrs[2 * i + 0] = (scalar_t *) data[4 * i + 1 + 2];
      idx_ptrs[1] = (index_t *) data[4 * i + 2 + 2];
      weights_ptrs[2 * i + 1] = (scalar_t *) data[4 * i + 3 + 2];

      // Add all constant offsets to src
      int dim_idx = 0;
      for (int j=0; j<p2_size; j++) {
        src_offset[j] = src;
        for (int i=0; i<out_ndims - 1; i++) {
          dim_idx = (j >> (out_ndims - 2 - i)) % 2;
          src_offset[j] += *constval_idx_ptrs[2 * i + dim_idx];
        }
      }
    }
    
    int64_t i = 0;
    int64_t n_step = n - (n % step);
    for (; i < n_step; i += step) {
      interp<out_ndims, scalar_t, index_t, step>(dst + i, buffer, src_offset, idx_ptrs, weights_ptrs);
      // Here we advance only on the last dimension (i.e. dim -1)
      idx_ptrs[0] += step;
      idx_ptrs[1] += step;
      weights_ptrs[2 * (out_ndims - 1) + 0] += step;
      weights_ptrs[2 * (out_ndims - 1) + 1] += step;
    }    
    for (; i < n; i++) {            
      interp<out_ndims, scalar_t, index_t, 1>(dst + i, buffer, src_offset, idx_ptrs, weights_ptrs);
      idx_ptrs[0] += 1;
      idx_ptrs[1] += 1;
      weights_ptrs[2 * (out_ndims - 1) + 0] += 1;
      weights_ptrs[2 * (out_ndims - 1) + 1] += 1;
    }
    
  };

  iter.for_each(loop);
}

template<typename index_t, typename scalar_t>
std::vector<at::Tensor> ti_compute_indices_weights_linear(
  int64_t input_size, int64_t output_size, int64_t stride, int64_t ndims, int64_t reshape_dim, 
  bool align_corners, const c10::optional<double> opt_scale
) {

  scalar_t scale = at::native::area_pixel_compute_scale<scalar_t>(input_size, output_size, align_corners, opt_scale);

  std::vector<at::Tensor> output;
  auto new_shape = std::vector<int64_t>(ndims, 1);
  new_shape[reshape_dim] = output_size;

  output.emplace_back(at::empty(new_shape, at::CPU(c10::CppTypeToScalarType<index_t>())));
  output.emplace_back(at::empty(new_shape, at::CPU(c10::CppTypeToScalarType<scalar_t>())));  
  output.emplace_back(at::empty(new_shape, at::CPU(c10::CppTypeToScalarType<index_t>())));
  output.emplace_back(at::empty(new_shape, at::CPU(c10::CppTypeToScalarType<scalar_t>())));

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
    // put stride into indices
    // index values correspond to input indices (0, 1, 2, 3, ...)
    // when multiplied by input stride, maximum possible value
    // input_size[dim-1] * input_size[dim-2] * ... for the given dimension.
    input_index0_ptr[i] *= stride;
    input_index1_ptr[i] *= stride;
  }
  return output;
}

using at::native::upsample::compute_output_size;
using at::native::upsample::get_scale_value;


// Upsampling linear interpolation kernel for N-d case.
// Internally, it uses TensorIterator to optimize the computations.
// Output is constructed inside the function and is a contiguous tensor.
template <typename index_t, int out_ndims>
at::Tensor ti_upsample_linearNd_kernel_impl(
    const at::Tensor& input,
    c10::optional<at::IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  if (output_size) {
    TORCH_CHECK(out_ndims == output_size->size());
  }  
  if (scale_factors) {
    TORCH_CHECK(out_ndims == scale_factors->size());
  }  
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);

  // input can be NCHW, NCL or NCKHW
  auto shape = input.sizes().vec();
  auto strides = input.strides().vec();

  for (int i=0; i<out_ndims; i++) {
    shape[i + 2] = osize[i];
    strides[i + 2] = 0;
  }
  auto restrided_input = input.as_strided(shape, strides);

  // Compute indices and weights for each interpolated dimension
  // indices_weights = {
  //      {indices_0, weights_0, indices_1, weights_1},  // dim -n
  //      {indices_0, weights_0, indices_1, weights_1},  // dim -(n-1)
  //      ...
  //      {indices_0, weights_0, indices_1, weights_1},  // dim -1
  // }
  // Indices and weights are reshaped as (1, 1, ..., N, ..., 1, 1) to
  // fit input/output tensors.
  // Indices are already containing the strides to optimize the computations
  //
  // Indices dtype can be int32_t or int64_t depending on canUse32BitIndexMath(input)
  // which should not overflow because maximum possible value that it could take is the 
  // product of interpolated input strides: input_size[dim-1] * input_size[dim-2] * ...
  // which is always smaller then the number of input elements checked by canUse32BitIndexMath
  std::vector<std::vector<at::Tensor>> indices_weights;
  AT_DISPATCH_FLOATING_TYPES(
    input.scalar_type(), "compute_indices_weights_linear", [&] {
      for (int i=0; i<out_ndims; i++) {
        indices_weights.emplace_back(
          ti_compute_indices_weights_linear<index_t, scalar_t>(
            input.size(i + 2), osize[i], input.stride(i + 2), input.dim(), i + 2, align_corners, get_scale_value(scale_factors, i))
        );
      }
    }
  );

  at::TensorIteratorConfig config;
  config.check_all_same_dtype(false)
    .declare_static_dtype_and_device(input.scalar_type(), input.device())
    .add_output(at::Tensor())
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


at::Tensor ti_upsample_bilinear2d_kernel_impl(
    const at::Tensor& input,
    c10::optional<at::IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {
  if (at::native::canUse32BitIndexMath(input))
    return ti_upsample_linearNd_kernel_impl<int32_t, 2>(
      input, output_size, align_corners, scale_factors);
  
  return ti_upsample_linearNd_kernel_impl<int64_t, 2>(
    input, output_size, align_corners, scale_factors);
}


at::Tensor ti_upsample_linear1d_kernel_impl(
    const at::Tensor& input,
    c10::optional<at::IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {
  if (at::native::canUse32BitIndexMath(input))
    return ti_upsample_linearNd_kernel_impl<int32_t, 1>(
      input, output_size, align_corners, scale_factors);

  return ti_upsample_linearNd_kernel_impl<int64_t, 1>(
    input, output_size, align_corners, scale_factors);
}


at::Tensor ti_upsample_trilinear3d_kernel_impl(
    const at::Tensor& input,
    c10::optional<at::IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {
  if (at::native::canUse32BitIndexMath(input))
    return ti_upsample_linearNd_kernel_impl<int32_t, 3>(
      input, output_size, align_corners, scale_factors);

  return ti_upsample_linearNd_kernel_impl<int64_t, 3>(
    input, output_size, align_corners, scale_factors);
}
