// STD
#include <iostream>

// AVX
#include <immintrin.h>


using uint8 = unsigned char;


int main(int argc, char** argv)
{
    static constexpr uint64_t size = 32;
    __attribute__ ((aligned (32))) uint8 aligned_x[size], aligned_y[size];
    __attribute__ ((aligned (32))) uint8 aligned_output[size];

    for (int i=0; i < size; i++) {
        aligned_x[i] = (uint8) (10 + i * 4);
        aligned_y[i] = (uint8) (22 + i * 5);
    }

    const __m256i x_vec = _mm256_loadu_si256((__m256i *) aligned_x);
    const __m256i y_vec = _mm256_loadu_si256((__m256i *) aligned_y);

    auto out_vec = _mm256_adds_epu8(x_vec, y_vec);

    _mm256_storeu_si256((__m256i *)aligned_output, out_vec);

    for (int i=0; i < size; i++) {
        std::cout << i << " : "
            << (int) aligned_x[i] << " + "
            << (int) aligned_y[i] << " = "
            << (int) aligned_output[i] << std::endl;
    }
    return 0;
}
