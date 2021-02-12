#!/bin/bash

echo "\n\n--- Generate main_proto.lst ---"

g++ -O3 -mavx -mfma -mavx2 -S -fverbose-asm -g main_proto.cpp -I${PWD} -I${TORCH_PATH}/include -I${TORCH_PATH}/include/torch/csrc/api/include -L${TORCH_PATH}/lib/ -ltorch -ltorch_cpu -lc10 -o main_proto.s
as -alhnd main_proto.s > main_proto.lst
