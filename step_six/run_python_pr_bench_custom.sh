#!/bin/bash

set -xeu

export MASTER_TORCH_PATH=/pytorch
export PR_TORCH_PATH=/workspace/pth-linear-interp
export n=20000

custom_tests="2dcf 2dcl"
PYTHONPATH=$MASTER_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 6 --custom $custom_tests
exit 1

# optional filename suffix
postfix=".3"

# # Run benchmark PyTorch nightly
version1=`PYTHONPATH=$MASTER_TORCH_PATH python -c "import torch; print(torch.__version__)"`
filepath1="pth_nightly_results_$version1.log.save$postfix"
echo "" > $filepath1
PYTHONPATH=$MASTER_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 6 --custom $custom_tests >> $filepath1
# echo "\n\n" >> $filepath1
# PYTHONPATH=$MASTER_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 1 --custom $custom_tests >> $filepath1

# Run benchmark PR
version2=`PYTHONPATH=$PR_TORCH_PATH python -c "import torch; print(torch.__version__)"`
filepath2="pr_results_$version2.log.save$postfix"
echo $filepath2
echo "" > $filepath2
PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 6 --custom $custom_tests >> $filepath2
# echo "\n\n" >> $filepath2
# PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_bench.py $n 1 1 1 --custom $custom_tests >> $filepath2

# Create pr_vs_pth_results.md
output="pr_${version2}_vs_pth_${version1}_results.md$postfix"
rm -rf $output
python make_results_table.py $output $filepath1 $filepath2

echo "\n\nIntermediate benchmark sources:\n" >> $output
echo "- $filepath1" >> $output
echo "- $filepath2" >> $output