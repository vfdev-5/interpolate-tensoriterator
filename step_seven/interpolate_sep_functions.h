#pragma once

#include <cmath>
#include <vector>
#include <ATen/TypeDefault.h>
#include <ATen/native/TensorIterator.h>
#include <ATen/native/IndexingUtils.h>
#include <ATen/native/UpSample.h>


namespace at {
namespace native {
namespace {


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
    const scalar_t real_input_index = area_pixel_compute_source_index<scalar_t>(
        ratio, output_index, align_corners, /*cubic=*/false);
    input_index0 = static_cast<index_t>(real_input_index);
    index_t offset = (input_index0 < input_size - 1) ? 1 : 0;
    input_index1 = input_index0 + offset;
    lambda1 = real_input_index - static_cast<scalar_t>(input_index0);
    lambda0 = static_cast<scalar_t>(1.) - lambda1;
  }
}
// End of copied ...


template <int n, typename scalar_t, typename index_t, int interp_size>
struct Interpolate {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      constexpr int half_interp_size = int(interp_size / 2);

      index_t ids = *(index_t*)&data[0][i * strides[0]];
      scalar_t wts = *(scalar_t*)&data[1][i * strides[1]];
      scalar_t t = Interpolate<n - 1, scalar_t, index_t, interp_size>::eval(src + ids, &data[interp_size], &strides[interp_size], i);
      scalar_t output = t * wts;
      for (int j=1; j<half_interp_size; j++) {
        ids = *(index_t*)&data[2 * j + 0][i * strides[2 * j + 0]];
        wts = *(scalar_t*)&data[2 * j + 1][i * strides[2 * j + 1]];
        t = Interpolate<n - 1, scalar_t, index_t, interp_size>::eval(src + ids, &data[interp_size], &strides[interp_size], i);
        output += t * wts;
      }
      return output;
  }
};


template <typename scalar_t, typename index_t, int interp_size>
struct Interpolate<1, scalar_t, index_t, interp_size> {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      constexpr int half_interp_size = int(interp_size / 2);
      index_t ids = *(index_t*)&data[0][i * strides[0]];
      scalar_t wts = *(scalar_t*)&data[1][i * strides[1]];
      scalar_t t = *(scalar_t *)&src[ids];
      scalar_t output = t * wts;
      for (int j=1; j<half_interp_size; j++) {
        ids = *(index_t*)&data[2 * j + 0][i * strides[2 * j + 0]];
        wts = *(scalar_t*)&data[2 * j + 1][i * strides[2 * j + 1]];
        t = *(scalar_t *)&src[ids];
        output += t * wts;
      }
      return output;
    }
};


template <int n, typename scalar_t, typename index_t>
struct Interpolate<n, scalar_t, index_t, 2> {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      index_t ids = *(index_t*)&data[0][i * strides[0]];
      return Interpolate<n - 1, scalar_t, index_t, 2>::eval(src + ids, &data[2], &strides[2], i);
  }
};


template <typename scalar_t, typename index_t>
struct Interpolate<1, scalar_t, index_t, 2> {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      index_t ids = *(index_t*)&data[0][i * strides[0]];
      return *(scalar_t *)&src[ids];
    }
};

template <int n, typename scalar_t, typename index_t, int interp_size>
static inline scalar_t interpolate(char* src, char** data, const int64_t* strides, int64_t i) {
  return Interpolate<n, scalar_t, index_t, interp_size>::eval(src, data, strides, i);
}

template<int interp_size>
static inline bool is_zero_stride(const int64_t* strides) {
  bool output = strides[0] == 0;
  for (int i=1; i<interp_size; i++) {
    output &= (strides[i] == 0);
  }
  return output;
}

template <typename scalar_t, typename index_t, int interp_size>
static inline bool is_contiguous_stride(const int64_t* strides) {
  bool output = (strides[0] == sizeof(index_t)) && (strides[1] == sizeof(scalar_t));
  for (int i=2; i<interp_size; i+=2) {
    output &= (strides[i] == sizeof(index_t)) && (strides[i + 1] == sizeof(scalar_t));
  }
  return output;
}

template <int N, int non_zero_stride_dim, typename scalar_t, typename index_t, int interp_size>
struct CheckAlmostAllZeroStrides {
  static inline bool eval(const int64_t* strides) {
    bool output;
    if (N == non_zero_stride_dim) {
      output = is_contiguous_stride<scalar_t, index_t, interp_size>(strides);
    } else {
      output = is_zero_stride<interp_size>(strides);
    }    
    return output && 
      CheckAlmostAllZeroStrides<N - 1, non_zero_stride_dim, scalar_t, index_t, interp_size>::eval(
        &strides[interp_size]);
  }
};

template <int non_zero_stride_dim, typename scalar_t, typename index_t, int interp_size>
struct CheckAlmostAllZeroStrides<0, non_zero_stride_dim, scalar_t, index_t, interp_size> {
  static inline bool eval(const int64_t* strides) {
    return true;
  }
};

template <int n, int s, typename scalar_t, typename index_t, int interp_size>
static inline bool check_almost_all_zero_stride(const int64_t* strides) {
  return CheckAlmostAllZeroStrides<n, s, scalar_t, index_t, interp_size>::eval(strides);
}

template <typename scalar_t, typename index_t, int out_ndims, int interp_size>
static inline void basic_loop(char** data, const int64_t* strides, int64_t n) {
  char* dst = data[0];
  char* src = data[1];
  for (int64_t i = 0; i < n; i++) {
    *(scalar_t*)&dst[i * strides[0]] = interpolate<out_ndims, scalar_t, index_t, interp_size>(
        src + i * strides[1], &data[2], &strides[2], i);
  }
}


template <typename scalar_t, typename index_t, int out_ndims, int interp_size>
void ti_cpu_upsample_generic(at::TensorIterator& iter)
{
  auto loop = [&](char** data, const int64_t* strides, int64_t n) {
    // special-cases to let the compiler apply compile-time input-specific optimizations
    if ((strides[0] == sizeof(scalar_t) && (strides[1] == 0) &&
        check_almost_all_zero_stride<out_ndims, 1, scalar_t, index_t, interp_size>(&strides[2]))) {
      // contiguous channels-first case
      basic_loop<scalar_t, index_t, out_ndims, interp_size>(data, strides, n);
    } else if ((strides[0] == sizeof(scalar_t) && (strides[1] == sizeof(scalar_t)) &&
               check_almost_all_zero_stride<out_ndims, -1, scalar_t, index_t, interp_size>(&strides[2]))) {
      // contiguous channels-last case
      basic_loop<scalar_t, index_t, out_ndims, interp_size>(data, strides, n);
    } else {
      // fallback
      basic_loop<scalar_t, index_t, out_ndims, interp_size>(data, strides, n);
    }
  };
  iter.for_each(loop);
}


// Compute indices and weights for each interpolated dimension
// indices_weights = {
//      {indices_0, 1.0, },  // dim -n
//      {indices_0, 1.0, },  // dim -(n-1)
//      ...
//      {indices_0, 1.0, },  // dim -1
// }
// Indices and weights are reshaped as (1, 1, ..., N, ..., 1, 1) to
// fit input/output tensors.
// Indices are already containing the strides to optimize the computations
//
// Indices dtype can be int32_t or int64_t depending on canUse32BitIndexMath(input)
// which should not overflow because maximum possible value that it could take is the 
// product of interpolated input strides: input_size[dim-1] * input_size[dim-2] * ...
// which is always smaller then the number of input elements checked by canUse32BitIndexMath
template<typename index_t, typename scalar_t>
std::vector<Tensor> ti_compute_indices_weights_nearest(
  int64_t input_size, int64_t output_size, int64_t stride, int64_t ndims, int64_t reshape_dim, 
  bool align_corners, const c10::optional<double> opt_scale
) {

  scalar_t scale = area_pixel_compute_scale<scalar_t>(input_size, output_size, align_corners, opt_scale);

  std::vector<Tensor> output;
  auto new_shape = std::vector<int64_t>(ndims, 1);
  new_shape[reshape_dim] = output_size;

  output.emplace_back(empty(new_shape, CPU(c10::CppTypeToScalarType<index_t>())));
  // Defines weights for consistency, but not used
  output.emplace_back(at::ones({1, }, CPU(c10::CppTypeToScalarType<scalar_t>())));

  auto input_index_ptr = output[0].data_ptr<index_t>();
  
  int64_t input_index;

  for (int64_t i=0; i<output_size; i++) {
    const scalar_t real_input_index = area_pixel_compute_source_index<scalar_t>(
        scale, i, /*align_corners=*/true, /*cubic=*/false);
    input_index = static_cast<int64_t>(floorf(real_input_index));
    input_index_ptr[i] = static_cast<index_t>(std::min(input_index, input_size - 1)) * stride;
  }
  return output;
}


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


// Compute indices and weights for each interpolated dimension
// indices_weights = {
//      {indices_0, weights_0, indices_1, weights_1, ..., indices_3, weights_3},  // dim -n
//      {indices_0, weights_0, indices_1, weights_1, ..., indices_3, weights_3},  // dim -(n-1)
//      ...
//      {indices_0, weights_0, indices_1, weights_1, ..., indices_3, weights_3},  // dim -1
// }
// Indices and weights are reshaped as (1, 1, ..., N, ..., 1, 1) to
// fit input/output tensors.
// Indices are already containing the strides to optimize the computations
//
// Indices dtype can be int32_t or int64_t depending on canUse32BitIndexMath(input)
// which should not overflow because maximum possible value that it could take is the 
// product of interpolated input strides: input_size[dim-1] * input_size[dim-2] * ...
// which is always smaller then the number of input elements checked by canUse32BitIndexMath
template<typename index_t, typename scalar_t>
std::vector<Tensor> ti_compute_indices_weights_cubic(
  int64_t input_size, int64_t output_size, int64_t stride, int64_t ndims, int64_t reshape_dim, 
  bool align_corners, const c10::optional<double> opt_scale
) {

  scalar_t scale = area_pixel_compute_scale<scalar_t>(input_size, output_size, align_corners, opt_scale);

  std::vector<Tensor> output;
  auto new_shape = std::vector<int64_t>(ndims, 1);
  new_shape[reshape_dim] = output_size;

  constexpr int interp_size = 8;
  constexpr int half_interp_size = int(interp_size / 2);
  for (int j=0; j<half_interp_size; j++) {
    output.emplace_back(empty(new_shape, CPU(c10::CppTypeToScalarType<index_t>())));
    output.emplace_back(empty(new_shape, CPU(c10::CppTypeToScalarType<scalar_t>())));
  }

  int64_t input_index;
  int64_t zero = static_cast<int64_t>(0);
  scalar_t coeffs[4];

  index_t * idx_ptr;
  scalar_t * wt_ptr;

  for (int64_t i=0; i<output_size; i++) {

    const scalar_t real_input_index = area_pixel_compute_source_index<scalar_t>(
        scale, i, align_corners, /*cubic=*/true);
    input_index = static_cast<int64_t>(floorf(real_input_index));
    get_cubic_upsample_coefficients<scalar_t>(coeffs, real_input_index - input_index);

    for (int j=0; j<half_interp_size; j++) {
      idx_ptr = output[2 * j + 0].data_ptr<index_t>();
      idx_ptr[i] = static_cast<index_t>(std::max(std::min(input_index + j - 1, input_size - 1), zero)) * stride;
      wt_ptr = output[2 * j + 1].data_ptr<scalar_t>();
      wt_ptr[i] = coeffs[j];
    }
  }
  return output;
}


// Upsampling linear interpolation kernel for N-d case.
// Internally, it uses TensorIterator to optimize the computations.
// Output is constructed inside the function and is a contiguous tensor.
// - index_t is template type for input index: int32_t or int64_t
// - out_ndims is the number of interpolated dims: 1, 2, 3
// - scale_type is template type for scales, typically c10::optional<double>
template <typename index_t, int out_ndims, typename scale_type>
void ti_upsample_linearNd_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    const scale_type& scales) {

  // input can be NCHW, NCL or NCKHW
  auto shape = input.sizes().vec();
  auto strides = input.strides().vec();
  auto oshape = output.sizes();

  TORCH_INTERNAL_ASSERT(
    shape.size() == oshape.size() && shape.size() == 2 + out_ndims
  );
  TORCH_INTERNAL_ASSERT(strides.size() == 2 + out_ndims);

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
            input.size(i + 2), oshape[i + 2], input.stride(i + 2) * input.element_size(), input.dim(), i + 2, align_corners, scales[i])
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
      ti_cpu_upsample_generic<scalar_t, index_t, out_ndims, 4>(iter);
  });
}


// ADAPTED COPY OF ABOVE METHOD
// TODO: TO REFACTOR LATER
template <typename index_t, int out_ndims, typename scale_type>
void ti_upsample_cubicNd_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    const scale_type& scales) {

  // input can be NCHW, NCL or NCKHW
  auto shape = input.sizes().vec();
  auto strides = input.strides().vec();
  auto oshape = output.sizes();

  TORCH_INTERNAL_ASSERT(
    shape.size() == oshape.size() && shape.size() == 2 + out_ndims
  );
  TORCH_INTERNAL_ASSERT(strides.size() == 2 + out_ndims);

  for (int i=0; i<out_ndims; i++) {
    shape[i + 2] = oshape[i + 2];
    strides[i + 2] = 0;
  }
  auto restrided_input = input.as_strided(shape, strides);

  std::vector<std::vector<Tensor>> indices_weights;
  AT_DISPATCH_FLOATING_TYPES(
    input.scalar_type(), "compute_indices_weights_cubic", [&] {
      for (int i=0; i<out_ndims; i++) {
        indices_weights.emplace_back(
          ti_compute_indices_weights_cubic<index_t, scalar_t>(
            input.size(i + 2), oshape[i + 2], input.stride(i + 2) * input.element_size(), input.dim(), i + 2, align_corners, scales[i])
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
      iter.dtype(), "upsample_cubicNd", [&] {
      ti_cpu_upsample_generic<scalar_t, index_t, out_ndims, 8>(iter);
  });
}


template <typename index_t, int out_ndims, typename scale_type>
void ti_upsample_nearestNd_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    const scale_type& scales) {

  // input can be NCHW, NCL or NCKHW
  auto shape = input.sizes().vec();
  auto strides = input.strides().vec();
  auto oshape = output.sizes();

  TORCH_INTERNAL_ASSERT(
    shape.size() == oshape.size() && shape.size() == 2 + out_ndims
  );
  TORCH_INTERNAL_ASSERT(strides.size() == 2 + out_ndims);

  for (int i=0; i<out_ndims; i++) {
    shape[i + 2] = oshape[i + 2];
    strides[i + 2] = 0;
  }
  auto restrided_input = input.as_strided(shape, strides);

  // Compute indices and weights for each interpolated dimension
  // indices_weights = {
  //      {indices_0, 1.0, },  // dim -n
  //      {indices_0, 1.0, },  // dim -(n-1)
  //      ...
  //      {indices_0, 1.0, },  // dim -1
  // }
  // Indices and weights are reshaped as (1, 1, ..., N, ..., 1, 1) to
  // fit input/output tensors.
  // Indices are already containing the strides to optimize the computations
  //
  // Indices dtype can be int32_t or int64_t depending on canUse32BitIndexMath(input)
  // which should not overflow because maximum possible value that it could take is the 
  // product of interpolated input strides: input_size[dim-1] * input_size[dim-2] * ...
  // which is always smaller then the number of input elements checked by canUse32BitIndexMath
  std::vector<std::vector<Tensor>> indices_weights;
  AT_DISPATCH_FLOATING_TYPES(
    input.scalar_type(), "compute_indices_weights_nearest", [&] {
      for (int i=0; i<out_ndims; i++) {
        indices_weights.emplace_back(
          ti_compute_indices_weights_nearest<index_t, scalar_t>(
            input.size(i + 2), oshape[i + 2], input.stride(i + 2) * input.element_size(), input.dim(), i + 2, align_corners, scales[i])
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
      iter.dtype(), "upsample_nearestNd", [&] {
      ti_cpu_upsample_generic<scalar_t, index_t, out_ndims, 2>(iter);
  });
}


// Helper structs to use with ti_upsample_generic_Nd_kernel_impl
template<typename index_t, typename scalar_t>
struct HelperInterpNearest {

  static const int interp_size = 2;

  static inline std::vector<Tensor> compute_indices_weights(
    int64_t input_size, int64_t output_size, int64_t stride, int64_t ndims, int64_t reshape_dim, 
    bool align_corners, const c10::optional<double> opt_scale) {
      return ti_compute_indices_weights_nearest<index_t, scalar_t>(
        input_size, output_size, stride, ndims, reshape_dim, align_corners, opt_scale);
  }

};


template<typename index_t, typename scalar_t>
struct HelperInterpLinear {

  static const int interp_size = 4;

  static inline std::vector<Tensor> compute_indices_weights(
    int64_t input_size, int64_t output_size, int64_t stride, int64_t ndims, int64_t reshape_dim, 
    bool align_corners, const c10::optional<double> opt_scale) {
      return ti_compute_indices_weights_linear<index_t, scalar_t>(
        input_size, output_size, stride, ndims, reshape_dim, align_corners, opt_scale);
  }

};


template<typename index_t, typename scalar_t>
struct HelperInterpCubic {

  static const int interp_size = 8;

  static inline std::vector<Tensor> compute_indices_weights(
    int64_t input_size, int64_t output_size, int64_t stride, int64_t ndims, int64_t reshape_dim, 
    bool align_corners, const c10::optional<double> opt_scale) {
      return ti_compute_indices_weights_cubic<index_t, scalar_t>(
        input_size, output_size, stride, ndims, reshape_dim, align_corners, opt_scale);
  }

};

template <typename index_t, int out_ndims, typename scale_type, template<typename, typename> class F>
void ti_upsample_generic_Nd_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    const scale_type& scales) {

  // input can be NCHW, NCL or NCKHW  
  auto shape = input.sizes().vec();
  auto strides = input.strides().vec();
  auto oshape = output.sizes();

  TORCH_INTERNAL_ASSERT(
    shape.size() == oshape.size() && shape.size() == 2 + out_ndims
  );
  TORCH_INTERNAL_ASSERT(strides.size() == 2 + out_ndims);

  for (int i=0; i<out_ndims; i++) {
    shape[i + 2] = oshape[i + 2];
    strides[i + 2] = 0;
  }
  auto restrided_input = input.as_strided(shape, strides);

  std::vector<std::vector<Tensor>> indices_weights;

  AT_DISPATCH_FLOATING_TYPES(
    input.scalar_type(), "compute_indices_weights_generic", [&] {
      for (int i=0; i<out_ndims; i++) {
        indices_weights.emplace_back(
          F<index_t, scalar_t>::compute_indices_weights(
            input.size(i + 2), oshape[i + 2], 
            input.stride(i + 2) * input.element_size(), 
            input.dim(), i + 2, align_corners, scales[i]
          )
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
      iter.dtype(), "upsample_generic_Nd", [&] {
      constexpr int interp_size = F<index_t, scalar_t>::interp_size;
      ti_cpu_upsample_generic<scalar_t, index_t, out_ndims, interp_size>(iter);
  });
}


using at::native::upsample::compute_output_size;
using at::native::upsample::get_scale_value;
using scale_t = std::vector<c10::optional<double>>;

// Below code is a C++ API for this main.cpp

void _ti_upsample_nearest2d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_h,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)) {
    ti_upsample_generic_Nd_kernel_impl<int32_t, 2, scale_t, HelperInterpNearest>(
    // ti_upsample_nearestNd_kernel_impl<int32_t, 2, scale_t>(
      output, input, align_corners, {scales_h, scales_w}
    );
  } else {
    ti_upsample_generic_Nd_kernel_impl<int64_t, 2, scale_t, HelperInterpNearest>(
    // ti_upsample_nearestNd_kernel_impl<int64_t, 2, scale_t>(
      output, input, align_corners, {scales_h, scales_w}
    );
  }
#else
  ti_upsample_generic_Nd_kernel_impl<int64_t, 2, scale_t, HelperInterpNearest>(
    output, input, align_corners, {scales_h, scales_w});
#endif
}


Tensor ti_upsample_nearest2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  // UpSampleNearest2d.cpp
  auto output = at::empty({0}, input.options());
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);
  auto scale_h = get_scale_value(scale_factors, 0);
  auto scale_w = get_scale_value(scale_factors, 1);

  auto full_output_size = native::upsample_2d_common_check(input.sizes(), osize);

  // Allow for empty batch size but not other dimensions
  TORCH_CHECK(
      input.numel() != 0 || c10::multiply_integers(input.sizes().begin() + 1, input.sizes().end()),
      "Non-empty 4D data tensor expected but got a tensor with sizes ",
      input.sizes());

  output.resize_(full_output_size, input.suggest_memory_format());
  _ti_upsample_nearest2d_kernel_impl(output, input, false, scale_h, scale_w);
  return output;
}


void _ti_upsample_bilinear2d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_h,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)) {
    ti_upsample_generic_Nd_kernel_impl<int32_t, 2, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_h, scales_w});
  } else {
    ti_upsample_generic_Nd_kernel_impl<int64_t, 2, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_h, scales_w});
  }
#else
  ti_upsample_generic_Nd_kernel_impl<int64_t, 2, scale_t, HelperInterpLinear>(
    output, input, align_corners, {scales_h, scales_w});
#endif
}


Tensor ti_upsample_bilinear2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  // UpSampleBilinear2d.cpp
  auto output = at::empty({0}, input.options());
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);
  auto scale_h = get_scale_value(scale_factors, 0);
  auto scale_w = get_scale_value(scale_factors, 1);

  auto full_output_size = native::upsample_2d_common_check(input.sizes(), osize);

  // Allow for empty batch size but not other dimensions
  TORCH_CHECK(
      input.numel() != 0 || c10::multiply_integers(input.sizes().begin() + 1, input.sizes().end()),
      "Non-empty 4D data tensor expected but got a tensor with sizes ",
      input.sizes());

  output.resize_(full_output_size, input.suggest_memory_format());
  _ti_upsample_bilinear2d_kernel_impl(output, input, align_corners, scale_h, scale_w);
  return output;
}


void _ti_upsample_bicubic2d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_h,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)) {
    ti_upsample_generic_Nd_kernel_impl<int32_t, 2, scale_t, HelperInterpCubic>(
      output, input, align_corners, {scales_h, scales_w});
  } else {
    ti_upsample_generic_Nd_kernel_impl<int64_t, 2, scale_t, HelperInterpCubic>(
      output, input, align_corners, {scales_h, scales_w});
  }
#else
  ti_upsample_generic_Nd_kernel_impl<int64_t, 2, scale_t, HelperInterpCubic>(
    output, input, align_corners, {scales_h, scales_w});
#endif
}



Tensor ti_upsample_bicubic2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  // UpSampleBicubic2d.cpp:upsample_bicubic2d_cpu
  auto output = at::empty({0}, input.options());
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);
  auto scale_h = get_scale_value(scale_factors, 0);
  auto scale_w = get_scale_value(scale_factors, 1);

  auto full_output_size = native::upsample_2d_common_check(input.sizes(), osize);
  // Allow for empty batch size but not other dimensions
  TORCH_CHECK(
      input.numel() != 0 || c10::multiply_integers(input.sizes().begin() + 1, input.sizes().end()),
      "Non-empty 4D data tensor expected but got a tensor with sizes ",
      input.sizes());

  // Special case: input/output same size, just copy
  // See upsample_bicubic2d_out_frame
  auto input_height = input.sizes()[2];
  auto input_width = input.sizes()[3];
  if (input_height == full_output_size[2] && input_width == full_output_size[3]) {
    return input.clone();
  }

  output.resize_(full_output_size, input.suggest_memory_format());
  _ti_upsample_bicubic2d_kernel_impl(output, input, align_corners, scale_h, scale_w);
  return output;
}


void _ti_upsample_nearest1d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)){
    ti_upsample_generic_Nd_kernel_impl<int32_t, 1, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_w});
  } else {
    ti_upsample_generic_Nd_kernel_impl<int64_t, 1, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_w});
  }
#else
  ti_upsample_generic_Nd_kernel_impl<int64_t, 1, scale_t, HelperInterpLinear>(
    output, input, align_corners, {scales_w});
#endif
}


Tensor ti_upsample_nearest1d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  // UpSampleNearest1d.cpp
  auto output = at::empty({0}, input.options());
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);
  auto scale_w = get_scale_value(scale_factors, 0);

  auto full_output_size = native::upsample_1d_common_check(input.sizes(), osize);

  // Allow for empty batch size but not other dimensions
  TORCH_CHECK(
      (input.size(1) != 0 && input.size(2) != 0) && input.dim() == 3,
      "Non-empty 3D data tensor expected but got a tensor with sizes ",
      input.sizes());

  output.resize_(full_output_size);
  _ti_upsample_nearest1d_kernel_impl(output, input, false, scale_w);
  return output;
}


void _ti_upsample_linear1d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)){
    ti_upsample_generic_Nd_kernel_impl<int32_t, 1, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_w});
  } else {
    ti_upsample_generic_Nd_kernel_impl<int64_t, 1, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_w});
  }
#else
  ti_upsample_generic_Nd_kernel_impl<int64_t, 1, scale_t, HelperInterpLinear>(
    output, input, align_corners, {scales_w});
#endif
}


Tensor ti_upsample_linear1d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  // UpSampleLinear1d.cpp
  auto output = at::empty({0}, input.options());
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);
  auto scale_w = get_scale_value(scale_factors, 0);

  auto full_output_size = native::upsample_1d_common_check(input.sizes(), osize);

  // Allow for empty batch size but not other dimensions
  TORCH_CHECK(
      (input.size(1) != 0 && input.size(2) != 0) && input.dim() == 3,
      "Non-empty 3D data tensor expected but got a tensor with sizes ",
      input.sizes());

  output.resize_(full_output_size);
  _ti_upsample_linear1d_kernel_impl(output, input, align_corners, scale_w);
  return output;
}


void _ti_upsample_nearest3d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_d,
    c10::optional<double> scales_h,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)) {
    ti_upsample_generic_Nd_kernel_impl<int32_t, 3, scale_t, HelperInterpNearest>(
      output, input, align_corners, {scales_d, scales_h, scales_w});
  } else {
    ti_upsample_generic_Nd_kernel_impl<int64_t, 3, scale_t, HelperInterpNearest>(
      output, input, align_corners, {scales_d, scales_h, scales_w});
  }
#else
  ti_upsample_generic_Nd_kernel_impl<int64_t, 3, scale_t, HelperInterpNearest>(
    output, input, align_corners, {scales_d, scales_h, scales_w});
#endif
}


Tensor ti_upsample_nearest3d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  // UpSampleNearest3d.cpp
  auto output = at::empty({0}, input.options());
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);
  auto scale_d = get_scale_value(scale_factors, 0);
  auto scale_h = get_scale_value(scale_factors, 1);
  auto scale_w = get_scale_value(scale_factors, 2);

  auto full_output_size = native::upsample_3d_common_check(input.sizes(), osize);

  // Allow for empty batch size but not other dimensions
  TORCH_CHECK(
      input.numel() != 0 || c10::multiply_integers(input.sizes().begin() + 1, input.sizes().end()),
      "Non-empty 5D data tensor expected but got a tensor with sizes ",
      input.sizes());

  output.resize_(full_output_size, input.suggest_memory_format());
  _ti_upsample_nearest3d_kernel_impl(output, input, false, scale_d, scale_h, scale_w);
  return output;
}


void _ti_upsample_trilinear3d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_d,
    c10::optional<double> scales_h,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)) {
    ti_upsample_generic_Nd_kernel_impl<int32_t, 3, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_d, scales_h, scales_w});
  } else {
    ti_upsample_generic_Nd_kernel_impl<int64_t, 3, scale_t, HelperInterpLinear>(
      output, input, align_corners, {scales_d, scales_h, scales_w});
  }
#else
  ti_upsample_generic_Nd_kernel_impl<int64_t, 3, scale_t, HelperInterpLinear>(
    output, input, align_corners, {scales_d, scales_h, scales_w});
#endif
}


Tensor ti_upsample_trilinear3d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt) {

  // UpSampleTrilinear3d.cpp
  auto output = at::empty({0}, input.options());
  auto osize = compute_output_size(input.sizes(), output_size, scale_factors);
  auto scale_d = get_scale_value(scale_factors, 0);
  auto scale_h = get_scale_value(scale_factors, 1);
  auto scale_w = get_scale_value(scale_factors, 2);

  auto full_output_size = native::upsample_3d_common_check(input.sizes(), osize);

  // Allow for empty batch size but not other dimensions
  TORCH_CHECK(
      input.numel() != 0 || c10::multiply_integers(input.sizes().begin() + 1, input.sizes().end()),
      "Non-empty 5D data tensor expected but got a tensor with sizes ",
      input.sizes());

  output.resize_(full_output_size, input.suggest_memory_format());
  _ti_upsample_trilinear3d_kernel_impl(output, input, align_corners, scale_d, scale_h, scale_w);
  return output;
}


} // anonymous namespace
} // namespace native
} // namespace at

