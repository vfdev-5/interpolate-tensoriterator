#!/bin/bash

echo "\n\n--- Generate main_pr.lst ---"

g++ -O3 -mavx -mfma -mavx2 -S -fverbose-asm -g main_pr.cpp -I${PR_TORCH_PATH}/include -I${TORCH_PATH}/include/torch/csrc/api/include -L${PR_TORCH_PATH}/lib/ -ltorch -ltorch_cpu -lc10 -o main_pr.s
as -alhnd main_pr.s > main_pr.lst
