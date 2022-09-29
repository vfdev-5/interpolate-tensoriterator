#!/bin/bash

set -xeu

export MASTER_TORCH_PATH=/pytorch
export PR_TORCH_PATH=/workspace/pth-linear-interp
export min_run_time=2

custom_tests="linear:3dcf"
# custom_tests="2dcf 2dcl 3dcf 3dcl"
# custom_tests="2dcf 3dcf 1d"
# custom_tests="all"

prefix=`date "+%Y%m%d-%H%M%S"`
postfix=".linear-spec-case"

output_folder="results"
mkdir -p ${output_folder}


echo "Run benchmark PyTorch nightly"
version1=`PYTHONPATH=$MASTER_TORCH_PATH python -c "import torch; print(torch.__version__)"`
filepath1="${output_folder}/${prefix}_pth_nightly_results_$version1.log.save$postfix"
pickle_filepath1="${output_folder}/${prefix}_pth_nightly_results_$version1$postfix"

echo "" > $filepath1
export OMP_NUM_THREADS=6
PYTHONPATH=$MASTER_TORCH_PATH python -u run_pytorch_native_bench.py $pickle_filepath1.$OMP_NUM_THREADS.pickle $min_run_time 1 1 --test_cases $custom_tests >> $filepath1

echo "\n\n" >> $filepath1
export OMP_NUM_THREADS=1
PYTHONPATH=$MASTER_TORCH_PATH python -u run_pytorch_native_bench.py $pickle_filepath1.$OMP_NUM_THREADS.pickle $min_run_time 1 1 --test_cases $custom_tests >> $filepath1

echo "Run benchmark PR"
version2=`PYTHONPATH=$PR_TORCH_PATH python -c "import torch; print(torch.__version__)"`
filepath2="${output_folder}/${prefix}_pr_results_$version2.log.save$postfix"
pickle_filepath2="${output_folder}/${prefix}_pr_results_$version2$postfix"
echo $filepath2

echo "" > $filepath2
export OMP_NUM_THREADS=6
PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_native_bench.py $pickle_filepath2.$OMP_NUM_THREADS.pickle $min_run_time 1 1 --test_cases $custom_tests --with_opencv >> $filepath2

echo "\n\n" >> $filepath2
export OMP_NUM_THREADS=1
PYTHONPATH=$PR_TORCH_PATH python -u run_pytorch_native_bench.py $pickle_filepath2.$OMP_NUM_THREADS.pickle $min_run_time 1 1 --test_cases $custom_tests --with_opencv >> $filepath2

# Create pr_vs_pth_results.md
output="${output_folder}/${prefix}_pr_${version2}_vs_pth_${version1}_results${postfix}.md"
rm -rf $output
PYTHONPATH=$MASTER_TORCH_PATH python make_results_table_from_pickles.py $output $pickle_filepath1.6.pickle $pickle_filepath1.1.pickle $pickle_filepath2.6.pickle $pickle_filepath2.1.pickle

echo "\n\nIntermediate benchmark sources:\n" >> $output
echo "- $filepath1" >> $output
echo "- $filepath2" >> $output
