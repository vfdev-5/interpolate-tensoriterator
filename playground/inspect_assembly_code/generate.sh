#!/bin/bash

output_name=${1:-main.list}

echo "\n\n--- Generate ${output_name} ---"

g++ -O3 -mavx -mfma -mavx2 -S -fverbose-asm -fopt-info-vec-missed -fopt-info-vec -g main.cpp -o main.s
as -alhnd main.s > ${output_name}
