#include <math.h>
#include <vector>
#include <ATen/native/TensorIterator.h>
#include <ATen/native/IndexingUtils.h>

// using index_t = int32_t;
// using index_t = int64_t;


template <typename scalar_t, typename index_t, int step>
inline void load(scalar_t* dst, scalar_t *src, index_t * index) {
  for (int k = 0; k < step; k++) {
    *dst = *(src + *index);
    dst++;
    index++;
  }
}


template <typename scalar_t, int step>
inline void compute_linear(scalar_t* dst, scalar_t * src1, scalar_t* src2, float* w0, float* w1) {
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
inline void compute_linear(scalar_t* dst, scalar_t * src1, scalar_t* src2, float w0, float w1) {
  for (int k = 0; k < step; k++) {
    *dst = *src1 * w0 + *src2 * w1;
    dst++;
    src1++;
    src2++;
  }
}


template <int n, typename scalar_t, typename index_t, int step>
struct Interp {
    static inline void eval(scalar_t* out, scalar_t* buf, scalar_t* src[], index_t* idx[], float* w[]) {
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
    static inline void eval(scalar_t* out, scalar_t* buf, scalar_t* src[], index_t* idx[], float* w[]) {
      load<scalar_t, index_t, step>(&buf[0], src[0], idx[0]);
      load<scalar_t, index_t, step>(&buf[step], src[0], idx[1]);
      compute_linear<scalar_t, step>(out, &buf[0], &buf[step], w[0], w[1]);
    }
};


template <int n, typename scalar_t, typename index_t, int step>
static inline void interp(scalar_t* out, scalar_t* buf, scalar_t* src[], index_t* idx[], float* w[]) {
  Interp<n, scalar_t, index_t, step>::eval(out, buf, src, idx, w);
}


template <typename scalar_t, typename index_t, int out_ndims>
inline void assert_strides(const int64_t* strides) {
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
  TORCH_INTERNAL_ASSERT(strides[4 * i + 1 + 2] == sizeof(float), strides[4 * i + 1 + 2], sizeof(float));  
}


template <typename scalar_t, typename index_t, int out_ndims>
void ti_cpu_upsample_linear(at::TensorIterator& iter) {

  auto loop = [&](char** data, const int64_t* strides, int64_t n) {

    scalar_t * dst = (scalar_t *) data[0];
    scalar_t * src = (scalar_t *) data[1];

    assert_strides<scalar_t, index_t, out_ndims>(strides);

    constexpr int step = 4;
    constexpr int p2_size = 1 << (out_ndims - 1);

    // temporary buffer for src values
    scalar_t buffer[p2_size * step];

    // placeholders for pointers on indices for iterated dimension (e.g. -1)
    index_t * idx_ptrs[2];
    float * weights_ptrs[2 * out_ndims];
    scalar_t * src_offset[p2_size];
    {
      index_t * constval_idx_ptrs[2 * (out_ndims - 1)];
      int i = 0;
      for (; i<out_ndims - 1; i++) {
        constval_idx_ptrs[2 * i + 0] = (index_t *) data[4 * i + 0 + 2];
        weights_ptrs[2 * i + 0] = (float *) data[4 * i + 1 + 2];
        constval_idx_ptrs[2 * i + 1] = (index_t *) data[4 * i + 2 + 2];
        weights_ptrs[2 * i + 1] = (float *) data[4 * i + 3 + 2];
      }
      idx_ptrs[0] = (index_t *) data[4 * i + 0 + 2];
      weights_ptrs[2 * i + 0] = (float *) data[4 * i + 1 + 2];
      idx_ptrs[1] = (index_t *) data[4 * i + 2 + 2];
      weights_ptrs[2 * i + 1] = (float *) data[4 * i + 3 + 2];

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
    
    index_t i = 0;
    for (; i < n - (n % step); i += step) {      
      interp<out_ndims, scalar_t, index_t, step>(dst + i, buffer, src_offset, idx_ptrs, weights_ptrs);
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

  // auto loop = [&](char** data, const int64_t* strides, int64_t n) {

  //   scalar_t * dst = (scalar_t *) data[0];
  //   scalar_t * src = (scalar_t *) data[1];

  //   index_t * i0[out_ndims], * i1[out_ndims];
  //   float * w0[out_ndims], * w1[out_ndims];

  //   for (int i=0; i<out_ndims; i++) {
  //     i0[i] = (index_t *) data[4 * i + 0 + 2];
  //     w0[i] = (float *) data[4 * i + 1 + 2];
  //     i1[i] = (index_t *) data[4 * i + 2 + 2];
  //     w1[i] = (float *) data[4 * i + 3 + 2];
  //   }
  //   assert_strides<scalar_t, index_t, out_ndims>(strides);

  //   constexpr int step = 4;
  //   scalar_t buffer[2 * out_ndims][step];
  //   scalar_t buffer2[2 * out_ndims][1];
  //   index_t offset0[out_ndims];
  //   index_t offset1[out_ndims];
  //   float wv0[out_ndims], wv1[out_ndims];
  //   for (int i=0; i<out_ndims - 1; i++) {
  //     offset0[i] = *i0[i];
  //     offset1[i] = *i1[i];
  //     wv0[i] = *w0[i];
  //     wv1[i] = *w1[i];
  //   }

  //   // Add all constant offsets
  //   scalar_t * src_[4];
  //   src_[0] = src + offset0[0] + offset0[1];
  //   src_[1] = src + offset0[0] + offset1[1];
  //   src_[2] = src + offset1[0] + offset0[1];
  //   src_[3] = src + offset1[0] + offset1[1];

  //   index_t * idx[2];
  //   float * w[2 * out_ndims];
  //   for (int i = 0; i < out_ndims; i ++) {
  //     w[2 * i] = w0[i];
  //     w[2 * i + 1] = w1[i];
  //   }

  //   index_t i = 0;
  //   for (; i < n - (n % step); i += step) {
  //     idx[0] = i0[2] + i; idx[1] = i1[2] + i;
  //     w[2 * (out_ndims - 1)] = w0[2] + i; w[2 * (out_ndims - 1) + 1] = w1[2] + i;
  //     interp<out_ndims, scalar_t, index_t, step>(dst + i, buffer, src_, idx, w);
  //   }
  //   for (; i < n; i++) {
  //     idx[0] = i0[2] + i; idx[1] = i1[2] + i;
  //     w[2 * (out_ndims - 1)] = w0[2] + i; w[2 * (out_ndims - 1) + 1] = w1[2] + i;
  //     interp<out_ndims, scalar_t, index_t, 1>(dst + i, buffer2, src_, idx, w);
  //   }
  // };

  iter.for_each(loop);
}

template<typename index_t>
void ti_compute_indices_weights_faster(
  int64_t input_size, int64_t output_size, int64_t stride, at::Tensor & input_index0, at::Tensor & input_index1, at::Tensor & lambda0, at::Tensor & lambda1
) {
    auto scale = float(input_size) / output_size;
    input_index0 = at::empty({output_size, }, at::CPU(c10::CppTypeToScalarType<index_t>()));
    input_index1 = at::empty({output_size, }, at::CPU(c10::CppTypeToScalarType<index_t>()));
    lambda1 = at::empty({output_size, }, at::CPU(at::kFloat));
    lambda0 = at::empty({output_size, }, at::CPU(at::kFloat));

    auto input_index0_ptr = (index_t *) input_index0.data_ptr();
    auto input_index1_ptr = (index_t *) input_index1.data_ptr();
    auto lambda1_ptr = (float *) lambda1.data_ptr();
    auto lambda0_ptr = (float *) lambda0.data_ptr();
    float xf;
    long xl;

    // auto input_index = real_input_index.floor();
    // lambda1 = real_input_index - input_index;
    // lambda0 = 1.0 - lambda1;
    // input_index0 = input_index.to(at::kLong);
    // input_index1 = (input_index0 + 1).clamp(0, input_size - 1);

    for (index_t i=0; i<output_size; i++) {
        xf = std::max((float)((i + 0.5f) * scale - 0.5), 0.0f);
        xl = (long) xf;
        input_index0_ptr[i] = xl * stride;
        input_index1_ptr[i] = std::min(xl + 1, input_size - 1) * stride;
        xf -= (float) xl;
        lambda1_ptr[i] = xf;
        lambda0_ptr[i] = 1.0 - xf;
    }
}


void ti_reshape_indices_weights(at::Tensor & ix, at::Tensor & wx, const std::vector<long> & shape)
{
    ix = ix.reshape(shape);
    wx = wx.reshape(shape);
}


template <typename index_t, int out_ndims>
at::Tensor ti_upsample_linearNd_kernel_impl(
    const at::Tensor& input,
    at::IntArrayRef output_size)
{
  // input can be NCHW, NCL or NCKHW
  auto shape = input.sizes().vec();
  auto strides = input.strides().vec();

  TORCH_CHECK(out_ndims == output_size.size());

  for (int i=0; i<out_ndims; i++) {
    shape[i + 2] = output_size[i];
    strides[i + 2] = 0;
  }
  auto restrided_input = input.as_strided(shape, strides);

  at::Tensor i0[out_ndims], i1[out_ndims], w0[out_ndims], w1[out_ndims];  
  for (int i=0; i<out_ndims; i++) {
    int64_t index_stride = input.stride(i + 2);
    ti_compute_indices_weights_faster<index_t>(input.size(i + 2), output_size[i], index_stride, i0[i], i1[i], w0[i], w1[i]);
    auto new_shape = std::vector<long>(input.dim(), 1);
    new_shape[i + 2] = -1;
    ti_reshape_indices_weights(i0[i], w0[i], new_shape);
    ti_reshape_indices_weights(i1[i], w1[i], new_shape);
  }

  at::TensorIteratorConfig config;
  config.check_all_same_dtype(false)
    .declare_static_dtype_and_device(input.scalar_type(), input.device())
    .add_output(at::Tensor())
    .add_input(restrided_input);
  
  for (int i=0; i<out_ndims; i++) {
    config.add_input(i0[i]);
    config.add_input(w0[i]);
    config.add_input(i1[i]);
    config.add_input(w1[i]);
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
    at::IntArrayRef output_size)
{
  if (at::native::canUse32BitIndexMath(input))
    return ti_upsample_linearNd_kernel_impl<int32_t, 2>(input, output_size);
  
  return ti_upsample_linearNd_kernel_impl<int64_t, 2>(input, output_size);
}


at::Tensor ti_upsample_linear1d_kernel_impl(
    const at::Tensor& input,
    at::IntArrayRef output_size)
{
  if (at::native::canUse32BitIndexMath(input))
    return ti_upsample_linearNd_kernel_impl<int32_t, 1>(input, output_size);

  return ti_upsample_linearNd_kernel_impl<int64_t, 1>(input, output_size);
}


at::Tensor ti_upsample_trilinear3d_kernel_impl(
    const at::Tensor& input,
    at::IntArrayRef output_size)
{
  if (at::native::canUse32BitIndexMath(input))
    return ti_upsample_linearNd_kernel_impl<int32_t, 3>(input, output_size);

  return ti_upsample_linearNd_kernel_impl<int64_t, 3>(input, output_size);
}
