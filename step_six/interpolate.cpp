#include <cmath>
#include <vector>
#include <ATen/TypeDefault.h>
#include <ATen/native/TensorIterator.h>
#include <ATen/native/IndexingUtils.h>
#include <ATen/native/UpSample.h>


#define USE_ALWAYS_INDEX64
// #define VERBOSE

namespace at {
namespace native {
namespace ti_upsample {

using scale_t = std::vector<c10::optional<double>>;

#ifdef VERBOSE
static int TI_BASIC_LOOP_CHANNELS_FIRST_TRIGGERED = 0;
static int TI_BASIC_LOOP_CHANNELS_LAST_TRIGGERED = 0;
static int TI_BASIC_LOOP_FALLBACK_TRIGGERED = 0;
static bool TI_SHOW_STRIDES = true;
#endif

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

template <int n, typename scalar_t, typename index_t>
struct InterpLinear {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
        index_t i0 = *(index_t*)&data[0][i * strides[0]];
        index_t i1 = *(index_t*)&data[2][i * strides[2]];
        scalar_t w0 = *(scalar_t *)&data[1][i * strides[1]];
        scalar_t w1 = *(scalar_t *)&data[3][i * strides[3]];

        scalar_t t0 = InterpLinear<n - 1, scalar_t, index_t>::eval(src + i0, &data[4], &strides[4], i);
        scalar_t t1 = InterpLinear<n - 1, scalar_t, index_t>::eval(src + i1, &data[4], &strides[4], i);

        return t0 * w0 + t1 * w1;
    }
};

template <typename scalar_t, typename index_t>
struct InterpLinear<1, scalar_t, index_t> {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
        index_t i0 = *(index_t*)&data[0][i * strides[0]];
        index_t i1 = *(index_t*)&data[2][i * strides[2]];
        scalar_t w0 = *(scalar_t *)&data[1][i * strides[1]];
        scalar_t w1 = *(scalar_t *)&data[3][i * strides[3]];
        scalar_t t0 = *(scalar_t *)&src[i0];
        scalar_t t1 = *(scalar_t *)&src[i1];
        return t0 * w0 + t1 * w1;
    }
};

template <int n, typename scalar_t, typename index_t>
static inline scalar_t interp_linear(char* src, char** data, const int64_t* strides, int64_t i) {
  return InterpLinear<n, scalar_t, index_t>::eval(src, data, strides, i);
}

static inline bool is_zero_stride(const int64_t* strides) {
  return (strides[0] == 0) && (strides[1] == 0) && (strides[2] == 0) && (strides[3] == 0);
}

template <typename scalar_t, typename index_t>
static inline bool is_contiguous_stride(const int64_t* strides) {
  return (strides[0] == sizeof(index_t)) && (strides[1] == sizeof(scalar_t)) &&
         (strides[2] == sizeof(index_t)) && (strides[3] == sizeof(scalar_t));
}

template <int N, int non_zero_stride_dim, typename scalar_t, typename index_t>
struct CheckAlmostAllZeroStrides {
  static inline bool eval(const int64_t* strides) {
    return (N == non_zero_stride_dim ? is_contiguous_stride<scalar_t, index_t>(strides) : is_zero_stride(strides)) &&
            CheckAlmostAllZeroStrides<N - 1, non_zero_stride_dim, scalar_t, index_t>::eval(&strides[4]);
  }
};

template <int non_zero_stride_dim, typename scalar_t, typename index_t>
struct CheckAlmostAllZeroStrides<0, non_zero_stride_dim, scalar_t, index_t> {
  static inline bool eval(const int64_t* strides) {
    return true;
  }
};

template <int n, int s, typename scalar_t, typename index_t>
static inline bool is_all_zero_stride(const int64_t* strides) {
  return CheckAlmostAllZeroStrides<n, s, scalar_t, index_t>::eval(strides);
}

template <typename scalar_t, typename index_t, int out_ndims>
static inline void basic_loop(char** data, const int64_t* strides, int64_t n) {
  char* dst = data[0];
  char* src = data[1];
  for (int64_t i = 0; i < n; i++) {
    *(scalar_t*)&dst[i * strides[0]] = interp_linear<out_ndims, scalar_t, index_t>(
        src + i * strides[1], &data[2], &strides[2], i);
  }
}

template <typename scalar_t, typename index_t, int out_ndims>
void ti_cpu_upsample_linear(at::TensorIterator& iter)
{
  auto loop = [&](char** data, const int64_t* strides, int64_t n) {

#ifdef VERBOSE
    if (TI_SHOW_STRIDES) {
      std::cout << "TI_SHOW: N=" << n << std::endl;
      std::cout << "TI_SHOW_STRIDES: "
        << strides[0] << " "
        << strides[1] << " ";
      for (int i=0; i<out_ndims; i++) {
        std::cout << strides[4 * i + 0 + 2] << " " << strides[4 * i + 1 + 2] << " ";
        std::cout << strides[4 * i + 2 + 2] << " " << strides[4 * i + 3 + 2] << " ";
      }
      std::cout << std::endl;
      TI_SHOW_STRIDES = false;
    }
#endif

    // special-cases to let the compiler apply compile-time input-specific optimizations
    if ((strides[0] == sizeof(scalar_t) && (strides[1] == 0) &&
        is_all_zero_stride<out_ndims, 1, scalar_t, index_t>(&strides[2]))) {
      // contiguous channels-first case
#ifdef VERBOSE
      if (TI_BASIC_LOOP_CHANNELS_FIRST_TRIGGERED < 1) {
        std::cout << "TI_BASIC_LOOP -> CHANNELS_FIRST" << std::endl << std::flush;
        TI_BASIC_LOOP_CHANNELS_FIRST_TRIGGERED += 1;
      }
#endif
      basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
    } else if ((strides[0] == sizeof(scalar_t) && (strides[1] == sizeof(scalar_t)) &&
               is_all_zero_stride<out_ndims, -1, scalar_t, index_t>(&strides[2]))) {
      // contiguous channels-last case
#ifdef VERBOSE
      if (TI_BASIC_LOOP_CHANNELS_LAST_TRIGGERED < 1) {
        std::cout << "TI_BASIC_LOOP -> CHANNELS_LAST" << std::endl << std::flush;
        TI_BASIC_LOOP_CHANNELS_LAST_TRIGGERED += 1;
      }
#endif
      basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
    } else {
      // fallback
#ifdef VERBOSE
      if (TI_BASIC_LOOP_FALLBACK_TRIGGERED < 1) {
        std::cout << "TI_BASIC_LOOP -> FALLBACK" << std::endl << std::flush;
        TI_BASIC_LOOP_FALLBACK_TRIGGERED += 1;
      }
#endif
      basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
    }
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

#ifdef VERBOSE
  TI_BASIC_LOOP_CHANNELS_FIRST_TRIGGERED = 0;
  TI_BASIC_LOOP_CHANNELS_LAST_TRIGGERED = 0;
  TI_BASIC_LOOP_FALLBACK_TRIGGERED = 0;
  TI_SHOW_STRIDES = true;

  std::cout << "\nInput tensor: " << input.sizes() << std::endl;
  std::cout << "Input is_contiguous memory_format torch.channels_last: " << (input.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
  std::cout << "Input is_contiguous memory_format torch.channels_last_3d: " << (input.is_contiguous(at::MemoryFormat::ChannelsLast3d) ? "true" : "false") << std::endl;
  std::cout << "Input is_contiguous : " << (input.is_contiguous() ? "true" : "false") << std::endl;

  std::cout << "\nOutput tensor: " << output.sizes() << std::endl;
  std::cout << "Output is_contiguous memory_format torch.channels_last: " << (output.is_contiguous(at::MemoryFormat::ChannelsLast) ? "true" : "false") << std::endl;
  std::cout << "Output is_contiguous memory_format torch.channels_last_3d: " << (output.is_contiguous(at::MemoryFormat::ChannelsLast3d) ? "true" : "false") << std::endl;
  std::cout << "Output is_contiguous : " << (output.is_contiguous() ? "true" : "false") << std::endl;
#endif

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
  std::vector<std::vector<Tensor>> indices_weights;
  AT_DISPATCH_FLOATING_TYPES(
    input.scalar_type(), "ti_compute_indices_weights_linear", [&] {
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

  for (auto & idx_weight: indices_weights) {
    for (auto& tensor : idx_weight) {
      config.add_input(tensor);
    }
  }

  auto iter = config.build();

  AT_DISPATCH_FLOATING_TYPES(
      iter.dtype(), "ti_upsample_linearNd", [&] {
      ti_cpu_upsample_linear<scalar_t, index_t, out_ndims>(iter);
  });

}


void _ti_upsample_bilinear2d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_h,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)) {
    ti_upsample_linearNd_kernel_impl<int32_t, 2, scale_t>(
      output, input, align_corners, {scales_h, scales_w});
  } else {
    ti_upsample_linearNd_kernel_impl<int64_t, 2, scale_t>(
      output, input, align_corners, {scales_h, scales_w});
  }
#else
  ti_upsample_linearNd_kernel_impl<int64_t, 2, scale_t>(
    output, input, align_corners, {scales_h, scales_w});
#endif
}


using at::native::upsample::compute_output_size;
using at::native::upsample::get_scale_value;


Tensor ti_upsample_bilinear2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors) {

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


void _ti_upsample_linear1d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)){
    ti_upsample_linearNd_kernel_impl<int32_t, 1, scale_t>(
      output, input, align_corners, {scales_w});
  } else {
    ti_upsample_linearNd_kernel_impl<int64_t, 1, scale_t>(
      output, input, align_corners, {scales_w});
  }
#else
  ti_upsample_linearNd_kernel_impl<int64_t, 1, scale_t>(
    output, input, align_corners, {scales_w});
#endif
}


Tensor ti_upsample_linear1d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors) {

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


void _ti_upsample_trilinear3d_kernel_impl(
    Tensor& output,
    const Tensor& input,
    bool align_corners,
    c10::optional<double> scales_d,
    c10::optional<double> scales_h,
    c10::optional<double> scales_w) {

#ifndef USE_ALWAYS_INDEX64
  if (canUse32BitIndexMath(input)) {
    ti_upsample_linearNd_kernel_impl<int32_t, 3, scale_t>(
      output, input, align_corners, {scales_d, scales_h, scales_w});
  } else {
    ti_upsample_linearNd_kernel_impl<int64_t, 3, scale_t>(
      output, input, align_corners, {scales_d, scales_h, scales_w});
  }
#else
  ti_upsample_linearNd_kernel_impl<int64_t, 3, scale_t>(
    output, input, align_corners, {scales_d, scales_h, scales_w});
#endif
}


Tensor ti_upsample_trilinear3d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors) {

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


} // namespace ti_upsample
} // namespace native
} // namespace at
