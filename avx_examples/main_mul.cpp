// STD
#include <vector>
#include <iostream>
#include <chrono>

// AVX
#include <immintrin.h>


#define FLOAT_VEC_EXPAND(ptr) *ptr, *(ptr + 1), *(ptr + 2), *(ptr + 3), *(ptr + 4), *(ptr + 5), *(ptr + 6), *(ptr + 7)


inline void prod_avx(float * x_ptr, float * y_ptr, float * out_ptr, uint64_t size) {

    static constexpr uint32_t step = 32 / sizeof(float);

    uint64_t d = 0;
    __m256 vec_x, vec_y, vec_output;
    for (; d < size - (size % step); d+=step) {        
        vec_x = _mm256_set_ps(FLOAT_VEC_EXPAND(x_ptr + d));
        vec_y = _mm256_set_ps(FLOAT_VEC_EXPAND(y_ptr + d));        
        vec_output = _mm256_mul_ps(vec_x, vec_y);
        _mm256_storeu_ps((out_ptr + d), vec_output);
    }

    if (size - d > 0) {
        float bufx[step] {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        float bufy[step] {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        for (uint64_t i=0; i < size - d; i++) {
            bufx[i] = *(x_ptr + d + i);
            bufy[i] = *(y_ptr + d + i);
        }
        vec_x = _mm256_set_ps(FLOAT_VEC_EXPAND(bufx));
        vec_y = _mm256_set_ps(FLOAT_VEC_EXPAND(bufy));    
        vec_output = _mm256_mul_ps(vec_x, vec_y);
        _mm256_storeu_ps(bufx, vec_output);
        for (uint64_t i=0; i < size - d; i++) {
            *(out_ptr + d + i) = bufx[i];
        }
    }
}


inline void prod_avx(const std::vector<float> & x, std::vector<float> & y, std::vector<float> & out) {
    static constexpr uint32_t step = 32 / sizeof(float);

    auto x_ptr = x.data();
    auto y_ptr = y.data();
    auto out_ptr = out.data();
    auto size = out.size();

    uint64_t d = 0;
    __m256 vec_x, vec_y, vec_output;
    for (; d < size - (size % step); d+=step) {        
        vec_x = _mm256_set_ps(FLOAT_VEC_EXPAND(x_ptr + d));
        vec_y = _mm256_set_ps(FLOAT_VEC_EXPAND(y_ptr + d));        
        vec_output = _mm256_mul_ps(vec_x, vec_y);
        _mm256_storeu_ps((out_ptr + d), vec_output);
    }

    if (size - d > 0) {
        float bufx[step] {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        float bufy[step] {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        for (uint64_t i=0; i < size - d; i++) {
            bufx[i] = *(x_ptr + d + i);
            bufy[i] = *(y_ptr + d + i);
        }
        vec_x = _mm256_set_ps(FLOAT_VEC_EXPAND(bufx));
        vec_y = _mm256_set_ps(FLOAT_VEC_EXPAND(bufy));    
        vec_output = _mm256_mul_ps(vec_x, vec_y);
        _mm256_storeu_ps(bufx, vec_output);
        for (uint64_t i=0; i < size - d; i++) {
            *(out_ptr + d + i) = bufx[i];
        }
    }
}



inline void prod_non_vec(float * x_ptr, float * y_ptr, float * out_ptr, uint64_t size) {

    for (uint64_t i=0; i < size; i++) {
        out_ptr[i] = x_ptr[i] * y_ptr[i];
    }
}

inline void prod_non_vec(const std::vector<float> & x, std::vector<float> & y, std::vector<float> & out) {

    auto size = out.size();
    for (uint64_t i=0; i < size; i++) {
        out[i] = x[i] * y[i];
    }
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

    static constexpr uint64_t size = 8 * 70000;
    {
        float another_x[size];
        std::cout << "align of another_x : " << alignof(another_x) << std::endl;
    }

    __attribute__ ((aligned (32))) float aligned_x[size], aligned_y[size];
    std::cout << "align of aligned_x = " << alignof(aligned_x) << std::endl;
    std::cout << "align of aligned_y = " << alignof(aligned_y) << std::endl;
    
    for (uint64_t i=0; i<size; i++) {
        aligned_x[i] = (float) i / size;
        aligned_y[i] = (float) i / size;
    }

    std::cout << "32-bit aligned aligned_x ? -> " << is_32bit_aligned__odd_impl(aligned_x) << " vs " << true << std::endl;
    std::cout << "32-bit aligned aligned_y ? -> " << is_32bit_aligned__odd_impl(aligned_y) << " vs " << true << std::endl;
    std::cout << "32-bit aligned aligned_x ? -> " << is_32bit_aligned(aligned_x) << " vs " << true << std::endl;
    std::cout << "32-bit aligned aligned_y ? -> " << is_32bit_aligned(aligned_y) << " vs " << true << std::endl;

    std::vector<float> x(aligned_x, aligned_x + size);
    std::vector<float> y(aligned_y, aligned_y + size);
    std::cout << "32-bit aligned x ? -> " << is_32bit_aligned(x.data()) << " vs " << true << std::endl;
    std::cout << "32-bit aligned y ? -> " << is_32bit_aligned(y.data()) << " vs " << true << std::endl;

    unsigned int n = 10000;
    {
        // std::vector<float> out(size);
        __attribute__ ((aligned (32))) float aligned_output[size];
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            // prod_non_vec(x, y, out);
            prod_non_vec(aligned_x, aligned_y, aligned_output, size);
            aligned_output[0]++;
        }
        auto end = std::chrono::steady_clock::now();        
        auto duration = end - timer;
        std::cout << "- prod_non_vec took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    {
        std::vector<float> out(size);
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            prod_non_vec(x, y, out);
            out[0]++;
        }
        auto end = std::chrono::steady_clock::now();        
        auto duration = end - timer;
        std::cout << "- prod_non_vec as std::vector took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    {
        // std::vector<float> out(size);
        __attribute__ ((aligned (32))) float aligned_output[size];
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            // prod_avx(x, y, out);
            prod_avx(aligned_x, aligned_y, aligned_output, size);
            aligned_output[0]++;
        }
        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- prod_avx took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    {
        std::vector<float> out(size);
        auto timer = std::chrono::steady_clock::now();
        for (uint64_t j=0; j<n; ++j)
        {
            prod_avx(x, y, out);
            out[0]++;
        }
        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- prod_avx as std::vector took " << duration.count() / n
                  << " microseconds to complete." << std::endl;
    }
    return 0;
}