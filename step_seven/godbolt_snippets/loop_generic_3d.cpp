// -O3 -mfma -mavx -mavx2 -fopt-info-vec-all
#include <stdlib.h>

#include <math.h>
#include <vector>


template <int n, typename scalar_t, typename index_t, int interp_size>
struct Interpolate {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      index_t ids = *(index_t*)&data[0][i * strides[0]];
      scalar_t wts = *(scalar_t*)&data[1][i * strides[1]];
      scalar_t t = Interpolate<n - 1, scalar_t, index_t, interp_size>::eval(src + ids, &data[2 * interp_size], &strides[2 * interp_size], i);
      scalar_t output = t * wts;
      for (int j=1; j<interp_size; j++) {
        ids = *(index_t*)&data[2 * j + 0][i * strides[2 * j + 0]];
        wts = *(scalar_t*)&data[2 * j + 1][i * strides[2 * j + 1]];
        t = Interpolate<n - 1, scalar_t, index_t, interp_size>::eval(src + ids, &data[2 * interp_size], &strides[2 * interp_size], i);
        output += t * wts;
      }
      return output;
  }
};

template <typename scalar_t, typename index_t, int interp_size>
struct Interpolate<1, scalar_t, index_t, interp_size> {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      index_t ids = *(index_t*)&data[0][i * strides[0]];
      scalar_t wts = *(scalar_t*)&data[1][i * strides[1]];
      scalar_t t = *(scalar_t *)&src[ids];
      scalar_t output = t * wts;
      for (int j=1; j<interp_size; j++) {
        ids = *(index_t*)&data[2 * j + 0][i * strides[2 * j + 0]];
        wts = *(scalar_t*)&data[2 * j + 1][i * strides[2 * j + 1]];
        t = *(scalar_t *)&src[ids];
        output += t * wts;
      }
      return output;
    }
};

template <int n, typename scalar_t, typename index_t>
struct Interpolate<n, scalar_t, index_t, 1> {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      index_t ids = *(index_t*)&data[0][i * strides[0]];
      return Interpolate<n - 1, scalar_t, index_t, 1>::eval(src + ids, &data[2], &strides[2], i);
  }
};

template <typename scalar_t, typename index_t>
struct Interpolate<1, scalar_t, index_t, 1> {
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
  for (int i=1; i<2 * interp_size; i++) {
    output &= (strides[i] == 0);
  }
  return output;
}

template <typename scalar_t, typename index_t, int interp_size>
static inline bool is_contiguous_stride(const int64_t* strides) {
  bool output = (strides[0] == sizeof(index_t)) && (strides[1] == sizeof(scalar_t));
  for (int i=2; i<2 * interp_size; i+=2) {
    output &= (strides[i] == sizeof(index_t)) && (strides[i + 1] == sizeof(scalar_t));
  }
  return output;
}

// Helper class to recursively check if all input strides corresponding to interpolated dimensions
// are equal zero except on a single dimension.
//
// Inputs: array of strides of size N, non_zero_stride_dim which can be -1, 0, 1, 2, ...
//   if non_zero_stride_dim, we check that all strides are equal zero, otherwise
//   4 strides corresponding to the strides for index_0, weight_0, index_1 and weight_1 for non_zero_stride_dim
//   dimension should be non zero.
//
// Unit check of the recursion is to verify whether 4 strides for one interpolated dimension are either zero,
// see method is_zero_stride, or (sizeof(index_t), sizeof(scalar_t), sizeof(index_t), sizeof(scalar_t)), see
// method is_contiguous_stride.
//
// In practice, we have the following cases:
// - for ND, float32, channel first, strides are
//         dimN-1,              dim1,           dim0
//         i0, w0, i1, w1, ..., i0, w0, i1, w1, i0, w0, i1, w1
// strides=(0,  0,  0,  0, ...,  0,  0,  0,  0,  4,  4,  4,  4)
//
// if size dim0 is 1 then its strides are 0 and dim1 strides are equal 4
//
// - for ND, float32, channel last, strides are
//         dimN-1,         dimN-2,             dim0
//         i0, w0, i1, w1, i0, w0, i1, w1, ... i0, w0, i1, w1
// strides=(0,  0,  0,  0,  0,  0,  0,  0, ..., 0,  0,  0,  0)
//
// Using these methods we can hint the compiler to factorize constant indices and weights
// in cpu_upsample_linear method
template <int N, int non_zero_stride_dim, typename scalar_t, typename index_t, int interp_size>
struct CheckAlmostAllZeroStrides {
  static inline bool eval(const int64_t* strides) {
    // N is dim index: N -> dim0, N-1 -> dim1, ...
    // non_zero_stride_dim should be out_dims - dim
    bool output;
    if (N == non_zero_stride_dim) {
      output = is_contiguous_stride<scalar_t, index_t, interp_size>(strides);
    } else {
      output = is_zero_stride<interp_size>(strides);
    }
    return output &&
      CheckAlmostAllZeroStrides<N - 1, non_zero_stride_dim, scalar_t, index_t, interp_size>::eval(
        &strides[2 * interp_size]);
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

// Helper method to compute interpolation for nearest, linear, cubic modes
template <typename scalar_t, typename index_t, int out_ndims, int interp_size>
static inline void basic_loop(char** data, const int64_t* strides, int64_t n) {
  char* dst = data[0];
  char* src = data[1];
  for (int64_t i = 0; i < n; i++) {
    *(scalar_t*)&dst[i * strides[0]] = interpolate<out_ndims, scalar_t, index_t, interp_size>(
        src + i * strides[1], &data[2], &strides[2], i);
  }
}

// Helper structs to use with upsample_generic_Nd_kernel_impl
template<typename scalar_t>
struct HelperInterpBase {

};

template<typename scalar_t>
struct HelperInterpNearest : public HelperInterpBase<scalar_t> {

  static const int interp_size = 1;

};

template<typename scalar_t>
struct HelperInterpLinear : public HelperInterpBase<scalar_t> {

  static const int interp_size = 2;
};


template<typename scalar_t>
struct HelperInterpCubic : public HelperInterpBase<scalar_t> {

  static const int interp_size = 4;
};

constexpr int out_ndims = 3;
using scalar_t = float;
constexpr int interp_size = 2;


void cpu_upsample_linear(char** data, const int64_t* strides, int64_t n)
{
    // special-cases to let the compiler apply compile-time input-specific optimizations
    if ((strides[0] == sizeof(scalar_t) && (strides[1] == 0) &&
        check_almost_all_zero_stride<out_ndims, 1, scalar_t, int64_t, interp_size>(&strides[2]))) {
      // contiguous channels-first case
      basic_loop<scalar_t, int64_t, out_ndims, interp_size>(data, strides, n);
    }
    // else if ((strides[0] == sizeof(scalar_t) && (strides[1] == sizeof(scalar_t)) &&
    //            check_almost_all_zero_stride<out_ndims, -1, scalar_t, int64_t, interp_size>(&strides[2]))) {
    //   // contiguous channels-last case
    //   basic_loop<scalar_t, int64_t, out_ndims, interp_size>(data, strides, n);
    // } else {
    //   // fallback
    //   basic_loop<scalar_t, int64_t, out_ndims, interp_size>(data, strides, n);
    // }
}


