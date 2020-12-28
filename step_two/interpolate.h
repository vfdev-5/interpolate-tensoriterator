#include <vector>
#include <ATen/native/TensorIterator.h>

/*
struct Indexer {
  Indexer(int64_t num_indexers, char** indexers, const int64_t* indexer_strides,
          IntArrayRef original_sizes, IntArrayRef original_strides)
    : num_indexers(num_indexers)
    , indexers(indexers)
    , indexer_strides(indexer_strides) {
    AT_ASSERT(original_strides.size() == num_indexers);
    AT_ASSERT(original_sizes.size() == num_indexers);
  }

  int64_t num_indexers;
  char** indexers;
  const int64_t* indexer_strides;

  int64_t get(int64_t idx) {
    int64_t offset = 0;
    for (int j = 0; j < num_indexers; j++) {
      int64_t value = *(int64_t*)&indexers[j][idx * indexer_strides[j]];
      offset += value * original_strides[j];
    }
    return offset;
  }
};
*/

template <typename scalar_t, typename func_t>
void ti_cpu_upsample_linear(
  at::TensorIterator& iter, const func_t& f, bool serial_execution=false
) {
  // When launch the index parallel version, set a relative samll grain size less than the INTERNAL::GRAIN_SIZE
  // to make the whole available thread numbers get more balanced work load and a better cache location.
  // The grain size here is chosen by the op benchmark to overcome the thread launch overhead
  // const int index_parallel_grain_size = 3000;
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

    int64_t offset1, offset2, offset3, offset4;
    TORCH_CHECK(strides[2] == strides[4]); // ix
    TORCH_CHECK(strides[6] == strides[8]); // iy
    TORCH_CHECK(strides[3] == strides[5]); // wx
    TORCH_CHECK(strides[7] == strides[9]); // wy

    if (strides[2] == 0) {
      offset1 = *(int64_t*)&ix0[0];
      offset2 = *(int64_t*)&ix1[0];
      TORCH_CHECK(strides[3] == 0);
      TORCH_CHECK(strides[5] == 0);
      for (int64_t i = 0; i < n; i++) {
        offset3 = *(int64_t*)&iy0[i * strides[6]];
        offset4 = *(int64_t*)&iy1[i * strides[8]];
        f(dst + strides[0] * i, src + strides[1] * i, \
          offset1, offset2, wx0, wx1, \
          offset3, offset4, wy0 + strides[7] * i, wy1 + strides[9] * i);
      }
    } else if (strides[6] == 0) {
      offset3 = *(int64_t*)&iy0[0];
      offset4 = *(int64_t*)&iy1[0];
      TORCH_CHECK(strides[7] == 0);
      TORCH_CHECK(strides[9] == 0);
      TORCH_CHECK(strides[1] == 0); // TODO need to add this case

      if (strides[2] == sizeof(int64_t) && strides[4] && sizeof(int64_t) && strides[3] == sizeof(float) && strides[5] == sizeof(float) && strides[0] == sizeof(scalar_t)) {
      for (int64_t i = 0; i < n; i++) {
        offset1 = *(int64_t*)&ix0[i * strides[2]];
        offset2 = *(int64_t*)&ix1[i * strides[4]];
        f(dst + strides[0] * i, src, \
          offset1, offset2, wx0 + strides[3] * i, wx1 + strides[5] * i, \
          offset3, offset4, wy0, wy1);
      }
      } else {
        TORCH_CHECK(false, "Not supposed to be here ", strides[0], " ", strides[1]);
      }
    } else {
      for (int64_t i = 0; i < n; i++) {
        offset1 = *(int64_t*)&ix0[i * strides[2]];
        offset2 = *(int64_t*)&ix1[i * strides[4]];
        offset3 = *(int64_t*)&iy0[i * strides[6]];
        offset4 = *(int64_t*)&iy1[i * strides[8]];
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
  long input_size, long output_size, int64_t stride, at::Tensor & input_index0, at::Tensor & input_index1, at::Tensor & lambda0, at::Tensor & lambda1
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
    input_index0 = at::empty({output_size, }, at::CPU(at::kLong));
    input_index1 = at::empty({output_size, }, at::CPU(at::kLong));
    lambda1 = at::empty({output_size, }, at::CPU(at::kFloat));
    lambda0 = at::empty({output_size, }, at::CPU(at::kFloat));

    auto input_index0_ptr = (int64_t *) input_index0.data_ptr();
    auto input_index1_ptr = (int64_t *) input_index1.data_ptr();
    auto lambda1_ptr = (float *) lambda1.data_ptr();
    auto lambda0_ptr = (float *) lambda0.data_ptr();
    float xf;
    long xl;

    // auto input_index = real_input_index.floor();
    // lambda1 = real_input_index - input_index;
    // lambda0 = 1.0 - lambda1;
    // input_index0 = input_index.to(at::kLong);
    // input_index1 = (input_index0 + 1).clamp(0, input_size - 1);

    for (uint64_t i=0; i<output_size; i++) {
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

    int64_t element_size_bytes = input.element_size();
    int64_t x_indexed_stride = input.stride(3) * element_size_bytes;
    int64_t y_indexed_stride = input.stride(2) * element_size_bytes;


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

    AT_DISPATCH_ALL_TYPES(
        iter.dtype(), "upsample_bilinear2d", [&] {
        ti_cpu_upsample_linear<scalar_t>(iter, [](
                char* dst, char* src, \
                int64_t offset1, int64_t offset2, char* v0, char* v1, \
                int64_t offset3, int64_t offset4, char* v2, char* v3)
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
