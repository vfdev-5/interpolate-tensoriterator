// -O3 -mfma -mavx -mavx2 -fopt-info-vec-all
#include <stdlib.h>

#include <math.h>
#include <vector>


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

// Helper method to compute linear interpolation
template <typename scalar_t, typename index_t, int out_ndims>
static inline void basic_loop(char** data, const int64_t* strides, int64_t n) {
  char* dst = data[0];
  char* src = data[1];
  for (int64_t i = 0; i < n; i++) {
    *(scalar_t*)&dst[i * strides[0]] = interp_linear<out_ndims, scalar_t, index_t>(
        src + i * strides[1], &data[2], &strides[2], i);
  }
}

constexpr int out_ndims = 3;
using scalar_t = float;

void cpu_upsample_linear(char** data, const int64_t* strides, int64_t n)
{
    // special-cases to let the compiler apply compile-time input-specific optimizations
    if ((strides[0] == sizeof(scalar_t) && (strides[1] == 0) &&
        is_all_zero_stride<out_ndims, 1, scalar_t, int64_t>(&strides[2]))) {
      // contiguous channels-first case
      basic_loop<scalar_t, int64_t, out_ndims>(data, strides, n);
    }
    // else if ((strides[0] == sizeof(scalar_t) && (strides[1] == sizeof(scalar_t)) &&
    //            is_all_zero_stride<out_ndims, -1, scalar_t, int64_t>(&strides[2]))) {
    //   // contiguous channels-last case
    //   basic_loop<scalar_t, int64_t, out_ndims>(data, strides, n);
    // } else {
    //   // fallback
    //   basic_loop<scalar_t, int64_t, out_ndims>(data, strides, n);
    // }
}


