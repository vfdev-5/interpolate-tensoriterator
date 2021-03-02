#!/bin/bash

set -xeu

export PR_TORCH_PATH=/workspace/pth-linear-interp
export n=20000

# # Run benchmark PyTorch nightly
version=`python -c "import torch; print(torch.__version__)"`
filepath1="pth_nightly_results_$version.log.save"
echo "" > $filepath1
python -u run_pytorch_bench.py $n 1 1 6 >> $filepath1
echo "\n\n" >> $filepath1
python -u run_pytorch_bench.py $n 1 1 1 >> $filepath1

# Run benchmark PR
version=`PYTHONPATH=$PR_TORCH_PATH python -c "import torch; print(torch.__version__)"`
filepath2="pr_results_$version.log.save"
echo $filepath2
echo "" > $filepath2
PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 6 >> $filepath2
echo "\n\n" >> $filepath2
PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 1 >> $filepath2

# Create pr_vs_pth_results.md
output="pr_${version}_vs_pth_results.md"
rm -rf $output
python make_results_table.py $output $filepath1 $filepath2
