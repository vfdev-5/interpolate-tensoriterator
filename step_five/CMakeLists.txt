cmake_minimum_required(VERSION 3.10)
project(step-five)

if (NOT TORCH_DIR)
  message(FATAL_ERROR "Please setup TORCH_DIR")
else()
  message("Found TORCH_DIR:${TORCH_DIR}")
endif()

if (WITH_ASAN)  
  message("With ASAN support")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -static-libsan")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mavx -mfma -mavx2")
endif()

include_directories(${TORCH_DIR}/include ${TORCH_DIR}/include/torch/csrc/api/include)
set(TORCH_LIBRARIES 
    "${TORCH_DIR}/lib/libtorch.so" 
    "${TORCH_DIR}/lib/libtorch_cpu.so" 
    "${TORCH_DIR}/lib/libc10.so"
)

# Optionally, include and link with OpenCV if provided
if(OPENCV_INCLUDE_DIR AND OPENCV_LIB_DIR)
  message("Found OPENCV_INCLUDE_DIR:${OPENCV_INCLUDE_DIR}")
  message("Found OPENCV_LIB_DIR:${OPENCV_LIB_DIR}")

  include_directories(${OPENCV_INCLUDE_DIR})
  set(OPENCV_LIBRARIES
    "${OPENCV_LIB_DIR}/libopencv_core.so"
    "${OPENCV_LIB_DIR}/libopencv_imgproc.so"
  )

  add_definitions(-DWITH_OPENCV)
endif()


find_package(OpenMP)

add_executable(bench main.cpp interpolate.h)
target_link_libraries(bench PUBLIC ${TORCH_LIBRARIES} ${OPENCV_LIBRARIES} OpenMP::OpenMP_CXX)
set_property(TARGET bench PROPERTY CXX_STANDARD 14)
