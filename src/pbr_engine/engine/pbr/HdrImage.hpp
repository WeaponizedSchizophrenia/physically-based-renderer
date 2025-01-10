#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/memory/IAllocator.hpp"

namespace pbr {
class HdrImage {
  core::SharedGpuHandle _gpu;
  Image2D _image;
  vk::Extent2D _extent;
  vk::UniqueDescriptorSet _descSet;
  vk::Image _outputImage;

public:
  HdrImage(core::SharedGpuHandle gpu, IAllocator& allocator,
           vk::UniqueDescriptorSet descSet, vk::Extent2D extent);

  auto updateOutputTexture(vk::Image image, vk::ImageView imageView) -> void;

  [[nodiscard]]
  constexpr auto getImage() const noexcept -> Image2D const&;
  [[nodiscard]]
  constexpr auto getExtent() const noexcept -> vk::Extent2D;
  [[nodiscard]]
  constexpr auto getDescriptorSet() const noexcept -> vk::DescriptorSet;
  [[nodiscard]]
  constexpr auto getOutputImage() const noexcept -> vk::Image;
};
} // namespace pbr

/* IMPLMENTATIONS */

constexpr auto pbr::HdrImage::getImage() const noexcept -> Image2D const& {
  return _image;
}
constexpr auto pbr::HdrImage::getExtent() const noexcept -> vk::Extent2D {
  return _extent;
}
constexpr auto pbr::HdrImage::getDescriptorSet() const noexcept -> vk::DescriptorSet {
  return _descSet.get();
}
constexpr auto pbr::HdrImage::getOutputImage() const noexcept -> vk::Image {
  return _outputImage;
}
