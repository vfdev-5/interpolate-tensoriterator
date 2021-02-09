
#define WITH_STRIDES_OPTIM
// #define WITH_STRIDES_OPTIM_2

#include <stdlib.h>
#include <cassert>


template <typename scalar_t, typename index_t>
static inline scalar_t interp_linear(char* src, char** data, const int64_t* strides, int64_t i) {
    // data = {const_indices, var_indices, weights}
    index_t ci = *(index_t*)&data[0][i * strides[0]];
    index_t vi = *(index_t*)&data[1][i * strides[1]];
    scalar_t w = *(scalar_t *)&data[2][i * strides[2]];
    scalar_t t = *(scalar_t *)&src[ci + vi];
    return t * w;
}


static inline void basic_loop(char ** data, const int64_t * strides, int64_t n) {

  char* dst = data[0];
  char* src = data[1];
  for (int64_t i = 0; i < n; i++) {

    *(float*)&dst[i * strides[0]] = interp_linear<float, int32_t>(
        src + i * strides[1], &data[2], &strides[2], i);

  }
}


void test_func(char ** data, const int64_t * strides, int64_t n) {

#ifdef WITH_STRIDES_OPTIM_ONLY
    if (strides[0] == sizeof(float) && (strides[1] == 0) && (strides[2] == 0) && (strides[3] != 0)) {
        basic_loop(data, strides, n);
    } 
    else {
        basic_loop(data, strides, n);
    }
#else
    basic_loop(data, strides, n);
#endif

}



int main(int argc, char ** argv) {

    assert (argc == 3);


    constexpr int64_t in_size = 320;
    constexpr int64_t out_size = 4;

    float output[out_size];
    for (int i=0; i<out_size; i++) {
        output[i] = 0.0;
    }

    float input[in_size];
    int32_t const_indices[in_size];
    int32_t var_indices[in_size];
    float weights[in_size];

    for (int i=0; i<in_size; i++) {
        input[i] = (i - in_size * 0.5) * 1.0 / in_size;
        const_indices[i] = 100;
        var_indices[i] = i % 200;
        weights[i] = i * 1.0 / in_size;
    }

    char * data[] = {
        (char *) output,
        (char *) input,
        (char *) const_indices,
        (char *) var_indices,
        (char *) weights
    };

    int64_t strides[] = {
        sizeof(float),
        atoi(argv[1]),
        atoi(argv[2]),
        sizeof(int32_t),
        sizeof(float),
    };

    test_func(data, strides, out_size);

    return int(data[0][0] + data[0][1]);
}