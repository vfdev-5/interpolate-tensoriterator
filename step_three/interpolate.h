#include <vector>
#include <cstddef>

#include <ATen/native/TensorIterator.h>
#include <ATen/cpu/vec256/vec256.h>


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


// #define GET_VEC_OFFSETS(i, x_index, x_inner_stride, x_outer_stride, y_index, y_inner_stride, y_outer_stride, c) { \
//   *(int64_t*)&x_index[(i + 0) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 0) * y_inner_stride] * y_outer_stride + (i + 0) * c, \
//   *(int64_t*)&x_index[(i + 1) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 1) * y_inner_stride] * y_outer_stride + (i + 1) * c, \
//   *(int64_t*)&x_index[(i + 2) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 2) * y_inner_stride] * y_outer_stride + (i + 2) * c, \
//   *(int64_t*)&x_index[(i + 3) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 3) * y_inner_stride] * y_outer_stride + (i + 3) * c, \
//   *(int64_t*)&x_index[(i + 4) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 4) * y_inner_stride] * y_outer_stride + (i + 4) * c, \
//   *(int64_t*)&x_index[(i + 5) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 5) * y_inner_stride] * y_outer_stride + (i + 5) * c, \
//   *(int64_t*)&x_index[(i + 6) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 6) * y_inner_stride] * y_outer_stride + (i + 6) * c, \
//   *(int64_t*)&x_index[(i + 7) * x_inner_stride] * x_outer_stride + *(int64_t*)&y_index[(i + 7) * y_inner_stride] * y_outer_stride + (i + 7) * c \
// }

#define GET_VEC_OFFSETS(i, x_index, x_inner_stride, y_index, y_inner_stride) { \
  *(int64_t*)&x_index[(i + 0) * x_inner_stride] + *(int64_t*)&y_index[(i + 0) * y_inner_stride], \
  *(int64_t*)&x_index[(i + 1) * x_inner_stride] + *(int64_t*)&y_index[(i + 1) * y_inner_stride], \
  *(int64_t*)&x_index[(i + 2) * x_inner_stride] + *(int64_t*)&y_index[(i + 2) * y_inner_stride], \
  *(int64_t*)&x_index[(i + 3) * x_inner_stride] + *(int64_t*)&y_index[(i + 3) * y_inner_stride], \
  *(int64_t*)&x_index[(i + 4) * x_inner_stride] + *(int64_t*)&y_index[(i + 4) * y_inner_stride], \
  *(int64_t*)&x_index[(i + 5) * x_inner_stride] + *(int64_t*)&y_index[(i + 5) * y_inner_stride], \
  *(int64_t*)&x_index[(i + 6) * x_inner_stride] + *(int64_t*)&y_index[(i + 6) * y_inner_stride], \
  *(int64_t*)&x_index[(i + 7) * x_inner_stride] + *(int64_t*)&y_index[(i + 7) * y_inner_stride] \ 
} 



template <typename scalar_t, typename vec_func_t>
void ti_cpu_upsample_linear_vectorized(
  // at::TensorIterator& iter, int64_t y_stride, int64_t x_stride, const vec_func_t& vec_f, bool serial_execution=false
  at::TensorIterator& iter, const vec_func_t& vec_f, bool serial_execution=false
) {

  // When launch the index parallel version, set a relative samll grain size less than the INTERNAL::GRAIN_SIZE
  // to make the whole available thread numbers get more balanced work load and a better cache location.
  // The grain size here is chosen by the op benchmark to overcome the thread launch overhead
  // const int index_parallel_grain_size = 3000;
  auto loop = [&](char** data, const int64_t* strides, int64_t n) {

    using Vec = at::vec256::Vec256<scalar_t>;

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

    if (strides[1] > 0) {
      std::cout << "Src stride is not 0" << std::endl;
      assert(false);
    }

    int64_t offset1, offset2, offset3, offset4;
    using Vec = at::vec256::Vec256<scalar_t>;
    using integer_t = at::vec256::int_same_size_t<scalar_t>;
    using iVec = at::vec256::Vec256<integer_t>;

    Vec vec_dst;
    Vec vec_srcs[4];
    Vec vec_wghts[4];

    auto step = Vec::size();

    auto setup_vec_src = [&](char* _src, int64_t* vec_offsets, Vec * _vec_srcs, uint8_t _vec_src_index) {
      // TODO replace step by min(step, num_remaining_elements)
      scalar_t buffer[8] = {
        *(scalar_t*) (_src + vec_offsets[0]),
        *(scalar_t*) (_src + vec_offsets[1]),
        *(scalar_t*) (_src + vec_offsets[2]),
        *(scalar_t*) (_src + vec_offsets[3]),
        *(scalar_t*) (_src + vec_offsets[4]),
        *(scalar_t*) (_src + vec_offsets[5]),
        *(scalar_t*) (_src + vec_offsets[6]),
        *(scalar_t*) (_src + vec_offsets[7])
      };
      _vec_srcs[_vec_src_index] = Vec::loadu(buffer, 8);
    };

    auto setup_vec_wght = [&](char* _src1, int64_t _stride1, char* _src2, int64_t _stride2, Vec * _vec_wghts, uint8_t _vec_wght_index) {
      scalar_t buffer1[8] = {
        *(scalar_t *)(_src1 + _stride1 * 0),
        *(scalar_t *)(_src1 + _stride1 * 1),
        *(scalar_t *)(_src1 + _stride1 * 2),
        *(scalar_t *)(_src1 + _stride1 * 3),
        *(scalar_t *)(_src1 + _stride1 * 4),
        *(scalar_t *)(_src1 + _stride1 * 5),
        *(scalar_t *)(_src1 + _stride1 * 6),
        *(scalar_t *)(_src1 + _stride1 * 7)
      };
      Vec v1 = Vec::loadu(buffer1, 8);

      // SLOWER THEN ABOVE CODE      
      // Vec v1 = at::vec256::gather<>((scalar_t*)_src1, iVec::arange(0, _stride1));

      scalar_t buffer2[8] = { 
        *(scalar_t *)(_src2 + _stride2 * 0),
        *(scalar_t *)(_src2 + _stride2 * 1),
        *(scalar_t *)(_src2 + _stride2 * 2),
        *(scalar_t *)(_src2 + _stride2 * 3),
        *(scalar_t *)(_src2 + _stride2 * 4),
        *(scalar_t *)(_src2 + _stride2 * 5),
        *(scalar_t *)(_src2 + _stride2 * 6),
        *(scalar_t *)(_src2 + _stride2 * 7)    
      };
      Vec v2 = Vec::loadu(buffer2, 8);

      // SLOWER THEN ABOVE CODE
      // Vec v2 = at::vec256::gather<>((scalar_t*)_src2, iVec::arange(0, _stride2));
      _vec_wghts[_vec_wght_index] = v1 * v2;
    };

    for (int64_t i = 0; i < n; i+= step) {
      
      // offset1 = *(int64_t*)&ix0[i * strides[2]] * x_stride;
      // offset2 = *(int64_t*)&ix1[i * strides[4]] * x_stride;
      // offset3 = *(int64_t*)&iy0[i * strides[6]] * y_stride;
      // offset4 = *(int64_t*)&iy1[i * strides[8]] * y_stride;

      // setup vec_srcs:
      // We assume that source stride is zero (stride[1] == 0)
      int64_t vec_offsets00[step] = GET_VEC_OFFSETS(i, ix0, strides[2], iy0, strides[6]);
      setup_vec_src(src, vec_offsets00, vec_srcs, 0);
      int64_t vec_offsets01[step] = GET_VEC_OFFSETS(i, ix1, strides[4], iy0, strides[6]);
      setup_vec_src(src, vec_offsets01, vec_srcs, 1);
      int64_t vec_offsets10[step] = GET_VEC_OFFSETS(i, ix0, strides[2], iy1, strides[8]);
      setup_vec_src(src, vec_offsets10, vec_srcs, 2);
      int64_t vec_offsets11[step] = GET_VEC_OFFSETS(i, ix1, strides[4], iy1, strides[8]);
      setup_vec_src(src, vec_offsets11, vec_srcs, 3);

      // *(scalar_t*)(wx0 + strides[3] * i) * *(scalar_t*)(wy0 + strides[7] * i);
      setup_vec_wght(wx0 + strides[3] * i, strides[3], wy0 + strides[7] * i, strides[7], vec_wghts, 0);
      // *(scalar_t*)(wx1 + strides[5] * i) * *(scalar_t*)(wy0 + strides[7] * i);
      setup_vec_wght(wx1 + strides[5] * i, strides[5], wy0 + strides[7] * i, strides[7], vec_wghts, 1);
      // *(scalar_t*)(wx0 + strides[3] * i) * *(scalar_t*)(wy1 + strides[9] * i);
      setup_vec_wght(wx0 + strides[3] * i, strides[3], wy1 + strides[9] * i, strides[9], vec_wghts, 2);
      // *(scalar_t*)(wx1 + strides[5] * i) * *(scalar_t*)(wy1 + strides[9] * i);
      setup_vec_wght(wx1 + strides[5] * i, strides[5], wy1 + strides[9] * i, strides[9], vec_wghts, 3);

      // Spatial (HxW) vectorization
      vec_f(vec_dst, vec_srcs, vec_wghts, 4);
      vec_dst.store((scalar_t*)(dst + strides[0] * i), step);

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

    // Dispatch without vectorization
    // AT_DISPATCH_ALL_TYPES(
    //     iter.dtype(), "upsample_bilinear2d", [&] {
    //     ti_cpu_upsample_linear<scalar_t>(
    //       iter, 
    //       y_indexed_stride, 
    //       x_indexed_stride,
    //       [](char* dst, char* src, int64_t offset1, int64_t offset2, char* v0, char* v1, int64_t offset3, int64_t offset4, char* v2, char* v3) {
    //         // output_data[output_offset] =
    //         //     h0lambda * w0lambda * input_indexr(c, ih0, iw0) + /* h0 * w0 * i00 */
    //         //     h0lambda * w1lambda * input_indexr(c, ih0, iw1) + /* h0 * w1 * i01 */
    //         //     h1lambda * w0lambda * input_indexr(c, ih1, iw0) + /* h1 * w0 * i10 */
    //         //     h1lambda * w1lambda * input_indexr(c, ih1, iw1);  /* h1 * w1 * i11 */            
                  
    //         *(scalar_t*)dst = *(scalar_t*)(src + offset1 + offset3) * *(scalar_t*)v0 * *(scalar_t*)v2 + \
    //                           *(scalar_t*)(src + offset2 + offset3) * *(scalar_t*)v1 * *(scalar_t*)v2 + \
    //                           *(scalar_t*)(src + offset1 + offset4) * *(scalar_t*)v0 * *(scalar_t*)v3 + \
    //                           *(scalar_t*)(src + offset2 + offset4) * *(scalar_t*)v1 * *(scalar_t*)v3;  
    //       });
    // });

    // Dispatch with vectorization
    AT_DISPATCH_ALL_TYPES(
        iter.dtype(), "upsample_bilinear2d", [&] {

        using Vec = at::vec256::Vec256<scalar_t>;

        ti_cpu_upsample_linear_vectorized<scalar_t>(
          iter, 
          // y_indexed_stride, 
          // x_indexed_stride,
          [&](Vec & vec_dst, const Vec * vec_srcs, const Vec * vec_wghts, uint8_t n) {
            // output_data[output_offset] =
            //     h0lambda * w0lambda * input_indexr(c, ih0, iw0) + /* h0 * w0 * i00 */
            //     h0lambda * w1lambda * input_indexr(c, ih0, iw1) + /* h0 * w1 * i01 */
            //     h1lambda * w0lambda * input_indexr(c, ih1, iw0) + /* h1 * w0 * i10 */
            //     h1lambda * w1lambda * input_indexr(c, ih1, iw1);  /* h1 * w1 * i11 */            
            // *(scalar_t*)dst = *(scalar_t*)(src + offset1 + offset3) * *(scalar_t*)v0 * *(scalar_t*)v2 + \
            //                   *(scalar_t*)(src + offset2 + offset3) * *(scalar_t*)v1 * *(scalar_t*)v2 + \
            //                   *(scalar_t*)(src + offset1 + offset4) * *(scalar_t*)v0 * *(scalar_t*)v3 + \
            //                   *(scalar_t*)(src + offset2 + offset4) * *(scalar_t*)v1 * *(scalar_t*)v3;
            vec_dst = vec_srcs[0] * vec_wghts[0];
            for (uint8_t j=1; j < n; j++) {
              // fmadd = a * b + c
              vec_dst = at::vec256::fmadd(vec_srcs[j], vec_wghts[j], vec_dst);
            }
          });
    });

    // // Only Float
    // using scalar_t = float;
    // using Vec = at::vec256::Vec256<scalar_t>;

    // ti_cpu_upsample_linear_vectorized<scalar_t>(
    //   iter, 
    //   y_indexed_stride, 
    //   x_indexed_stride,
    //   [&](Vec & vec_dst, const Vec * vec_srcs, const Vec * vec_wghts, uint8_t n) {
    //     // output_data[output_offset] =
    //     //     h0lambda * w0lambda * input_indexr(c, ih0, iw0) + /* h0 * w0 * i00 */
    //     //     h0lambda * w1lambda * input_indexr(c, ih0, iw1) + /* h0 * w1 * i01 */
    //     //     h1lambda * w0lambda * input_indexr(c, ih1, iw0) + /* h1 * w0 * i10 */
    //     //     h1lambda * w1lambda * input_indexr(c, ih1, iw1);  /* h1 * w1 * i11 */            
    //     // *(scalar_t*)dst = *(scalar_t*)(src + offset1 + offset3) * *(scalar_t*)v0 * *(scalar_t*)v2 + \
    //     //                   *(scalar_t*)(src + offset2 + offset3) * *(scalar_t*)v1 * *(scalar_t*)v2 + \
    //     //                   *(scalar_t*)(src + offset1 + offset4) * *(scalar_t*)v0 * *(scalar_t*)v3 + \
    //     //                   *(scalar_t*)(src + offset2 + offset4) * *(scalar_t*)v1 * *(scalar_t*)v3;
    //     vec_dst = vec_srcs[0] * vec_wghts[0];
    //     for (uint8_t j=1; j < n; j++) {
    //       vec_dst += vec_srcs[j] * vec_wghts[j];
    //     }
    //   });

    return iter.output();
}
