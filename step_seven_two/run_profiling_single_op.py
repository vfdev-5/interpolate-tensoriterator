import torch
import torch._C as C

from run_pytorch_native_bench import *


def main():
    torch.manual_seed(10)
    print(f"Torch config: {torch.__config__.show()}")
    print(f"Num threads: {torch.get_num_threads()}")

    t_input = torch.rand(1, 3, 320, 320, dtype=torch.float, device="cpu")

    print(f"\nInput tensor: {list(t_input.shape)}")
    print(f"Input is_contiguous memory_format torch.channels_last: {t_input.is_contiguous(memory_format=torch.channels_last)}")
    print(f"Input is_contiguous : {t_input.is_contiguous()}")

    output_size = (512, 512)
    _ = C._nn.upsample_bilinear2d(t_input, output_size, False)
    
    
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
