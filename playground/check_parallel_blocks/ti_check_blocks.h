#include <vector>
#include <ATen/native/TensorIterator.h>


at::Tensor ti_check_blocks(const at::Tensor& input)
{

    auto iter = at::TensorIteratorConfig()
        .add_output(at::Tensor())
        .add_input(input)
        .build();

    auto test_loop = [&](char **data, const int64_t* strides, int64_t n) {

        std::cout << "n : " << n << std::endl;
        
        auto * out_data = data[0];
        auto * in_data = data[1];
        
        for (int i = 0; i < n; i++) {
            
            float * in_data_f = reinterpret_cast<float*>(in_data);
            *reinterpret_cast<float*>(out_data) = (*in_data_f);
            
            out_data += strides[0];
            in_data += strides[1];
            
        }
    };

    iter.for_each(test_loop);

    return iter.output();
}
