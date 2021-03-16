import torch
import torch._C as C
import torch.autograd.profiler as profiler

from run_pytorch_native_bench import *


def main():
    torch.manual_seed(10)

    args = get_args(all_tests)
    output_filepath = args.output_filepath
    min_run_time = args.min_run_time
    num_threads = args.num_threads
    test_cases, full_bench, test_all_dims = get_test_cases(args)

    print(f"Torch config: {torch.__config__.show()}")

    print(f"Num threads: {torch.get_num_threads()}")

    with profiler.profile(with_stack=True, profile_memory=False) as prof:

        for mode in all_modes:

            print(f"\n\n---- Benchmark {mode} 2D ----")
            with profiler.record_function("{mode} 2D"):
                bench_2d(mode, min_run_time, full_bench, 320, 256, 512, test_cases)
                if full_bench:
                    bench_2d(mode, min_run_time, False, 500, 256, 800, test_cases)
            print(f"\n---- END Benchmark {mode} 2D ----")

            if not test_all_dims:
                continue

            if mode == "cubic":
                continue

            print(f"\n\n---- Benchmark {mode} 1D ----")
            with profiler.record_function("{mode} 1D"):
                bench_1d(mode, min_run_time, full_bench, test_cases)
            print(f"\n---- END Benchmark {mode} 1D ----")

            print(f"\n\n---- Benchmark {mode} 3D ----")
            with profiler.record_function("{mode} 3D"):
                bench_3d(mode, min_run_time, full_bench, test_cases)
            print(f"\n---- END Benchmark {mode} 3D ----")

    print(prof.key_averages(group_by_stack_n=10).table(sort_by='self_cpu_time_total', row_limit=5))


if __name__ == "__main__":

    print(f"__version__ = {torch.version.__version__}")
    print(f"debug = {torch.version.debug}")
    print(f"cuda = {torch.version.cuda}")
    print(f"git_version = {torch.version.git_version}")
    print(f"hip = {torch.version.hip}")
    print("")
    print("")

    env += f"PyTorch {torch.version.__version__}"

    main()
