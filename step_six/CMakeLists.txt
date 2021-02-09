cmake_minimum_required(VERSION 3.10)
project(step-six)

if (NOT TORCH_DIR)
  message(FATAL_ERROR "Please setup TORCH_DIR")
else()
  message("Found TORCH_DIR:${TORCH_DIR}")
endif()

if (WITH_ASAN)  
  message("With ASAN support")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -static-libasan")
  add_definitions(-DWITH_ASAN)
else()
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -mavx -mfma -mavx2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -fvisibility-inlines-hidden -fopenmp -DNDEBUG -O2 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow")
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
target_link_libraries(bench PUBLIC ${TORCH_LIBRARIES} ${OPENCV_LIBRARIES})
if (OpenMP_C_FOUND)
  target_link_libraries(bench PUBLIC OpenMP::OpenMP_C)
endif()
if (OpenMP_CXX_FOUND)
  target_link_libraries(bench PUBLIC OpenMP::OpenMP_CXX)
endif()
set_property(TARGET bench PROPERTY CXX_STANDARD 14)
