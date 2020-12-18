cmake_minimum_required(VERSION 3.10)
project(step-one)

set(CMAKE_CXX_FLAGS "${CMAKE_CUDA_FLAGS} -O3")

if (NOT TORCH_DIR)
  message(FATAL_ERROR "Please setup TORCH_DIR")
else()
  message("Found TORCH_DIR:${TORCH_DIR}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CUDA_FLAGS} -O3")

include_directories(${TORCH_DIR}/include ${TORCH_DIR}/include/torch/csrc/api/include)
set(TORCH_LIBRARIES 
    "${TORCH_DIR}/lib/libtorch.so" 
    "${TORCH_DIR}/lib/libtorch_cpu.so" 
    "${TORCH_DIR}/lib/libc10.so"
)

add_executable(bench main.cpp interpolate.h)
target_link_libraries(bench ${TORCH_LIBRARIES})
set_property(TARGET bench PROPERTY CXX_STANDARD 14)
