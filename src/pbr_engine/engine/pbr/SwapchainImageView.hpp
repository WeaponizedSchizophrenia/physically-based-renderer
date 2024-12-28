#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/Swapchain.hpp"

#include <cstdint>

namespace pbr {
/**
 * Non owning view of a specific swapchain image.
 */
class SwapchainImageView {
  core::Swapchain const* _swapchain;
  std::uint32_t _imageIndex;

public:
  SwapchainImageView(core::Swapchain const& swapchain, std::uint32_t imageIndex);

  /** 
   * Presents this swapchain image.
   */
  auto present(vk::Semaphore waitSemaphore) -> void;

  [[nodiscard]]
  constexpr auto getImage() const -> vk::Image;
  [[nodiscard]]
  constexpr auto getImageView() const -> vk::ImageView;
  [[nodiscard]]
  constexpr auto getExtent() const noexcept -> vk::Extent2D;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto pbr::SwapchainImageView::getImage() const -> vk::Image {
  return _swapchain->getImage(_imageIndex);
}
constexpr auto pbr::SwapchainImageView::getImageView() const -> vk::ImageView {
  return _swapchain->getImageView(_imageIndex);
}

constexpr auto pbr::SwapchainImageView::getExtent() const noexcept -> vk::Extent2D {
  return _swapchain->getExtent();
}
