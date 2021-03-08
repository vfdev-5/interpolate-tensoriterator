#pragma once

#include <ATen/ATen.h>


namespace at {
namespace native {
namespace ti_upsample {


Tensor ti_upsample_bilinear2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor ti_upsample_linear1d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor ti_upsample_trilinear3d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


} // namespace ti_upsample
} // namespace native
} // namespace at
