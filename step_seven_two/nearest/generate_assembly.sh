#!/bin/bash

input_filename=${1:-interpolate.cpp}
output_name=${2:-${input_filename}.list}

echo "\n\n--- Generate ${output_name} ---"

g++ -O3 -mavx -mfma -mavx2 -mno-avx256-split-unaligned-load -mno-avx256-split-unaligned-store \
    -S -fverbose-asm -fopt-info-vec-missed -fopt-info-vec -g ${input_filename} \
    -I. -I${TORCH_PATH}/include -I${TORCH_PATH}/include/torch/csrc/api/include \
    -L${TORCH_PATH}/lib/ -ltorch -ltorch_cpu -lc10 -o build/${input_filename}.s &> gen_assembly_${input_filename}.log

sed -e "/${input_filename}/!d" -i gen_assembly_${input_filename}.log

as -alhnd build/${input_filename}.s > build/${output_name}

rm -rf a.out