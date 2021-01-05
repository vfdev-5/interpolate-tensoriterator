#include <vector>
#include <ATen/native/TensorIterator.h>
#include <ATen/cpu/vec256/vec256.h>

#include <ATen/cpu/vec256/intrinsics.h>

using index_t = int32_t;

#if 0
template <int N>
struct Indexer {
  Indexer(index_t num_interp_dims, index_t k_size, char** indexers, const index_t* indexer_strides)
    : num_interp_dims(num_interp_dims)
    , k_size(k_size)
    , indexers(indexers)
    , indexer_strides(indexer_strides) {
    // AT_ASSERT(original_strides.size() == num_indexers);
    // AT_ASSERT(original_sizes.size() == num_indexers);
    /*
    for (int i=0; i < num_interp_dims * k_size; i += k_size) {
      for (int j=0; j < k_size; j++) {
        TORCH_CHECK(strides[i] == strides[j])
      }
    }
    */
  }

  index_t num_interp_dims;
  index_t k_size;
  char** indexers;
  const index_t* indexer_strides;

  index_t get(index_t idx) {
    index_t offset = 0;
    for (int j = 0; j < num_indexers; j++) {
      index_t value = *(index_t*)&indexers[j][idx * indexer_strides[j]];
    }
    return offset;

    for (index_t j = 0; j < num_interp_dims; j++) {
      for (index_t k = 0; k < k_size, k++) {
        index_t loc = 2 + j * k_size + k;
        char* idx = indexers[loc];
        char* weight = indexers[]
        for (index_t i = 0; i < n; i++) {

        }
      }

      for (index_t i = 0; i < n; i++) {
        offset1 = *(index_t*)&indexers[2][i * indexer_strides[2]];
        offset2 = *(index_t*)&indexers[4][i * indexer_strides[4]];
        offset3 = *(index_t*)&indexers[6][i * indexer_strides[6]];
        offset4 = *(index_t*)&indexers[8][i * indexer_strides[8]];
        f(dst + strides[0] * i, src + strides[1] * i, \
          offset1, offset2, data[3] + strides[3] * i, data[5] + strides[5] * i, \
          offset3, offset4, data[7] + strides[7] * i, data[9] + strides[9] * i);
      }
    }
  }
};



template <typename scalar_t, int N>
inline scalar_t dot_product(scalar_t *a, scalar_t * b, index_t n) {
  scalar_t res = a[0] * b[0];
  for (index_t i = 1; i < n; i++) {
    res += a[i] * b[i];
  }
  return res;
}
#endif


template <typename scalar_t, int step>
//inline void load1(scalar_t* temp, char *src, index_t * ix0_) {
inline void load1(scalar_t* temp, scalar_t *src, index_t * ix0_) {
    for (int k = 0; k < step; k++) {
      //temp[k] = *(scalar_t*)(src + (*ix0_));
      //temp[k] = *((scalar_t*)src + *ix0_);
      temp[k] = *(src + *ix0_);
      ix0_++;
    }
}

template <typename scalar_t, int step>
inline void load2(scalar_t* temp, char *src, index_t * ix0_) {
    using Vec = at::vec256::Vec256<index_t>;
    auto ix0 = Vec::loadu(ix0_);
    return at::vec256::gather(temp, ix0, 1);
}

template <typename scalar_t, int step>
inline void compute_loaded(scalar_t* temp, scalar_t * src1, scalar_t* src2, float* wx0_, float* wx1_) {
    for (int k = 0; k < step; k++) {
      temp[k] = *src1 * *wx0_ + *src2 * *wx1_;
      src1++;
      src2++;
      wx0_++;
      wx1_++;
    }
}


template <typename scalar_t, int step>
inline void compute1(scalar_t* temp, char *src, index_t * ix0_, index_t * ix1_, float* wx0_, float* wx1_) {
    for (int k = 0; k < step; k++) {
      temp[k] = (*(scalar_t*)(src + (*ix0_)) * (*wx0_) + *(scalar_t*)(src + (*ix1_)) * (*wx1_));
      ix0_++;
      ix1_++;
      wx0_++;
      wx1_++;
    }
}

template <typename scalar_t, int step>
inline void compute2(scalar_t * dst, scalar_t* temp1, scalar_t* temp2, float wy0_, float wy1_) {
    for (int k = 0; k < step; k++) {
      *dst = temp1[k] * wy0_ + temp2[k] * wy1_;
      dst++;
    }
}


template <typename scalar_t, typename func_t>
void ti_cpu_upsample_linear(
  at::TensorIterator& iter, const func_t& f, bool serial_execution=false
) {
  // When launch the index parallel version, set a relative samll grain size less than the INTERNAL::GRAIN_SIZE
  // to make the whole available thread numbers get more balanced work load and a better cache location.
  // The grain size here is chosen by the op benchmark to overcome the thread launch overhead
  // const int index_parallel_grain_size = 3000;
  //auto loop = [&](char** data, const index_t* strides, index_t n, index_t n_interp_dims, index_t k_size) {
  auto loop = [&](char** data, const int64_t* strides, int64_t n) {
    char* dst = data[0];
    char* src = data[1];
    char* ix0 = data[2];
    char* wx0 = data[3];
    char* ix1 = data[4];
    char* wx1 = data[5];
    char* iy0 = data[6];
    char* wy0 = data[7];
    char* iy1 = data[8];
    char* wy1 = data[9];

    index_t offset1, offset2, offset3, offset4;
    TORCH_INTERNAL_ASSERT(strides[2] == strides[4]); // ix
    TORCH_INTERNAL_ASSERT(strides[6] == strides[8]); // iy
    TORCH_INTERNAL_ASSERT(strides[3] == strides[5]); // wx
    TORCH_INTERNAL_ASSERT(strides[7] == strides[9]); // wy

    if (strides[2] == 0) {
      offset1 = *(index_t*)&ix0[0];
      offset2 = *(index_t*)&ix1[0];
      TORCH_CHECK(strides[3] == 0);
      TORCH_CHECK(strides[5] == 0);
      for (index_t i = 0; i < n; i++) {
        offset3 = *(index_t*)&iy0[i * strides[6]];
        offset4 = *(index_t*)&iy1[i * strides[8]];
        f(dst + strides[0] * i, src + strides[1] * i, \
          offset1, offset2, wx0, wx1, \
          offset3, offset4, wy0 + strides[7] * i, wy1 + strides[9] * i);
      }
    } else if (strides[6] == 0) {
      offset3 = *(index_t*)&iy0[0];
      offset4 = *(index_t*)&iy1[0];
      TORCH_CHECK(strides[7] == 0);
      TORCH_CHECK(strides[9] == 0);
      TORCH_CHECK(strides[1] == 0); // TODO need to add this case

      if (strides[2] == sizeof(index_t) && strides[4] && sizeof(index_t) && strides[3] == sizeof(float) && strides[5] == sizeof(float) && strides[0] == sizeof(scalar_t)) {

      constexpr int step = 4;
      scalar_t temp1[step] = {0};
      scalar_t temp2[step] = {0};
      scalar_t temp3[step] = {0};
      scalar_t temp4[step] = {0};
      scalar_t * src1 = (scalar_t *)(src) + offset3;
      scalar_t * src2 = (scalar_t *)(src) + offset4;
      index_t i = 0;
      index_t * ix0_ = (index_t *) ix0;
      index_t * ix1_ = (index_t *) ix1;

      float * wx0_ = (float*) wx0;
      float * wx1_ = (float*) wx1;

      float wy0_ = *(float*) wy0;
      float wy1_ = *(float*) wy1;

      scalar_t *dst_ = (scalar_t*) dst;

      for (; i < n - (n % step); i += step) {
        //compute1<scalar_t, step>(temp1, src1, (index_t *) ix0 + i, (index_t*) ix1 + i, (float*) wx0 + i, (float*) wx1 + i);
        //compute1<scalar_t, step>(temp2, src2, (index_t *) ix0 + i, (index_t*) ix1 + i, (float*) wx0 + i, (float*) wx1 + i);
        load1<scalar_t, step>(temp3, src1, ix0_ + i);
        load1<scalar_t, step>(temp4, src1, ix1_ + i);
        compute_loaded<scalar_t, step>(temp1, temp3, temp4,  wx0_ + i, wx1_ + i);
        load1<scalar_t, step>(temp3, src2, ix0_ + i);
        load1<scalar_t, step>(temp4, src2, ix1_ + i);
        compute_loaded<scalar_t, step>(temp2, temp3, temp4,  wx0_ + i, wx1_ + i);
        compute2<scalar_t, step>(dst_ + i, temp1, temp2, wy0_, wy1_);
      }
      //for (index_t i = 0; i < n; i++) {
      for (; i < n; i++) {
        load1<scalar_t, 1>(temp3, src1, ix0_ + i);
        load1<scalar_t, 1>(temp4, src1, ix1_ + i);
        compute_loaded<scalar_t, 1>(temp1, temp3, temp4,  wx0_ + i, wx1_ + i);
        load1<scalar_t, 1>(temp3, src2, ix0_ + i);
        load1<scalar_t, 1>(temp4, src2, ix1_ + i);
        compute_loaded<scalar_t, 1>(temp2, temp3, temp4, wx0_ + i, wx1_ + i);
        compute2<scalar_t, 1>(dst_ + i, temp1, temp2, wy0_, wy1_);
      }
      for (; i < n; i++) {
        offset1 = *(index_t*)&ix0[i * strides[2]];
        offset2 = *(index_t*)&ix1[i * strides[4]];
        f(dst + strides[0] * i, src, \
          offset1, offset2, wx0 + strides[3] * i, wx1 + strides[5] * i, \
          offset3, offset4, wy0, wy1);
      }
      } else {
        TORCH_CHECK(false, "Not supposed to be here ", strides[0], " ", strides[1], " ", strides[2]);
      }
    } else {
      for (index_t i = 0; i < n; i++) {
        offset1 = *(index_t*)&ix0[i * strides[2]];
        offset2 = *(index_t*)&ix1[i * strides[4]];
        offset3 = *(index_t*)&iy0[i * strides[6]];
        offset4 = *(index_t*)&iy1[i * strides[8]];
        f(dst + strides[0] * i, src + strides[1] * i, \
          offset1, offset2, wx0 + strides[3] * i, wx1 + strides[5] * i, \
          offset3, offset4, wy0 + strides[7] * i, wy1 + strides[9] * i);
      }

    }
  };
  if (serial_execution) {
    iter.serial_for_each(loop, {0, iter.numel()});
  } else {
    // iter.for_each(loop, index_parallel_grain_size);
    iter.for_each(loop);
  }
}


void ti_compute_indices_weights(
  long input_size, long output_size, index_t stride, at::Tensor & input_index0, at::Tensor & input_index1, at::Tensor & lambda0, at::Tensor & lambda1
) {
    auto scale = float(input_size) / output_size;
    // ((at::arange(output_size).to(at::kFloat) + 0.5) * scale - 0.5).clamp(0);
    auto real_input_index = at::linspace(0.5 * scale - 0.5, (output_size - 0.5) * scale - 0.5, output_size).clamp(0);
    auto input_index = real_input_index.floor();
    lambda1 = real_input_index - input_index;
    lambda0 = 1.0 - lambda1;
    input_index0 = input_index.to(at::kLong);
    input_index1 = (input_index0 + 1).clamp(0, input_size - 1);
}

void ti_compute_indices_weights_faster(
  int64_t input_size, int64_t output_size, int64_t stride, at::Tensor & input_index0, at::Tensor & input_index1, at::Tensor & lambda0, at::Tensor & lambda1
) {
    auto scale = float(input_size) / output_size;
    input_index0 = at::empty({output_size, }, at::CPU(at::kInt));
    input_index1 = at::empty({output_size, }, at::CPU(at::kInt));
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


at::Tensor ti_upsample_bilinear2d_kernel_impl(
    const at::Tensor& input,
    at::IntArrayRef output_size)
{
    // assumption about input spatial dimensions: HCHW
    auto shape = input.sizes().vec();
    shape[2] = output_size[0];
    shape[3] = output_size[1];
    auto strides = input.strides().vec();
    strides[2] = 0;
    strides[3] = 0;
    auto restrided_input = input.as_strided(shape, strides);

    index_t element_size_bytes = input.element_size();
    index_t x_indexed_stride = input.stride(3);// * element_size_bytes;
    index_t y_indexed_stride = input.stride(2);// * element_size_bytes;


    at::Tensor ix0, ix1, wx0, wx1;
    at::Tensor iy0, iy1, wy0, wy1;
    ti_compute_indices_weights_faster(input.size(3), output_size[1], x_indexed_stride, ix0, ix1, wx0, wx1);
    ti_compute_indices_weights_faster(input.size(2), output_size[0], y_indexed_stride, iy0, iy1, wy0, wy1);

    auto sx = std::vector<long>(input.dim(), 1);
    sx[3] = -1;
    ti_reshape_indices_weights(ix0, wx0, sx);
    ti_reshape_indices_weights(ix1, wx1, sx);
    auto sy = std::vector<long>(input.dim(), 1);
    sy[2] = -1;
    ti_reshape_indices_weights(iy0, wy0, sy);
    ti_reshape_indices_weights(iy1, wy1, sy);

    at::TensorIteratorConfig config;
    auto iter = config.check_all_same_dtype(false)
        .declare_static_dtype_and_device(input.scalar_type(), input.device())
        // .set_check_mem_overlap(false)
        .add_output(at::Tensor())
        .add_input(restrided_input)
        .add_input(ix0)
        .add_input(wx0)
        .add_input(ix1)
        .add_input(wx1)
        .add_input(iy0)
        .add_input(wy0)
        .add_input(iy1)
        .add_input(wy1)
        .build();

    //AT_DISPATCH_ALL_TYPES(
    AT_DISPATCH_FLOATING_TYPES(
        iter.dtype(), "upsample_bilinear2d", [&] {
        ti_cpu_upsample_linear<scalar_t>(iter, [](
                char* dst, char* src, \
                index_t offset1, index_t offset2, char* v0, char* v1, \
                index_t offset3, index_t offset4, char* v2, char* v3)
        {
//           output_data[output_offset] =
//               h0lambda * w0lambda * input_indexr(c, ih0, iw0) + /* h0 * w0 * i00 */
//               h0lambda * w1lambda * input_indexr(c, ih0, iw1) + /* h0 * w1 * i01 */
//               h1lambda * w0lambda * input_indexr(c, ih1, iw0) + /* h1 * w0 * i10 */
//               h1lambda * w1lambda * input_indexr(c, ih1, iw1);  /* h1 * w1 * i11 */            
//

          *(scalar_t*)dst = *(scalar_t*)v2 * (*(scalar_t*)(src + offset1 + offset3) * *(scalar_t*)v0 + \
                            *(scalar_t*)(src + offset2 + offset3) * *(scalar_t*)v1) + \
                            *(scalar_t*)v3 * (*(scalar_t*)(src + offset1 + offset4) * *(scalar_t*)v0 + \
                            *(scalar_t*)(src + offset2 + offset4) * *(scalar_t*)v1);
#if 0
          *(scalar_t*)dst = *(scalar_t*)(src + offset1 + offset3) * *(scalar_t*)v0 * *(scalar_t*)v2 + \
                            *(scalar_t*)(src + offset2 + offset3) * *(scalar_t*)v1 * *(scalar_t*)v2 + \
                            *(scalar_t*)(src + offset1 + offset4) * *(scalar_t*)v0 * *(scalar_t*)v3 + \
                            *(scalar_t*)(src + offset2 + offset4) * *(scalar_t*)v1 * *(scalar_t*)v3;
#endif
        });
    });

    return iter.output();
}
