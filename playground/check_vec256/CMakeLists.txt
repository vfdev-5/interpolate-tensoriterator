cmake_minimum_required(VERSION 3.10)
project(check-vec256)

if (NOT TORCH_DIR)
  message(FATAL_ERROR "Please setup TORCH_DIR")
else()
  message("Found TORCH_DIR:${TORCH_DIR}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CUDA_FLAGS} -mavx -mavx2 -mfma -O2")
add_definitions("-DCPU_CAPABILITY_AVX2")

include_directories(${TORCH_DIR}/include ${TORCH_DIR}/include/torch/csrc/api/include)
set(TORCH_LIBRARIES 
    "${TORCH_DIR}/lib/libtorch.so" 
    "${TORCH_DIR}/lib/libtorch_cpu.so" 
    "${TORCH_DIR}/lib/libc10.so"
)

find_package(OpenMP)

add_executable(bench main.cpp)
target_link_libraries(bench PUBLIC ${TORCH_LIBRARIES} OpenMP::OpenMP_CXX)
set_property(TARGET bench PROPERTY CXX_STANDARD 14)
