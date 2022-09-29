// -O3 -mfma -mavx -mavx2 -fopt-info-vec-all
#include <stdlib.h>


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

template <typename scalar_t, typename index_t, int out_ndims, int interp_size>
static inline void basic_loop(char** data, const int64_t* strides, int64_t size0, int64_t size1) {

  constexpr int ntensor = 2 + 2 * out_ndims * interp_size;
  const int64_t* outer_strides = &strides[ntensor];

  char *dst, *src;
  for (int64_t j=0; j < size1; j++) {

    if (j > 0) {
      for (int arg = 0; arg < ntensor; arg++) {
        data[arg] += outer_strides[arg];
      }
    }
    dst = data[0];
    src = data[1];
    for (int64_t i = 0; i < size0; i++) {
      *(scalar_t*)&dst[i * strides[0]] = interpolate<out_ndims, scalar_t, index_t, interp_size>(
          src + i * strides[1], &data[2], &strides[2], i);
    }
  }
}


using index_t = int64_t;
using scalar_t = float;
constexpr int out_ndims = 2;
constexpr int interp_size = 2;


void func(char** data, const int64_t* strides, int64_t size0, int64_t size1)
{

    constexpr int ntensors = 2 + 2 * interp_size * out_ndims;
    // special-cases to let the compiler apply compile-time input-specific optimizations
    // if ((strides[0] == sizeof(scalar_t)) && (strides[1] == 0) &&
    //     check_almost_all_zero_stride<out_ndims, 1, scalar_t, index_t, interp_size>(&strides[2])) {
    //   // contiguous channels-first case
    //   basic_loop<scalar_t, index_t, out_ndims, interp_size>(data, strides, size0, size1, ntensors);
    // } 
    // else 
    if (
      (strides[0] == sizeof(scalar_t)) && (strides[1] == sizeof(scalar_t))
      && check_almost_all_zero_stride<out_ndims, -1, scalar_t, index_t, interp_size>(&strides[2])
      // here we check outer_strides
      && (strides[ntensors + 0] >= sizeof(scalar_t)) && (strides[ntensors + 1] == 0)
      && check_almost_all_zero_stride<out_ndims, 1, scalar_t, index_t, interp_size>(&strides[ntensors + 2])
    ) {      // contiguous channels-last case
      basic_loop<scalar_t, index_t, out_ndims, interp_size>(data, strides, size0, size1);
    } 
    // else 
    // {
    //   // fallback
    //   basic_loop<scalar_t, index_t, out_ndims, interp_size>(data, strides, size0, size1, ntensors);
    // }
}
