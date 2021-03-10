import argparse
import pickle
import json
from pathlib import Path

import torch.utils.benchmark as benchmark


if __name__ == "__main__":
    
    parser = argparse.ArgumentParser("Tool to create a table with results as markdown file")
    parser.add_argument("output", default=str, help="Output filename, e.g. output.md")
    parser.add_argument("inputs", default=str, nargs='+', help="Input pickle files")

    args = parser.parse_args()
    assert not Path(args.output).exists(), f"{args.output} should not exist"

    ab_results = []
    for in_filepath in args.inputs:
        assert Path(in_filepath).exists(), f"{in_filepath} is not found"
        with open(in_filepath, "rb") as handler:
            ab_results.extend(pickle.load(handler))

    compare = benchmark.Compare(ab_results)

    with open(args.output, "w") as handler:
        handler.write(str(compare))
