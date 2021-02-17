#include <vector>
#include <ATen/native/TensorIterator.h>


template <typename scalar_t, typename func_t>
void ti_cpu_upsample_linear(
  at::TensorIterator& iter, int64_t y_stride, int64_t x_stride, const func_t& f, bool serial_execution=false
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

    for (int64_t i = 0; i < n; i++) {
      offset1 = *(int64_t*)&ix0[i * strides[2]] * x_stride;
      offset2 = *(int64_t*)&ix1[i * strides[4]] * x_stride;
      offset3 = *(int64_t*)&iy0[i * strides[6]] * y_stride;
      offset4 = *(int64_t*)&iy1[i * strides[8]] * y_stride;
      f(dst + strides[0] * i, src + strides[1] * i, \
        offset1, offset2, wx0 + strides[3] * i, wx1 + strides[5] * i, \
        offset3, offset4, wy0 + strides[7] * i, wy1 + strides[9] * i);
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
  long input_size, long output_size, at::Tensor & input_index0, at::Tensor & input_index1, at::Tensor & lambda0, at::Tensor & lambda1
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

    at::Tensor ix0, ix1, wx0, wx1;
    at::Tensor iy0, iy1, wy0, wy1;
    ti_compute_indices_weights(input.size(3), output_size[1], ix0, ix1, wx0, wx1);
    ti_compute_indices_weights(input.size(2), output_size[0], iy0, iy1, wy0, wy1);

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

    int64_t element_size_bytes = input.element_size();
    int64_t x_indexed_stride = input.stride(3) * element_size_bytes;
    int64_t y_indexed_stride = input.stride(2) * element_size_bytes;    

    AT_DISPATCH_ALL_TYPES(
        iter.dtype(), "upsample_bilinear2d", [&] {
        ti_cpu_upsample_linear<scalar_t>(iter, y_indexed_stride, x_indexed_stride, [](
                char* dst, char* src, \
                int64_t offset1, int64_t offset2, char* v0, char* v1, \
                int64_t offset3, int64_t offset4, char* v2, char* v3)
        {
//           output_data[output_offset] =
//               h0lambda * w0lambda * input_indexr(c, ih0, iw0) + /* h0 * w0 * i00 */
//               h0lambda * w1lambda * input_indexr(c, ih0, iw1) + /* h0 * w1 * i01 */
//               h1lambda * w0lambda * input_indexr(c, ih1, iw0) + /* h1 * w0 * i10 */
//               h1lambda * w1lambda * input_indexr(c, ih1, iw1);  /* h1 * w1 * i11 */            
            
          *(scalar_t*)dst = *(scalar_t*)(src + offset1 + offset3) * *(scalar_t*)v0 * *(scalar_t*)v2 + \
                            *(scalar_t*)(src + offset2 + offset3) * *(scalar_t*)v1 * *(scalar_t*)v2 + \
                            *(scalar_t*)(src + offset1 + offset4) * *(scalar_t*)v0 * *(scalar_t*)v3 + \
                            *(scalar_t*)(src + offset2 + offset4) * *(scalar_t*)v1 * *(scalar_t*)v3;
        });
    });

    return iter.output();
}
