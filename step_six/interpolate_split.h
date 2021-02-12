#pragma once

#include <vector>
#include <ATen/ATen.h>

namespace at {
namespace native {
namespace prototype {


Tensor ti_upsample_bilinear2d_kernel_impl(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);

Tensor ti_upsample_linear1d_kernel_impl(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);

Tensor ti_upsample_trilinear3d_kernel_impl(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


} // anonymous namespace
} // namespace native
} // namespace at
