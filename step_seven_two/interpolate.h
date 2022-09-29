#pragma once

#include <ATen/ATen.h>


namespace at {
namespace native {
namespace ti_upsample {


Tensor upsample_nearest2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor upsample_bilinear2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor upsample_bicubic2d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor upsample_nearest1d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor upsample_linear1d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor upsample_nearest3d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


Tensor upsample_trilinear3d_cpu(
    const Tensor& input,
    c10::optional<IntArrayRef> output_size,
    bool align_corners,
    c10::optional<c10::ArrayRef<double>> scale_factors = c10::nullopt);


} // namespace ti_upsample
} // namespace native
} // namespace at
