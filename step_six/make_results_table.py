import argparse
import json
from pathlib import Path


class StopWordException(Exception):
    pass


class EndFileException(Exception):
    pass


def seek_str(handler, search_word, stop_search_words=None):
    while True:
        pos = handler.tell()
        line = handler.readline()
        if len(line) < 1:
            raise EndFileException("No more data found")        
        data = line.strip()
        if stop_search_words is not None:
            for word in stop_search_words:
                if word in data:
                    handler.seek(pos, 0)
                    raise StopWordException(f"Found stop word: {word}")
        if search_word in data:
            return data


def parse_value(data, split_char):
    assert split_char in data, f"{split_char} not in {data}"
    split = data.split(split_char)
    assert len(split) == 2, split
    return split[1].strip()


def get_value(handler, search_word, split_char, stop_search_words=None):
    data = seek_str(handler, search_word, stop_search_words=stop_search_words)
    return parse_value(data, split_char)


def get_input_tensor_data(handler):
    output = {
        "In size": json.loads(get_value(
            handler, "Input tensor", ":", 
            stop_search_words=["---- Benchmark", "END Benchmark", "PyTorch built with"]
        )),
        "Channels last": json.loads(get_value(
            handler, "Input is_contiguous memory_format torch.channels_last", ":",
            stop_search_words=["- Bench ", ]
        ).lower()),
        "Is contiguous": json.loads(get_value(
            handler, "Input is_contiguous", ":",
            stop_search_words=["- Bench ", ]
        ).lower()),
    }
    return output


def get_measure(handler, op_name):

    osize = get_value(
        handler, f"- Bench {op_name}", " to ", 
        stop_search_words=["Input tensor", "END Benchmark", "PyTorch built with"]
    )
    if "x" in osize:
        osize = [int(v) for v in osize.split("x")]
    else:
        osize = json.loads(osize)
    elapsed = float(get_value(handler, "Elapsed time (ms)", ":"))
    output = {
        "Out size": osize,
        "Time (ms)": elapsed
    }
    return output


def get_bench_results(handler, op_name, debug=False):
    bench_res = []
    while True:
        try:
            input_data = get_input_tensor_data(handler)
            while True:
                res = dict(input_data)
                try:
                    res.update(get_measure(handler, op_name=op_name))
                except StopWordException as e1:
                    if debug: print("Measure:", e1)
                    break
                bench_res.append(res)
        except StopWordException as e2:
            if debug: print("Input: ", e2)
            break
    return bench_res


def parse_bench_file(filepath):
    output = {}
    with open(filepath, "r") as handler:
        output["version"] = get_value(handler, "__version__", "=")
        output["build_config"] = get_value(handler, "- Build settings:", ":")
        output["runs"] = []
        while True:
            try:
                run_output = {}
                run_output["num_threads"] = int(get_value(handler, "Num threads", ":"))

                _ = seek_str(handler, "---- Benchmark 2D")
                bench_2d = get_bench_results(handler, "upsample_bilinear2d")
                run_output["bench_2d"] = bench_2d

                _ = seek_str(handler, "---- Benchmark 1D")
                bench_1d = get_bench_results(handler, "upsample_linear1d")
                run_output["bench_1d"] = bench_1d

                _ = seek_str(handler, "---- Benchmark 3D")
                bench_3d = get_bench_results(handler, "upsample_trilinear3d")
                run_output["bench_3d"] = bench_3d
                output["runs"].append(run_output)
            except EndFileException:
                break

    return output


table_template = """\

<details>
<summary>
Interpolation {nd_case} - {num_threads} thread(s)
</summary>


In | Out | Is contiguous | Channels last | master | this PR | speed-up
---|---|---|---|---|---|---
{table}

</details>
"""


def create_table(nd_case, num_threads, master_bench_res, pr_bench_res):
    output = ""
    checked_keys = ["In size", "Out size", "Is contiguous", "Channels last"]
    for master_res, pr_res in zip(master_bench_res, pr_bench_res):
        t = "{In size} | {Out size} | {Is contiguous} | {Channels last} | {master:.4f} | {pr:.4f} | {speed_up:.4f}\n"
        assert all([master_res[k] == pr_res[k] for k in checked_keys]), f"{master_res} vs {pr_res}"
        speed_up = master_res["Time (ms)"] / pr_res["Time (ms)"]
        master = master_res["Time (ms)"]
        pr = pr_res["Time (ms)"]
        output += t.format(**master_res, speed_up=speed_up, master=master, pr=pr)
    
    return table_template.format(nd_case=nd_case, num_threads=num_threads, table=output)


tables_template = """
{result}

<details>
<summary>
Versions and build configs
</summary>

PyTorch master: {pth_version}
PyTorch master build setting:
```
{pth_build_config}
```

PR : {pr_version}
PR build setting:
```
{pr_build_config}
```
</details>
"""


def to_markdown(pth_output, pr_output):
    
    result = ""
    iterated_keys = ["bench_2d", "bench_1d", "bench_3d"]
    for pth_run, pr_run in zip(pth_output["runs"], pr_output["runs"]):
        assert pth_run["num_threads"] == pr_run["num_threads"]
        for key in iterated_keys:
            nd_case = key.replace("bench_", "")
            result += create_table(
                nd_case,
                pth_run["num_threads"],
                pth_run[key],
                pr_run[key],        
            )
    
    output = tables_template.format(
        pth_version=pth_output["version"],
        pth_build_config=pth_output["build_config"],      
        pr_version=pr_output["version"],
        pr_build_config=pr_output["build_config"],      
        result=result
    )
    return output


if __name__ == "__main__":
    
    parser = argparse.ArgumentParser("Tool to create a table with results as markdown file")
    parser.add_argument("output", default=str, help="Output filename, e.g. output.md")
    parser.add_argument("inputs", default=str, nargs=2, help="Input master and PR .log.save files")

    args = parser.parse_args()
    assert not Path(args.output).exists(), f"{args.output} should not exist"

    for in_filepath in args.inputs:
        assert Path(in_filepath).exists(), f"{in_filepath} is not found"

    master_bench_filepath = args.inputs[0]
    pr_bench_filepath = args.inputs[1]

    master_benchs = parse_bench_file(master_bench_filepath)
    pr_benchs = parse_bench_file(pr_bench_filepath)

    md_result = to_markdown(master_benchs, pr_benchs)

    with open(args.output, "w") as handler:
        handler.write(md_result)
