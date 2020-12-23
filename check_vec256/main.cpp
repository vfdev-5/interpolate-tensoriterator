// STD
#include <iostream>
#include <chrono>
#include <thread>

// AVX
#include <immintrin.h>

// Torch
#include <ATen/ATen.h>
#include <ATen/cpu/vec256/vec256.h>



using namespace std::chrono_literals;


template <typename scalar_t, typename Op>
inline scalar_t vec_reduce_all(
    const Op& vec_fun,
    at::vec256::Vec256<scalar_t> acc_vec,
    int64_t size) {

  using Vec = at::vec256::Vec256<scalar_t>;
    
  auto sz = Vec::size();
  scalar_t acc_arr[sz];
  acc_vec.store(acc_arr, sz);

  for (int64_t i = 1; i < size; i++) {
    std::array<scalar_t, sz> acc_arr_next = {0};
    acc_arr_next[0] = acc_arr[i];
    Vec acc_vec_next = Vec::loadu(acc_arr_next.data());
    acc_vec = vec_fun(acc_vec, acc_vec_next);
  }

  acc_vec.store(acc_arr, sz);
  return acc_arr[0];
}


template <typename scalar_t, typename Op>
inline scalar_t reduce_all(const Op& vec_fun, const scalar_t* data, int64_t size) {

  using Vec = at::vec256::Vec256<scalar_t>;

  if (size < Vec::size())
    return vec_reduce_all(vec_fun, Vec::loadu(data, size), size);

  int64_t d = Vec::size();
  Vec acc_vec = Vec::loadu(data);
  for (; d < size - (size % Vec::size()); d += Vec::size()) {
    Vec data_vec = Vec::loadu(data + d);
    acc_vec = vec_fun(acc_vec, data_vec);
  }
  if (size - d > 0) {
    Vec data_vec = Vec::loadu(data + d, size - d);
    acc_vec = Vec::set(acc_vec, vec_fun(acc_vec, data_vec), size - d);
  }
  return vec_reduce_all(vec_fun, acc_vec, Vec::size());
}


at::Tensor prod_vec256(const at::Tensor & x, const at::Tensor & y) {
    using Vec = at::vec256::Vec256<float>;
    auto x_ptr = (float *) x.data_ptr();
    auto y_ptr = (float *) x.data_ptr();

    auto output = at::empty_like(x);
    auto out_ptr = (float *) output.data_ptr();

    auto size = x.numel();
    auto step = Vec::size();
    int64_t d = 0;
    Vec vec_x, vec_y, vec_output;
    for (; d < size - (size % step); d+=step) {
        vec_x = Vec::loadu(x_ptr + d, step);
        vec_y = Vec::loadu(y_ptr + d, step);
        vec_output = vec_x * vec_y;
        vec_output.store(out_ptr + d, step);
    }

    if (size - d > 0) {
        vec_x = Vec::loadu(x_ptr + d, size - d);
        vec_y = Vec::loadu(y_ptr + d, size - d);
        vec_output = vec_x * vec_y;
        vec_output.store(out_ptr + d, size - d);
    }
    return output;
}


at::Tensor prod_avx(const at::Tensor & x, const at::Tensor & y) {

    static constexpr uint32_t step = 32 / sizeof(float);

    auto x_ptr = (float *) x.data_ptr();
    auto y_ptr = (float *) x.data_ptr();

    auto output = at::empty_like(x);
    auto out_ptr = (float *) output.data_ptr();

    auto size = x.numel();    
    int64_t d = 0;
    __m256 vec_x, vec_y, vec_output;
    for (; d < size - (size % step); d+=step) {        
        vec_x = _mm256_loadu_ps(x_ptr + d);
        vec_y = _mm256_loadu_ps(y_ptr + d);
        vec_output = _mm256_mul_ps(vec_x, vec_y);
        _mm256_storeu_ps((out_ptr + d), vec_output);
    }

    if (size - d > 0) {

        float bufx[step] {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
        float bufy[step] {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

        for (int i=0; i < size - d; i++) {
            bufx[i] = *(x_ptr + d + i);
            bufy[i] = *(y_ptr + d + i);
        }
        vec_x = _mm256_loadu_ps(bufx);
        vec_y = _mm256_loadu_ps(bufy);
        vec_output = _mm256_mul_ps(vec_x, vec_y);
        _mm256_storeu_ps(bufx, vec_output);
        for (int i=0; i < size - d; i++) {
            *(out_ptr + d + i) = bufx[i];
        }
        
    }
    return output;
}


at::Tensor prod_non_vec(const at::Tensor & x, const at::Tensor & y) {

    auto x_ptr = (float *) x.data_ptr();
    auto y_ptr = (float *) x.data_ptr();

    auto output = at::empty_like(x);
    auto out_ptr = (float *) output.data_ptr();

    auto size = x.numel();
    int64_t d = 0;
    for (; d < size; d++) {
        *(float *)(out_ptr + d) = *(float *)(x_ptr + d) * *(float *)(y_ptr + d);
    }
    return output;
}


bool is_32bit_aligned(void * ptr) {
    // http://www.cplusplus.com/forum/beginner/249090/
    return ((((intptr_t) ptr + 31) & ~ intptr_t(31)) == (intptr_t) ptr);
}



int main(int argc, char** argv)
{

#if 0
    std::cout << "- at::vec256::Vec256<float>::size() = " << at::vec256::Vec256<float>::size() << std::endl;
    std::cout << "- at::vec256::Vec256<int>::size() = " << at::vec256::Vec256<int>::size() << std::endl;
    std::cout << "- at::vec256::Vec256<double>::size() = " << at::vec256::Vec256<double>::size() << std::endl;
    std::cout << "- at::vec256::Vec256<uint8_t>::size() = " << at::vec256::Vec256<uint8_t>::size() << std::endl;

    auto sz = at::vec256::Vec256<float>::size();
    float acc_arr[sz];
    at::vec256::Vec256<float> acc_vec {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    std::cout << "- before: " << acc_vec << std::endl;
    for (int i=0; i<sz; i++)
        std::cout << acc_arr[i] << " ";
    std::cout << std::endl;
    acc_vec.store(acc_arr, sz);
    std::cout << "- after: " << acc_vec << std::endl;
    for (int i=0; i<sz; i++)
        std::cout << acc_arr[i] << " ";
    std::cout << std::endl;

#endif

    int64_t size = 32 * 10000;

#if 1
    std::cout << "Setup input with linspace" << std::endl;
    auto x = at::linspace(-1.0, 1.0, size, at::CPU(at::kFloat));
    auto y = at::linspace(-1.0, 1.0, size, at::CPU(at::kFloat));
#else
    std::cout << "Setup input using 32-bit aligned buffers" << std::endl;
    __attribute__ ((aligned (32))) float aligned_x[size], aligned_y[size];
    for (unsigned int i=0; i<size; i++) {
        aligned_x[i] = (float) i / size;
        aligned_y[i] = (float) i / size;
    }
    std::cout << "align of aligned_x = " << alignof(aligned_x) << std::endl;
    std::cout << "align of aligned_y = " << alignof(aligned_y) << std::endl;
    std::cout << "32-bit aligned aligned_x ? -> " << is_32bit_aligned(aligned_x) << " vs " << true << std::endl;
    std::cout << "32-bit aligned aligned_y ? -> " << is_32bit_aligned(aligned_y) << " vs " << true << std::endl;

    auto x = at::from_blob(aligned_x, size, at::CPU(at::kFloat));
    auto y = at::from_blob(aligned_y, size, at::CPU(at::kFloat));    
#endif

    std::cout << "32-bit aligned x ? -> " << is_32bit_aligned(x.data_ptr()) << std::endl;
    std::cout << "32-bit aligned y ? -> " << is_32bit_aligned(y.data_ptr()) << std::endl;

    auto z = prod_vec256(x, y);
    assert (z.allclose(x * y));

    z = prod_avx(x, y);
    assert (z.allclose(x * y));

    z = prod_non_vec(x, y);
    assert (z.allclose(x * y));

    int n = 10000;
    {
        at::Tensor z;
        auto timer = std::chrono::steady_clock::now();        
        for (int j=0; j<n; ++j)
        {
            z = prod_non_vec(x, y);
            z[0] += 1.0;
        }

        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- prod_non_vec(x, y) took " << duration.count() / n * 0.001
                  << " ms to complete." << std::endl;
    }
    {
        at::Tensor z;
        auto timer = std::chrono::steady_clock::now();
        for (int j=0; j<n; ++j)
        {
            z = prod_avx(x, y);
            z[0] += 1.0;
        }

        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- prod_avx(x, y) took " << duration.count() / n * 0.001
                  << " ms to complete." << std::endl;
    }
    {
        at::Tensor z;
        auto timer = std::chrono::steady_clock::now();
        for (int j=0; j<n; ++j)
        {
            z = prod_vec256(x, y);
            z[0] += 1.0;
        }

        auto end = std::chrono::steady_clock::now();
        auto duration = end - timer;
        std::cout << "- prod_vec256(x, y) took " << duration.count() / n * 0.001
                  << " ms to complete." << std::endl;
    }

#if 0
    auto t_input = at::arange(0, 100, at::CPU(at::kFloat));
    std::cout << t_input.dtype() << std::endl;
    std::cout << t_input.sum() << std::endl;

    using Vecf = at::vec256::Vec256<float>;

    auto res = reduce_all<float>(
        [](Vecf& x, Vecf& y) { return x + y; },
        (float *) t_input.data_ptr(),
        100
    );

    std::cout << res << std::endl;
#endif

    return 0;    
}