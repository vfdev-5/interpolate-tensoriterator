#!/bin/bash

output_name=${1:-main.list}

mkdir -p build_assembly

echo "\n\n--- Generate ${output_name} ---"

export TORCH_DIR=/workspace/pth-linear-interp/torch

set -xeu

pth_cxx_flags="-mavx -mfma -mavx2 -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store -Wno-deprecated -fvisibility-inlines-hidden -fopenmp -DNDEBUG -O3 -fPIC -Wno-narrowing -Wall -Wextra -Werror=return-type -Wno-missing-field-initializers -Wno-type-limits -Wno-array-bounds -Wno-unknown-pragmas -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-result -Wno-unused-local-typedefs -Wno-strict-overflow -Wno-strict-aliasing -Wno-error=deprecated-declarations -Wno-stringop-overflow -Wno-psabi -Wno-error=pedantic -Wno-error=redundant-decls -Wno-error=old-style-cast -fdiagnostics-color=always -faligned-new -Wno-unused-but-set-variable -Wno-maybe-uninitialized -fno-math-errno -fno-trapping-math -Werror=format -Werror=cast-function-type -Wno-stringop-overflow"
cxx_flags="-S -fverbose-asm -g ${pth_cxx_flags}"
include_dirs="-I. -I${TORCH_DIR}/include -I${TORCH_DIR}/include/torch/csrc/api/include"
libs="-L${TORCH_DIR}/lib/ -ltorch -ltorch_cpu -lc10"

# g++ ${cxx_flags} ${include_dirs} ${libs} interpolate.cpp -o build_assembly/interpolate.o
g++ ${cxx_flags} ${include_dirs} ${libs} main.cpp -o build_assembly/main.s
as -alhnd build_assembly/main.s > build_assembly/${output_name}
