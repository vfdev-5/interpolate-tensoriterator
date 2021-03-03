#!/bin/bash

set -xeu

export PR_TORCH_PATH=/workspace/pth-linear-interp
export n=20000

# optional filename suffix
postfix=".2"

# # Run benchmark PyTorch nightly
version1=`python -c "import torch; print(torch.__version__)"`
filepath1="pth_nightly_results_$version1.log.save$postfix"
echo "" > $filepath1
python -u run_pytorch_bench.py $n 1 1 6 >> $filepath1
# echo "\n\n" >> $filepath1
# python -u run_pytorch_bench.py $n 1 1 1 >> $filepath1

# Run benchmark PR
version2=`PYTHONPATH=$PR_TORCH_PATH python -c "import torch; print(torch.__version__)"`
filepath2="pr_results_$version2.log.save$postfix"
echo $filepath2
echo "" > $filepath2
PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 6 >> $filepath2
# echo "\n\n" >> $filepath2
# PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 1 >> $filepath2

# Create pr_vs_pth_results.md
output="pr_${version2}_vs_pth_${version1}_results.md$postfix"
rm -rf $output
python make_results_table.py $output $filepath1 $filepath2
