// STD
#include <iostream>
#include <chrono>
#include <thread>

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
    
  scalar_t acc_arr[Vec::size()];
  acc_vec.store(acc_arr);
  for (int64_t i = 1; i < size; i++) {
    std::array<scalar_t, Vec::size()> acc_arr_next = {0};
    acc_arr_next[0] = acc_arr[i];
    Vec acc_vec_next = Vec::loadu(acc_arr_next.data());
    acc_vec = vec_fun(acc_vec, acc_vec_next);
  }
  acc_vec.store(acc_arr);
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


int main(int argc, char** argv)
{

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

    return 0;
}