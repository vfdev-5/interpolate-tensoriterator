
#define WITH_STRIDES_OPTIM

#include <stdlib.h>
#include <cassert>


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

// TODO: semantics of s are a bit weird maybe?
template <int N, int s, typename scalar_t, typename index_t>
struct IsAllZeroStride {
  static inline bool eval(const int64_t* strides) {
    return (N == s ? is_contiguous_stride<scalar_t, index_t>(strides) : is_zero_stride(strides)) &&
            IsAllZeroStride<N - 1, s, scalar_t, index_t>::eval(&strides[4]);
  }
};

template <int s, typename scalar_t, typename index_t>
struct IsAllZeroStride<1, s, scalar_t, index_t> {
  static inline bool eval(const int64_t* strides) {
    return (s == 1 ? is_contiguous_stride<scalar_t, index_t>(strides) : is_zero_stride(strides));
  }
};

template <int n, int s, typename scalar_t, typename index_t>
static inline bool is_all_zero_stride(const int64_t* strides) {
  return IsAllZeroStride<n, s, scalar_t, index_t>::eval(strides);
}

template <typename scalar_t, typename index_t, int out_ndims>
static inline void
basic_loop(char** data, const int64_t* strides, int64_t n) {
  char* dst = data[0];
  char* src = data[1];
  for (int64_t i = 0; i < n; i++) {
    *(scalar_t*)&dst[i * strides[0]] = interp_linear<out_ndims, scalar_t, index_t>(
        src + i * strides[1], &data[2], &strides[2], i);
  }
}

template <typename scalar_t, typename index_t, int out_ndims>
void ti_cpu_upsample_linear(char** data, const int64_t* strides, int64_t n) {
#ifdef WITH_STRIDES_OPTIM
    // special-cases to let the compiler apply compile-time input-specific optimizations
    if ((strides[0] == sizeof(scalar_t) && (strides[1] == 0) &&
        is_all_zero_stride<out_ndims, 1, scalar_t, index_t>(&strides[2]))) {
      // contiguous channels-first case
      basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
    } 
    else if ((strides[0] == sizeof(scalar_t) && (strides[1] == sizeof(scalar_t)) &&
               is_all_zero_stride<out_ndims, -1, scalar_t, index_t>(&strides[2]))) {
      // contiguous channels-last case
      basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
    } 
    else 
    {
      // fallback
      basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
    }
#else
    basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
#endif
}


int main(int argc, char ** argv) {

    const int64_t in_size = 320;
    const int64_t out_size = 4;
    float scale = out_size / in_size;

    assert (argc == 1 + 2 + in_size);

    float output[out_size];
    for (int i=0; i<out_size; i++) {
        output[i] = 0.0;
    }

    float input[in_size];
    int32_t ix0[in_size], ix1[in_size], iy0[in_size], iy1[in_size];
    float wx0[in_size], wx1[in_size], wy0[in_size], wy1[in_size];

    for (int i=0; i<in_size; i++) {
        input[i] = atof(argv[1 + i]) * 1.0 / in_size;
        ix0[i] = int32_t(i * scale);
        ix1[i] = ix0[i] + 1;
        wx0[i] = i * scale - ix0[i];
        wx1[i] = 1.0 - wx0[i];

        iy0[i] = int32_t(i * scale);
        iy1[i] = iy0[i] + 1;
        wy0[i] = i * scale - iy0[i];
        wy1[i] = 1.0 - wy0[i];
    }

    char * data[] = {
        (char *) output,
        (char *) input,
        (char *) iy0,
        (char *) wy0,
        (char *) iy1,
        (char *) wy1,
        (char *) ix0,
        (char *) wx0,
        (char *) ix1,
        (char *) wx1,
    };

    int64_t strides[] = {
        sizeof(float),
        0,

        0,  // iy0
        0,  // wy0
        0,  // iy1
        0,  // wy1
        sizeof(int32_t),  // ix0
        sizeof(int32_t),  // wx0
        sizeof(int32_t),  // ix1
        sizeof(int32_t),  // wx1

        // sizeof(int32_t),  // iy0
        // sizeof(int32_t),  // wy0
        // sizeof(int32_t),  // iy1
        // sizeof(int32_t),  // wy1
        // 0,  // ix0
        // 0,  // wx0
        // 0,  // ix1
        // 0,  // wx1

    };

    ti_cpu_upsample_linear<float, int32_t, 2>(data, strides, out_size);

    return int(data[0][0] + data[0][1]);
}