#
# export PR_TORCH_PATH=/workspace/pth-linear-interp/
# sh run_python_pr_bench.sh
# > pr_vs_pth_results.md
#
import pickle
import argparse
import time
from itertools import product

import torch
import torch._C as C

import torch.utils.benchmark as benchmark

test_results = []
env = ""

def sub_bench_Nd_single_op(mode, n, min_run_time, t_input, output_size):
    if n == 3:
        mem_format = "channels_last" if t_input.is_contiguous(memory_format=torch.channels_last_3d) else "channels_first"
    else:
        mem_format = "channels_last" if t_input.is_contiguous(memory_format=torch.channels_last) else "channels_first"
    is_contiguous = "contiguous" if t_input.is_contiguous() else "non-contiguous"
    num_threads = torch.get_num_threads()

    ndim_prefix = ""
    if n == 2:
        ndim_prefix = "bi"
    elif n == 3:
        ndim_prefix = "tri"

    mode_Nd_fn_map = {
        "linear": f"upsample_{ndim_prefix}linear{n}d",
        "nearest": f"upsample_nearest{n}d",
        "cubic": f"upsample_{ndim_prefix}cubic{n}d",
    }

    func_name = mode_Nd_fn_map[mode]
    align_corners = "" if mode == "near" else False

    m = benchmark.Timer(
        stmt=f"{func_name}(t_input, output_size, {align_corners})",
        globals={
            't_input': t_input, 
            'output_size': output_size,
            func_name: getattr(C._nn, func_name)
        },
        num_threads=num_threads,
        label=f"{func_name} {mem_format} {is_contiguous}",
        sub_label=f"{list(t_input.shape)} -> {output_size}",
        description=f"{torch.version.__version__}",
    ).blocked_autorange(min_run_time=min_run_time)
    print(m)
    test_results.append(m)


def sub_bench_2d_single_op(mode, min_run_time, t_input, output_size):
    sub_bench_Nd_single_op(mode, 2, min_run_time, t_input, output_size)


def sub_bench_2d(mode, min_run_time, t_input, dn_osize, up_osize):
    print(f"\nInput tensor: {list(t_input.shape)}")
    print(f"Input is_contiguous memory_format torch.channels_last: {t_input.is_contiguous(memory_format=torch.channels_last)}")
    print(f"Input is_contiguous : {t_input.is_contiguous()}")

    print(f"\n- Bench upsample {mode} 2d ({min_run_time} min_run_time) - downsampling to {dn_osize}x{dn_osize}")
    sub_bench_2d_single_op(mode, min_run_time, t_input, (dn_osize, dn_osize))

    print(f"\n- Bench upsample {mode} 2d ({min_run_time} min_run_time) - upsampling to {up_osize}x{up_osize}")
    sub_bench_2d_single_op(mode, min_run_time, t_input, (up_osize, up_osize))


def sub_bench_2d_non_contiguous_channel_first(mode, min_run_time, isize, dn_osize, up_osize):
    t_input = torch.rand(1, 3, isize + 100, isize + 100, dtype=torch.float, device="cpu")
    t_input = t_input[..., :isize, :isize]
    sub_bench_2d(mode, min_run_time, t_input, dn_osize, up_osize)


def sub_bench_2d_contiguous_channel_first(mode, min_run_time, isize, dn_osize, up_osize):
    t_input = torch.rand(1, 3, isize, isize, dtype=torch.float, device="cpu")
    sub_bench_2d(mode, min_run_time, t_input, dn_osize, up_osize)


def sub_bench_2d_mingfeima_channel_last(mode, min_run_time):
    print(f"\n1 - Test size as in https://github.com/mingfeima/op_bench-py")
    t_input = torch.rand(32, 64, 64, 128, dtype=torch.float, device="cpu")
    t_input = t_input.permute(0, 3, 1, 2)
    sub_bench_2d(mode, min_run_time, t_input, 32, 128)

    print(f"\n1.2 - Test sizes similar to https://github.com/pytorch/pytorch/blob/master/benchmarks/operator_benchmark/pt/interpolate_test.py")
    t_input = torch.rand(2, 64, 46, 128, dtype=torch.float, device="cpu")
    t_input = t_input.permute(0, 3, 1, 2)
    sub_bench_2d(mode, min_run_time, t_input, 32, 128)


def sub_bench_2d_mingfeima_channel_first(mode, min_run_time):
    print(f"\n2 - Test size as in https://github.com/mingfeima/op_bench-py")
    t_input = torch.rand(32, 128, 64, 64, dtype=torch.float, device="cpu")
    sub_bench_2d(mode, min_run_time, t_input, 32, 128)


def sub_bench_2d_non_contiguous_channel_last(mode, min_run_time, isize, dn_osize, up_osize, n_channels=3):
    t_input = torch.rand(1, isize, isize, n_channels, dtype=torch.float, device="cpu")
    t_input = t_input.permute(0, 3, 1, 2)
    sub_bench_2d(mode, min_run_time, t_input, dn_osize, up_osize)


def is_in(case, test_cases):
    return any([case in test_case for test_case in test_cases])


def bench_2d(mode, min_run_time, full_bench, isize, dn_osize, up_osize, test_cases):
    if not any(is_in(k, test_cases) for k in ["2dcf", "2dcl"]):
        return

    if is_in("2dcf", test_cases):
        sub_bench_2d_contiguous_channel_first(mode, min_run_time, isize, dn_osize, up_osize)
        sub_bench_2d_non_contiguous_channel_first(mode, min_run_time, isize, dn_osize, up_osize)

    if is_in("2dcl", test_cases):
        sub_bench_2d_non_contiguous_channel_last(mode, min_run_time, isize, dn_osize, up_osize)

    if not full_bench:
        return

    if is_in("2dcl", test_cases):
        sub_bench_2d_mingfeima_channel_last(mode, min_run_time)

    if is_in("2dcf", test_cases):
        sub_bench_2d_mingfeima_channel_first(mode, min_run_time)


def sub_bench_1d_single_op(mode, min_run_time, t_input, output_size):
    sub_bench_Nd_single_op(mode, 1, min_run_time, t_input, output_size)


def bench_1d(mode, min_run_time, full_bench, test_cases):
    if not any(is_in(k, test_cases) for k in ["1d", ]):
        return

    t_input = torch.rand(4, 512, 320, dtype=torch.float, device="cpu")
    print(f"\nInput tensor: {list(t_input.shape)}")
    print(f"Input is_contiguous memory_format torch.channels_last: {t_input.is_contiguous(memory_format=torch.channels_last)}")
    print(f"Input is_contiguous : {t_input.is_contiguous()}")

    print(f"\n- Bench upsample {mode} 1d ({min_run_time} min_run_time) - downsampling to 256")
    sub_bench_1d_single_op(mode, min_run_time, t_input, [256, ])

    output_size = [512, ]
    print(f"\n- Bench upsample {mode} 1d ({min_run_time} min_run_time) - upsampling to 512")
    sub_bench_1d_single_op(mode, min_run_time, t_input, [512, ])


def sub_bench_3d_single_op(mode, min_run_time, t_input, output_size):
    sub_bench_Nd_single_op(mode, 3, min_run_time, t_input, output_size)


def sub_bench_3d(mode, min_run_time, t_input, dn_size, up_size):
    print(f"\nInput tensor: {list(t_input.shape)}")
    print(f"Input is_contiguous memory_format torch.channels_last: {t_input.is_contiguous(memory_format=torch.channels_last_3d)}")
    print(f"Input is_contiguous : {t_input.is_contiguous()}")

    print(f"\n- Bench upsample {mode} 3d ({min_run_time} min_run_time) - downsampling to {dn_size}")
    sub_bench_3d_single_op(mode, min_run_time, t_input, dn_size)

    print(f"\n- Bench upsample {mode} 3d ({min_run_time} min_run_time) - upsampling to {up_size}")
    sub_bench_3d_single_op(mode, min_run_time, t_input, up_size)


def bench_3d(mode, min_run_time, full_bench, test_cases):
    if not any(is_in(k, test_cases) for k in ["3dcf", "3dcl"]):
        return

    if is_in("3dcf", test_cases):
        t_input = torch.rand(1, 3, 16, 320, 320, dtype=torch.float, device="cpu")
        sub_bench_3d(mode, min_run_time, t_input, [8, 256, 256], [32, 512, 512])

    if not full_bench:
        return

    if is_in("3dcl", test_cases):
        t_input = torch.rand(1, 16, 320, 320, 3, dtype=torch.float, device="cpu")
        t_input = t_input.permute(0, 4, 1, 2, 3)
        sub_bench_3d(mode, min_run_time, t_input, [8, 256, 256], [32, 512, 512])


def main():
    torch.manual_seed(10)

    parser = argparse.ArgumentParser("Benchmark upsampling/downsampling ops")
    parser.add_argument(
        "output_filepath", type=str,
        help="Output file path", nargs='?'
    )
    parser.add_argument(
        "min_run_time", type=int, default=5, 
        help="Number of rounds", nargs='?'
    )
    parser.add_argument(
        "full_bench", type=int, default=False, choices=[0, 1],
        help="If run the full benchmark", nargs='?'
    )
    parser.add_argument(
        "test_all_dims", type=int, default=0, choices=[0, 1],
        help="If run tests for all dimensions", nargs='?'
    )
    parser.add_argument(
        "num_threads", type=int, default=6,
        help="Sets number of threads", nargs='?'
    )
    all_dims = ["2dcf", "2dcl", "3dcf", "3dcl", "1d"]
    all_modes = ["linear", "nearest", "cubic"]
    all_tests = [f"{mode}:{dim}" for mode, dim in product(all_modes, all_dims)]
    parser.add_argument(
        "--test_cases", type=str,
        choices=all_tests + ["all", ],
        nargs='+', default=None,
        help="Run custom benchmarks"
    )

    args = parser.parse_args()
    output_filepath = args.output_filepath
    min_run_time = args.min_run_time
    full_bench = bool(args.full_bench)
    test_all_dims = bool(args.test_all_dims)
    num_threads = args.num_threads

    if args.test_cases is None:
        args.test_cases = all_tests
    else:
        if args.test_cases == ["all", ]:
            args.test_cases = all_tests
        print("Uses tests cases: ", args.test_cases)
        full_bench = True
        test_all_dims = True

    print(f"Torch config: {torch.__config__.show()}")

    print(f"Num threads: {torch.get_num_threads()}")

    for mode in all_modes:

        print(f"\n\n---- Benchmark {mode} 2D ----")
        bench_2d(mode, min_run_time, full_bench, 320, 256, 512, args.test_cases)
        if full_bench:
            bench_2d(mode, min_run_time, False, 500, 256, 800, args.test_cases)
        print(f"\n---- END Benchmark {mode} 2D ----")

        if not test_all_dims:
            continue

        if mode == "cubic":
            continue

        print(f"\n\n---- Benchmark {mode} 1D ----")
        bench_1d(mode, min_run_time, full_bench, args.test_cases)
        print(f"\n---- END Benchmark {mode} 1D ----")

        print(f"\n\n---- Benchmark {mode} 3D ----")
        bench_3d(mode, min_run_time, full_bench, args.test_cases)
        print(f"\n---- END Benchmark {mode} 3D ----")

    with open(output_filepath, "wb") as handler:
        pickle.dump(test_results, handler)
    return 0


if __name__ == "__main__":

    print(f"__version__ = {torch.version.__version__}")
    print(f"debug = {torch.version.debug}")
    print(f"cuda = {torch.version.cuda}")
    print(f"git_version = {torch.version.git_version}")
    print(f"hip = {torch.version.hip}")
    print("")
    print("")

    env += f"PyTorch {torch.version.__version__}"

    exit(main())
