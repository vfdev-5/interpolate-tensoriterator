# OMP_NUM_THREADS=1 PYTHONPATH=/pytorch/ python -u bench.py

import argparse
import PIL
from PIL import Image

import torch
import torch.utils.benchmark as benchmark

# Original image size: 906, 438
sizes = [
    (320, 196),
    (460, 220),
    (120, 96),
    (1200, 196),
    (120, 1200),
]

resampling_map = {
    "bilinear": PIL.Image.Resampling.BILINEAR,
    "nearest": PIL.Image.Resampling.NEAREST,
    "bicubic": PIL.Image.Resampling.BICUBIC
}


def run_bench(t_img, pil_img, size, mode, device, do_backward=False, antialias=True):
    # All variables are taken from __main__ scope

    inv_size = size[::-1]
    resample = resampling_map[mode]

    align_corners = False
    if mode == "nearest":
        antialias = False
        align_corners = None

    if t_img.is_contiguous(memory_format=torch.channels_last):
        if t_img.is_contiguous():
            mem_format = ""
        else:
            mem_format = "channels_last"
    else:
        mem_format = "channels_first"
    is_contiguous = "contiguous" if t_img.is_contiguous() else "non-contiguous"

    op_direction = "backward " if do_backward else ""
    label = f"Downsampling {op_direction}({mode}): {t_img.shape} -> {size}"
    sub_label = f"{mem_format} {is_contiguous} {t_img.dtype}"
    min_run_time = 3

    results = []

    if pil_img is not None:
        results.append(
            benchmark.Timer(
                # pil_img.resize(size, resample=resample_val)
                stmt=f"img.resize(size, resample=resample_val)",
                globals={
                    "img": pil_img,
                    "size": size,
                    "resample_val": resample,
                },
                num_threads=torch.get_num_threads(),
                label=label,
                sub_label=sub_label,
                description=f"Reference, PIL {PIL.__version__}, mode: {pil_img.mode}",
            ).blocked_autorange(min_run_time=min_run_time)
        )

    if not do_backward:
        results.append(
            benchmark.Timer(
                # pth_downsample(t_img, mode, size)
                stmt=f"f(x, mode='{mode}', size=size, align_corners={align_corners}, antialias={antialias})",
                globals={
                    "x": t_img,
                    "size": inv_size,
                    "f": torch.nn.functional.interpolate
                },
                num_threads=torch.get_num_threads(),
                label=label,
                sub_label=sub_label,
                description=f"{torch.version.__version__} {device}",
            ).blocked_autorange(min_run_time=min_run_time),
        )
    else:
        t_img_w_grad = t_img.clone()
        t_img_w_grad.requires_grad_()
        out = torch.nn.functional.interpolate(
            t_img_w_grad, mode=mode, size=size, align_corners=False, antialias=True
        )
        loss = out.mean()
        results.append(
            benchmark.Timer(
                stmt=f"loss.backward(); assert t_img_w_grad.grad is not None",
                globals={
                    "loss": loss,
                    "t_img_w_grad": t_img_w_grad
                },
                num_threads=torch.get_num_threads(),
                label=label,
                sub_label=sub_label,
                description=f"{torch.version.__version__} {device}",
            ).blocked_autorange(min_run_time=min_run_time),
        )

    return results


if __name__ == "__main__":

    parser = argparse.ArgumentParser("Benchmark interpolation with anti-alias option")
    parser.add_argument(
        "--mode", default="bilinear", type=str,
        choices=["bilinear", "bicubic", "nearest"],
        help="Interpolation mode"
    )
    parser.add_argument(
        "--cuda", action="store_true",
        help="Run on GPU only"
    )
    parser.add_argument(
        "--cpu", action="store_true",
        help="Run on CPU only"
    )
    parser.add_argument(
        "--backward", action="store_true",
        help="Measure only backward op"
    )
    parser.add_argument(
        "--size", type=int, nargs=2,
        help="Use the specified size for the tests"
    )

    args = parser.parse_args()

    mode = args.mode
    if args.cpu:
        devices = ["cpu"]
    elif args.cuda:
        devices = ["cuda"]
    else:
        devices = ["cpu", "cuda"]

    resample = resampling_map[mode]

    if args.size is not None:
        print(f"- Use specified size: {args.size}")
        sizes = [args.size, ]

    if args.backward:
        print("- Measure only backward op")

    print("")
    print(f"Torch version: {torch.__version__}")
    print(f"Torch config: {torch.__config__.show()}")
    print(f"Num threads: {torch.get_num_threads()}")

    # # Benchmark resize PTH (1, 1, H, W) vs PIL uint8
    # t_img_cf = torch.randint(0, 256, size=(1, 1, 906, 438), dtype=torch.float, device="cpu")
    # print(t_img_cf.is_contiguous(memory_format=torch.channels_last), t_img_cf.is_contiguous())
    # t_img_cf = t_img_cf.contiguous()
    # print(t_img_cf.is_contiguous(memory_format=torch.channels_last), t_img_cf.is_contiguous())
    # torch.nn.functional.interpolate(t_img_cf, sizes[0], mode="nearest")

    # exit(0)

    all_results = []

    for i, device in enumerate(devices):

        if device == "cpu":
            # Benchmark resize PTH (1, 1, H, W) vs PIL uint8
            t_img_cf = torch.randint(0, 256, size=(1, 1, 906, 438), dtype=torch.uint8, device=device)
            a_img = t_img_cf.cpu().numpy().transpose((0, 2, 3, 1)).astype("uint8")
            pil_img = Image.fromarray(a_img[0, 0, ...])

            # Channel-first
            for s in sizes:
                all_results += run_bench(t_img_cf, pil_img, s, mode, device, do_backward=args.backward)

        break

        # Benchmark resize PTH RGB,F32 vs PIL uint8
        t_img_cf = torch.randint(0, 256, size=(1, 3, 906, 438), dtype=torch.float, device=device)
        t_img_cl = t_img_cf.contiguous(memory_format=torch.channels_last)
        a_img = t_img_cf.cpu().numpy().transpose((0, 2, 3, 1)).astype("uint8")
        if i == 0 and not args.backward:
            pil_img = Image.fromarray(a_img[0, ...])
        else:
            # skip bench on PIL resize for different tensor devices
            pil_img = None

        # Channel-first
        for s in sizes:
            all_results += run_bench(t_img_cf, pil_img, s, mode, device, do_backward=args.backward)

        # Channel-last
        for s in sizes:
            all_results += run_bench(t_img_cl, pil_img, s, mode, device, do_backward=args.backward)

        # Benchmark resize PTH 1-Channel,F32 vs PIL F32
        t_img_cf = torch.randint(0, 256, size=(1, 1, 906, 438), dtype=torch.float, device=device)
        a_img = t_img_cf.cpu().numpy().transpose((0, 2, 3, 1))
        if i == 0 and not args.backward:
            pil_img = Image.fromarray(a_img[0, ..., 0])
        else:
            # skip bench on PIL resize for different tensor devices
            pil_img = None

        # Channel-first
        for s in sizes:
            all_results += run_bench(t_img_cf, pil_img, s, mode, device, do_backward=args.backward)

    compare = benchmark.Compare(all_results)
    compare.print()
