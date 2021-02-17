#!/bin/bash

set -xeu

export TORCH_PATH=/pytorch/torch
export PR_TORCH_PATH=/workspace/pth-linear-interp/torch



# Build and run benchmark PTH vs This Repo
filepath=../pth_vs_this_full_results.log.save
mkdir -p build && cd build

rm -rf *
cmake -DTORCH_DIR=$TORCH_PATH ..
make
cat $TORCH_PATH/version.py > $filepath
echo "\n" >> $filepath
./bench 20000 1 1 6 >> $filepath
echo "\n\n" >> $filepath
./bench 20000 1 1 1 >> $filepath
cd ../

# Build and run benchmark PR vs This Repo
filepath=../PR_vs_this_full_results.log.save
mkdir -p new_build && cd new_build

rm -rf *
cmake -DTORCH_DIR=$PR_TORCH_PATH ..
make
cat $PR_TORCH_PATH/version.py > $filepath
echo "\n" >> $filepath
./bench 20000 1 1 6 >> $filepath
echo "\n\n" >> $filepath
./bench 20000 1 1 1 >> $filepath
cd ../



