#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image.hpp"

#include <concepts>
#include <utility>

namespace pbr {
class Image2D : public Image {
  vk::UniqueImageView _view;

public:
  template <typename... Ts>
    requires std::constructible_from<Image, Ts...>
  constexpr Image2D(core::GpuHandle const& gpu, vk::Format format,
                             vk::ImageAspectFlags aspect, Ts&&... imageArgs);

  [[nodiscard]]
  constexpr auto getImageView() const noexcept -> vk::ImageView;
};
} // namespace pbr

/* IMPLEMENTATIONS */

template <typename... Ts>
  requires std::constructible_from<pbr::Image, Ts...>
constexpr pbr::Image2D::Image2D(core::GpuHandle const& gpu, vk::Format format,
                                vk::ImageAspectFlags aspect, Ts&&... imageArgs)
    : Image(std::forward<Ts>(imageArgs)...)
    , _view(gpu.getDevice().createImageViewUnique({
          .image = getImage(),
          .viewType = vk::ImageViewType::e2D,
          .format = format,
          .subresourceRange {
              .aspectMask = aspect,
              .levelCount = 1,
              .layerCount = 1,
          },
      })) {}

constexpr auto pbr::Image2D::getImageView() const noexcept -> vk::ImageView {
  return _view.get();
}
