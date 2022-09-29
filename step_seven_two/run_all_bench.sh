#!/bin/bash

set -xeu

export TORCH_PATH=/workspace/pth-linear-interp/torch

# Build and run benchmark PTH vs This Repo
filepath=$PWD/pth_vs_this_full_results.log.save
echo "" > $filepath

n_cubic=1200
n_linear=12000
n_nearest=12000




echo "" >> $filepath
echo "-----------------------" >> $filepath
echo "--- Benchmark cubic ---" >> $filepath
echo "-----------------------" >> $filepath
echo "" >> $filepath

cd cubic
mkdir -p build && cd build
rm -rf *

cmake -DTORCH_DIR=$TORCH_PATH ..
make
cat $TORCH_PATH/version.py >> $filepath
echo "\n" >> $filepath
./bench $n_cubic 1 1 6 >> $filepath
cd ../../


echo "" >> $filepath
echo "------------------------" >> $filepath
echo "--- Benchmark linear ---" >> $filepath
echo "------------------------" >> $filepath
echo "" >> $filepath

cd linear
mkdir -p build && cd build
rm -rf *

cmake -DTORCH_DIR=$TORCH_PATH ..
make
cat $TORCH_PATH/version.py >> $filepath
echo "\n" >> $filepath
./bench $n_linear 1 1 6 >> $filepath
cd ../../



echo "" >> $filepath
echo "-------------------------" >> $filepath
echo "--- Benchmark nearest ---" >> $filepath
echo "-------------------------" >> $filepath
echo "" >> $filepath

cd nearest
mkdir -p build && cd build
rm -rf *

cmake -DTORCH_DIR=$TORCH_PATH ..
make
cat $TORCH_PATH/version.py >> $filepath
echo "\n" >> $filepath
./bench $n_nearest 1 1 6 >> $filepath
cd ../../
