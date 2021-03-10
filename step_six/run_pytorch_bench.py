#
# export PR_TORCH_PATH=/workspace/pth-linear-interp/
# sh run_python_pr_bench.sh
# > pr_vs_pth_results.md
#
import argparse
import time
import torch
import torch._C as C


def sub_bench_2d(n, t_input, dn_osize, up_osize, m=None):
    print(f"\nInput tensor: {list(t_input.shape)}")
    print(f"Input is_contiguous memory_format torch.channels_last: {t_input.is_contiguous(memory_format=torch.channels_last)}")
    print(f"Input is_contiguous : {t_input.is_contiguous()}")

    output_size = (dn_osize, dn_osize)
    print(f"\n- Bench upsample_bilinear2d ({n} rounds) - downsampling to {dn_osize}x{dn_osize}")

    _ = C._nn.upsample_bilinear2d(t_input, output_size, False)

    start = time.time()
    for _ in range(n):
        ref_out = C._nn.upsample_bilinear2d(t_input, output_size, False)
        result = ref_out.size(0)
    
    end = time.time()
    elapsed_seconds = end - start
    print(f"Elapsed time (ms): {elapsed_seconds / n * 1000}")

    if m is not None:
        n = m

    output_size = (up_osize, up_osize)
    print(f"\n- Bench upsample_bilinear2d ({n} rounds) - upsampling to {up_osize}x{up_osize}")

    _ = C._nn.upsample_bilinear2d(t_input, output_size, False)

    start = time.time()
    for _ in range(n):
        ref_out = C._nn.upsample_bilinear2d(t_input, output_size, False)
        result = ref_out.size(0)
    
    end = time.time()
    elapsed_seconds = end - start
    print(f"Elapsed time (ms): {elapsed_seconds / n * 1000}")


def sub_bench_2d_non_contiguous_channel_first(n, isize, dn_osize, up_osize):
    t_input = torch.rand(1, 3, isize + 100, isize + 100, dtype=torch.float, device="cpu")
    t_input = t_input[..., :isize, :isize]
    sub_bench_2d(n, t_input, dn_osize, up_osize)


def sub_bench_2d_contiguous_channel_first(n, isize, dn_osize, up_osize):
    t_input = torch.rand(1, 3, isize, isize, dtype=torch.float, device="cpu")
    sub_bench_2d(n, t_input, dn_osize, up_osize)


def sub_bench_2d_mingfeima_channel_last(n):
    print(f"\n1 - Test size as in https://github.com/mingfeima/op_bench-py")
    t_input = torch.rand(32, 64, 64, 128, dtype=torch.float, device="cpu")
    t_input = t_input.permute(0, 3, 1, 2)
    sub_bench_2d(n, t_input, 32, 128, n // 5)

    print(f"\n1.2 - Test sizes similar to https://github.com/pytorch/pytorch/blob/master/benchmarks/operator_benchmark/pt/interpolate_test.py")
    t_input = torch.rand(2, 64, 46, 128, dtype=torch.float, device="cpu")
    t_input = t_input.permute(0, 3, 1, 2)
    sub_bench_2d(n, t_input, 32, 128, n // 2)


def sub_bench_2d_mingfeima_channel_first(n):
    print(f"\n2 - Test size as in https://github.com/mingfeima/op_bench-py")
    t_input = torch.rand(32, 128, 64, 64, dtype=torch.float, device="cpu")
    sub_bench_2d(n, t_input, 32, 128, n // 5)


def sub_bench_2d_non_contiguous_channel_last(n, isize, dn_osize, up_osize, n_channels=3):
    t_input = torch.rand(1, isize, isize, n_channels, dtype=torch.float, device="cpu")
    t_input = t_input.permute(0, 3, 1, 2)
    sub_bench_2d(n, t_input, dn_osize, up_osize)


def bench_2d(n, full_bench, isize, dn_osize, up_osize, args):
    if not any(k in args.custom for k in ["2dcf", "2dcl"]):
        return

    if "2dcf" in args.custom:
        sub_bench_2d_contiguous_channel_first(n, isize, dn_osize, up_osize)
        sub_bench_2d_non_contiguous_channel_first(n, isize, dn_osize, up_osize)

    if "2dcl" in args.custom:
        sub_bench_2d_non_contiguous_channel_last(n, isize, dn_osize, up_osize)

    if not full_bench:
        return

    if "2dcl" in args.custom:
        sub_bench_2d_mingfeima_channel_last(n)

    if "2dcf" in args.custom:
        sub_bench_2d_mingfeima_channel_first(n)


def bench_1d(n, full_bench, args):
    if not any(k in args.custom for k in ["1d", ]):
        return

    t_input = torch.rand(4, 512, 320, dtype=torch.float, device="cpu")
    print(f"\nInput tensor: {list(t_input.shape)}")
    print(f"Input is_contiguous memory_format torch.channels_last: {t_input.is_contiguous(memory_format=torch.channels_last)}")
    print(f"Input is_contiguous : {t_input.is_contiguous()}")

    output_size = [256, ]
    print(f"\n- Bench upsample_linear1d ({n} rounds) - downsampling to 256")
    start = time.time()
    for _ in range(n):
        ref_out = C._nn.upsample_linear1d(t_input, output_size, False)
        result = ref_out.size(0)
    
    end = time.time()
    elapsed_seconds = end - start
    print(f"Elapsed time (ms): {elapsed_seconds / n * 1000}")

    if not full_bench:
        return 1

    output_size = [512, ]
    print(f"\n- Bench upsample_linear1d ({n} rounds) - upsampling to 512")
    start = time.time()
    for _ in range(n):
        ref_out = C._nn.upsample_linear1d(t_input, output_size, False)
        result = ref_out.size(0)
    
    end = time.time()
    elapsed_seconds = end - start
    print(f"Elapsed time (ms): {elapsed_seconds / n * 1000}")


def sub_bench_3d(n, t_input, dn_size, up_size):
    print(f"\nInput tensor: {list(t_input.shape)}")
    print(f"Input is_contiguous memory_format torch.channels_last: {t_input.is_contiguous(memory_format=torch.channels_last_3d)}")
    print(f"Input is_contiguous : {t_input.is_contiguous()}")

    print(f"\n- Bench upsample_trilinear3d ({n} rounds) - downsampling to {dn_size}")
    start = time.time()
    for _ in range(n):
        ref_out = C._nn.upsample_trilinear3d(t_input, dn_size, False)
        result = ref_out.size(0)
    
    end = time.time()
    elapsed_seconds = end - start
    print(f"Elapsed time (ms): {elapsed_seconds / n * 1000}")

    print(f"\n- Bench upsample_trilinear3d ({n} rounds) - upsampling to {up_size}")
    start = time.time()
    for _ in range(n):
        ref_out = C._nn.upsample_trilinear3d(t_input, up_size, False)
        result = ref_out.size(0)
    
    end = time.time()
    elapsed_seconds = end - start
    print(f"Elapsed time (ms): {elapsed_seconds / n * 1000}")


def bench_3d(n, full_bench, args):
    if not any(k in args.custom for k in ["3dcf", "3dcl"]):
        return

    if "3dcf" in args.custom:
        t_input = torch.rand(1, 3, 16, 320, 320, dtype=torch.float, device="cpu")
        sub_bench_3d(n, t_input, [8, 256, 256], [32, 512, 512])

    if not full_bench:
        return

    if "3dcf" in args.custom:
        t_input = torch.rand(1, 16, 320, 320, 3, dtype=torch.float, device="cpu")
        t_input = t_input.permute(0, 4, 1, 2, 3)
        sub_bench_3d(n, t_input, [8, 256, 256], [32, 512, 512])


def warmup():
    for _ in range(2000):
        t_input = torch.rand(1, 5, 360, 360, dtype=torch.float, device="cpu")
        ref_out = C._nn.upsample_bilinear2d(t_input, [224, 224], False)
        result = ref_out.size(0)


def main():
    torch.manual_seed(10)

    parser = argparse.ArgumentParser("Benchmark upsampling/downsampling ops")
    parser.add_argument(
        "n", type=int, default=7500, 
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
    all_tests = ["2dcf", "2dcl", "3dcf", "3dcl", "1d"]
    parser.add_argument(
        "--custom", type=str, 
        choices=all_tests,
        nargs='+', default=None,
        help="Run custom benchmarks"
    )
    parser.add_argument(
        "--no_warmup", action="store_true",
        help="Skip warmup"
    )

    args = parser.parse_args()
    n = args.n
    full_bench = bool(args.full_bench)
    test_all_dims = bool(args.test_all_dims)
    num_threads = args.num_threads

    if args.custom is None:
        args.custom = all_tests
    else:
        print("Uses custom tests config: ", args.custom)
        full_bench = True
        test_all_dims = True

    print(f"Torch config: {torch.__config__.show()}")

    torch.set_num_threads(num_threads)
    print(f"Num threads: {num_threads}")

    if not args.no_warmup:
        print("\n- Perform a warmup ...", end=" ")
        warmup()
        print("Done")

    print("\n\n---- Benchmark 2D ----")
    bench_2d(n, full_bench, 320, 256, 512, args)
    if full_bench:
        bench_2d(n, False, 500, 256, 800, args)
    print("\n---- END Benchmark 2D ----")

    if not test_all_dims:
        return 0

    print("\n\n---- Benchmark 1D ----")
    bench_1d(n, full_bench, args)
    print("\n---- END Benchmark 1D ----")

    print("\n\n---- Benchmark 3D ----")
    bench_3d(n // 5, full_bench, args)
    print("\n---- END Benchmark 3D ----")

    return 0


if __name__ == "__main__":

    print(f"__version__ = {torch.version.__version__}")
    print(f"debug = {torch.version.debug}")
    print(f"cuda = {torch.version.cuda}")
    print(f"git_version = {torch.version.git_version}")
    print(f"hip = {torch.version.hip}")
    print("")
    print("")

    exit(main())
