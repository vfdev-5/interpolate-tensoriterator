


import torch
import torch._C as C

x = torch.rand(1, 32, 32, 3).permute(0, 3, 1, 2)
print(x.is_contiguous(memory_format=torch.channels_last))

_ = C._nn.upsample_bilinear2d(x, (64, 64), False)