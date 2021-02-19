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

template <typename scalar_t, typename index_t, int out_ndims>
static inline void
basic_loop(char** data, const int64_t* strides, const int64_t n) {
  char* dst = data[0];
  char* src = data[1];
  for (int64_t i = 0; i < n; i++) {
    *(scalar_t*)&dst[i * strides[0]] = interp_linear<out_ndims, scalar_t, index_t>(
        src + i * strides[1], &data[2], &strides[2], i);
  }
}

template <typename scalar_t, typename index_t, int out_ndims>
void ti_cpu_upsample_linear(char** data, const int64_t* strides, const int64_t n) {
    
    if (strides[0] == sizeof(scalar_t) && strides[1] == sizeof(scalar_t) && n > 128) {
      // contiguous channels-last case
      basic_loop<scalar_t, index_t, out_ndims>(data, strides, n);
    } 

}


int main(int argc, char ** argv) {

    int64_t in_size = atoi(argv[1]);
    int64_t out_size = atoi(argv[2]);
    float scale = out_size / in_size;

    float * output = new float[out_size];
    for (int i=0; i<out_size; i++) {
        output[i] = 0.0;
    }

    float * input = new float[in_size];
    int32_t * ix0 = new int32_t[in_size];
    int32_t * ix1 = new int32_t[in_size];
    int32_t * iy0 = new int32_t[in_size];
    int32_t * iy1 = new int32_t[in_size];

    float * wx0 = new float[in_size];
    float * wx1 = new float[in_size];
    float * wy0 = new float[in_size];
    float * wy1 = new float[in_size];

    for (int i=0; i<in_size; i++) {
        input[i] = atof(argv[1 + 2 + i]) * 1.0 / in_size;
        ix0[i] = int32_t(i * scale);
        ix1[i] = ix0[i] + 1;
        wx0[i] = i * scale - ix0[i];
        wx1[i] = 1.0 - wx0[i];

        iy0[i] = int32_t(i * scale);
        iy1[i] = iy0[i] + 1;
        wy0[i] = i * scale - iy0[i];
        wy1[i] = 1.0 - wy0[i];
    }

    char ** data = new char*[10];
    data[0] = (char *) output;
    data[1] = (char *) input;
    data[2] = (char *) iy0;
    data[3] = (char *) wy0;
    data[4] = (char *) iy1;
    data[5] = (char *) wy1;
    data[6] = (char *) ix0;
    data[7] = (char *) wx0;
    data[8] = (char *) ix1;
    data[9] = (char *) wx1;

    int64_t * strides = new int64_t[10];
    strides[0] = sizeof(float);
    strides[1] = sizeof(float);

    strides[2] = 0;  // iy0
    strides[3] = 0;  // wy0
    strides[4] = 0;  // iy1
    strides[5] = 0;  // wy1

    strides[6] = 0;  // ix0
    strides[7] = 0;  // wx0
    strides[8] = 0;  // ix1
    strides[9] = 0;  // wx1

    const int n = atoi(argv[1 + 2 + in_size]);
    // int n = 224;

    ti_cpu_upsample_linear<float, int32_t, 2>(data, strides, n);

    return int(data[0][0] + data[0][1]);
}