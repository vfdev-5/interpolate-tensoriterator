// STD
#include <vector>
#include <iostream>
#include <chrono>
#include <cassert>

// AVX
#include <immintrin.h>


using uint8 = unsigned char;


inline void sum_avx_uint8(const uint8 * x_ptr, const uint8 * y_ptr, uint8 * out_ptr, uint64_t size) {

    static constexpr uint32_t step = 32 / sizeof(uint8);

    uint64_t d = 0;
    __m256i vec_x, vec_y, vec_output;
    for (; d < size - (size % step); d+=step) {
        vec_x = _mm256_loadu_si256((__m256i *)(x_ptr + d));
        vec_y = _mm256_loadu_si256((__m256i *)(y_ptr + d));
        vec_output = _mm256_adds_epu8(vec_x, vec_y);
        _mm256_storeu_si256((__m256i *)(out_ptr + d), vec_output);
    }

    if (size - d > 0) {
        uint8 bufx[step];
        uint8 bufy[step];

        uint64_t i=0;
        for (; i < size - d; i++) {
            bufx[i] = *(x_ptr + d);
            bufy[i] = *(y_ptr + d);
        }
        for (;i < step; i++) {
            bufx[i] = 0;
            bufy[i] = 0;
        }
        vec_x = _mm256_loadu_si256((__m256i *) bufx);
        vec_y = _mm256_loadu_si256((__m256i *) bufy);
        vec_output = _mm256_adds_epu8(vec_x, vec_y);
        _mm256_storeu_si256((__m256i *) bufx, vec_output);
        for (i=0; i < size - d; i++) {
            *(out_ptr + d + i) = bufx[i];
        }
    }
}


inline void sum_avx_uint8(const std::vector<uint8> & x, std::vector<uint8> & y, std::vector<uint8> & out) {
    sum_avx_uint8(x.data(), y.data(), out.data(), out.size());
}


inline void sum_non_vec(const uint8 * x_ptr, const uint8 * y_ptr, uint8 * out_ptr, uint64_t size) {
    for (uint64_t i=0; i < size; i++) {
        long o_long = (long) x_ptr[i] + (long) y_ptr[i];
        out_ptr[i] = (uint8)( (o_long > 256) ? 255 : o_long );
    }
}

inline void sum_non_vec(const std::vector<uint8> & x, std::vector<uint8> & y, std::vector<uint8> & out) {
    sum_non_vec(x.data(), y.data(), out.data(), out.size());
}


bool is_32bit_aligned__odd_impl(void * ptr) {
    // http://www.cplusplus.com/forum/beginner/249090/
    return ((((intptr_t) ptr + 31) & ~ intptr_t(31)) == (intptr_t) ptr);
}

bool is_32bit_aligned(void * ptr) {
    // http://www.cplusplus.com/forum/beginner/249090/
    return ((intptr_t) ptr) % 32 == 0;
}


int main(int argc, char** argv)
{

    static constexpr uint64_t size = 10000;

    __attribute__ ((aligned (32))) uint8 aligned_x[size], aligned_y[size];
    std::cout << "align of aligned_x = " << alignof(aligned_x) << std::endl;
    std::cout << "align of aligned_y = " << alignof(aligned_y) << std::endl;

    for (uint64_t i=0; i<size; i++) {
        aligned_x[i] = i % 256;
        aligned_y[i] = (i + 12) % 256;
    }

    std::cout << "32-bit aligned aligned_x ? -> " << is_32bit_aligned__odd_impl(aligned_x) << " vs " << true << std::endl;
    std::cout << "32-bit aligned aligned_y ? -> " << is_32bit_aligned__odd_impl(aligned_y) << " vs " << true << std::endl;
    std::cout << "32-bit aligned aligned_x ? -> " << is_32bit_aligned(aligned_x) << " vs " << true << std::endl;
    std::cout << "32-bit aligned aligned_y ? -> " << is_32bit_aligned(aligned_y) << " vs " << true << std::endl;

    std::vector<uint8> x(aligned_x, aligned_x + size);
    std::vector<uint8> y(aligned_y, aligned_y + size);
    std::cout << "32-bit aligned x ? -> " << is_32bit_aligned(x.data()) << " vs " << true << std::endl;
    std::cout << "32-bit aligned y ? -> " << is_32bit_aligned(y.data()) << " vs " << true << std::endl;

    // Correctness check
    {
        __attribute__ ((aligned (32))) uint8 aligned_output[size];
        __attribute__ ((aligned (32))) uint8 aligned_expected[size];
        sum_avx_uint8(aligned_x, aligned_y, aligned_output, size);
        sum_non_vec(aligned_x, aligned_y, aligned_expected, size);

        for (uint64_t j=0; j<size; ++j) {
            assert(aligned_output[j] == aligned_expected[j]);
        }
    }

    unsigned int n = 10000;
    {
        __attribute__ ((aligned (32))) uint8 aligned_output[size];
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            sum_non_vec(aligned_x, aligned_y, aligned_output, size);
            aligned_output[0]++;
        }
        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- sum_non_vec took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    {
        std::vector<uint8> out(size);
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            sum_non_vec(x, y, out);
            out[0]++;
        }
        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- sum_non_vec as std::vector took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    {
        __attribute__ ((aligned (32))) uint8 aligned_output[size];
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            sum_avx_uint8(aligned_x, aligned_y, aligned_output, size);
            aligned_output[0]++;
        }
        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- sum_avx took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    {
        std::vector<uint8> out(size);
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            sum_avx_uint8(x, y, out);
            out[0]++;
        }
        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- sum_avx as std::vector took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    return 0;
}