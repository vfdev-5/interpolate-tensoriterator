#include <stdlib.h>

// Helper structs and methods for cpu_upsample_linear
//
// Interpolation methods that used below are separable, and as such we can compute the interpolation
// independently per dimension in a recursive way. Please, refer to #10482 for more context.
//
// Linear Interpolation structure to compute output value in n-dimensional case.
// - recursively compute interpolated output for each dimension
// - we rely a lot on compiler's code optimization such that implemented operations
//   can be automatically factorized and vectorized using SSE and AVX2
template <int n, typename scalar_t, typename index_t, int interp_size>
struct Interpolate {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
      index_t ids = *(index_t*)&data[0][i * strides[0]];
      scalar_t wts = *(scalar_t*)&data[1][i * strides[1]];
      scalar_t t = Interpolate<n - 1, scalar_t, index_t, interp_size>::eval(src + ids, &data[2 * interp_size], &strides[2 * interp_size], i);
      scalar_t output = t * wts;
    //   for (const auto j : c10::irange(1, interp_size)) {
      for (auto j=1;j < interp_size; j++) {
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
      // for (const auto j : c10::irange(1, interp_size)) {
      for (auto j=1;j < interp_size; j++) {
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

// There is an unexpected 2x slowdown for upsample_trilinear3d channels_first
// for both 1 and 6 threads. We have to specialize this case as below:
// Once the issue is fixed we can keep generic implementation and remove:
// struct Interpolate<n, scalar_t, index_t, 2> and
// struct Interpolate<1, scalar_t, index_t, 2>
template <int n, typename scalar_t, typename index_t>
struct Interpolate<n, scalar_t, index_t, 2> {
    static inline scalar_t eval(char* src, char** data, const int64_t* strides, int64_t i) {
        index_t i0 = *(index_t*)&data[0][i * strides[0]];
        index_t i1 = *(index_t*)&data[2][i * strides[2]];
        scalar_t w0 = *(scalar_t *)&data[1][i * strides[1]];
        scalar_t w1 = *(scalar_t *)&data[3][i * strides[3]];

        scalar_t t0 = Interpolate<n - 1, scalar_t, index_t, 2>::eval(src + i0, &data[4], &strides[4], i);
        scalar_t t1 = Interpolate<n - 1, scalar_t, index_t, 2>::eval(src + i1, &data[4], &strides[4], i);

        return t0 * w0 + t1 * w1;
  }
};

template <typename scalar_t, typename index_t>
struct Interpolate<1, scalar_t, index_t, 2> {
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

template <int n, typename scalar_t, typename index_t, int interp_size>
static inline scalar_t interpolate(char* src, char** data, const int64_t* strides, int64_t i) {
  return Interpolate<n, scalar_t, index_t, interp_size>::eval(src, data, strides, i);
}

template<int m>
static inline bool is_zero_stride(const int64_t* strides) {
  bool output = strides[0] == 0;
//   for (const auto i : c10::irange(1, m)) {
  for (auto i=1;i < m; i++) {
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
    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    bool output;
    if (N == non_zero_stride_dim) {
      output = is_contiguous_stride<scalar_t, index_t, interp_size>(strides);
    } else {
      output = is_zero_stride<2 * interp_size>(strides);
    }
    return output &&
      CheckAlmostAllZeroStrides<N - 1, non_zero_stride_dim, scalar_t, index_t, interp_size>::eval(
        &strides[2 * interp_size]);
  }
};

template <int non_zero_stride_dim, typename scalar_t, typename index_t, int interp_size>
struct CheckAlmostAllZeroStrides<0, non_zero_stride_dim, scalar_t, index_t, interp_size> {
  static inline bool eval(const int64_t* /*strides*/) {
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

//   for (const auto i : c10::irange(n)) {
  for (auto i=0; i < n; i++){
    *(scalar_t*)&dst[i * strides[0]] = interpolate<out_ndims, scalar_t, index_t, interp_size>(
        src + i * strides[1], &data[2], &strides[2], i);
  }
}

// Generic upsampling computation method using TensorIterator for Nd case.
// Supports: nearest, linear, cubic modes with interp_size template argument: 1, 2, 4
//
// Single loop function for 1d, 2d and 3d cases and modes
// For N dimensions, output value up to Di dimension can be computed as
//
// output_i[a] = interpolate(output_{i+1}[a], w_{i+1}[a], output_{i+1}[a+1], w_{i+1}[a+1], ...)
// with
// output_DN[a] = interpolate(input_DN[a], w_DN[a], input_DN[a+1], w_DN[a+1], ...)
// and i - dimension index and a - linear index for spatial coordinates
//
// The recursive call is implemented with InterpLinear struct using template for
// the loop unrolling on compile time.
template <typename scalar_t, int out_ndims, int interp_size>
void foo(char** data, const int64_t* strides, int64_t n) {

    // special-cases to let the compiler apply compile-time input-specific optimizations
    if ((strides[0] == sizeof(scalar_t) && (strides[1] == 0) &&
        // NOLINTNEXTLINE(bugprone-branch-clone)
        check_almost_all_zero_stride<out_ndims, 1, scalar_t, int64_t, interp_size>(&strides[2]))) {
      // contiguous channels-first case

      basic_loop<scalar_t, int64_t, out_ndims, interp_size>(data, strides, n);
    }
//     else if ((strides[0] == sizeof(scalar_t) && (strides[1] == sizeof(scalar_t)) &&
//                check_almost_all_zero_stride<out_ndims, -1, scalar_t, int64_t, interp_size>(&strides[2]))) {


// #ifdef VERBOSE
//       if (TI_BASIC_LOOP_CHANNELS_LAST_TRIGGERED < 1) {
//         std::cout << "TI_BASIC_LOOP -> CHANNELS_LAST" << std::endl << std::flush;
//         TI_BASIC_LOOP_CHANNELS_LAST_TRIGGERED += 1;
//       }
// #endif

//       // contiguous channels-last case
//       basic_loop<scalar_t, int64_t, out_ndims, interp_size>(data, strides, n);
//     }
//     else {
//       // fallback

// #ifdef VERBOSE
//       if (TI_BASIC_LOOP_FALLBACK_TRIGGERED < 1) {
//         std::cout << "TI_BASIC_LOOP -> FALLBACK" << std::endl << std::flush;
//         TI_BASIC_LOOP_FALLBACK_TRIGGERED += 1;
//       }
// #endif

//       basic_loop<scalar_t, int64_t, out_ndims, interp_size>(data, strides, n);
//     }
}


void main_func(char** data, const int64_t* strides, int64_t n) {
  foo<float, 2, 1>(data, strides, n);
}

