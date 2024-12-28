#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace pbr::core {
class Swapchain {
  SharedGpuHandle _gpu;

  vk::SurfaceFormatKHR _format;
  vk::PresentModeKHR _presentMode;
  vk::Extent2D _extent;

  vk::UniqueSwapchainKHR _swapchain;

  std::vector<vk::Image> _images;
  std::vector<vk::UniqueImageView> _views;

public:
  Swapchain(SharedGpuHandle gpu, vk::SurfaceKHR surface, vk::Extent2D extent);

  auto recreate(vk::SurfaceKHR surface, vk::Extent2D extent) -> void;

  [[nodiscard]]
  auto acquireImageIndex(vk::Semaphore semaphore, vk::Fence fence = nullptr,
                         std::optional<std::chrono::nanoseconds> timeout = std::nullopt)
      -> vk::ResultValue<std::uint32_t>;
  [[nodiscard]]
  auto operator[](std::size_t index) const -> std::pair<vk::Image, vk::ImageView>;

  [[nodiscard]]
  constexpr auto getGpuHandle() const noexcept -> SharedGpuHandle const&;

  [[nodiscard]]
  constexpr auto getExtent() const noexcept -> vk::Extent2D;

  [[nodiscard]]
  constexpr auto getSwapchain() const noexcept -> vk::SwapchainKHR;

  [[nodiscard]]
  constexpr auto getImage(std::size_t index) const noexcept -> vk::Image;

  [[nodiscard]]
  constexpr auto getImageView(std::size_t index) const noexcept -> vk::ImageView;

private:
  auto initializeViews() -> void;
  [[nodiscard]]
  auto getSwapchainCreateInfo(vk::SurfaceKHR surface,
                              vk::SurfaceCapabilitiesKHR capabilities) const noexcept
      -> vk::SwapchainCreateInfoKHR;
};
} // namespace pbr::core

/* IMPLEMENTATIONS */

constexpr auto pbr::core::Swapchain::getGpuHandle() const noexcept -> SharedGpuHandle const& {
  return _gpu;
}

constexpr auto pbr::core::Swapchain::getExtent() const noexcept -> vk::Extent2D {
  return _extent;
}

constexpr auto pbr::core::Swapchain::getSwapchain() const noexcept -> vk::SwapchainKHR {
  return _swapchain.get();
}
constexpr auto pbr::core::Swapchain::getImage(std::size_t index) const noexcept -> vk::Image {
  return _images.at(index);
}

constexpr auto pbr::core::Swapchain::getImageView(std::size_t index) const noexcept -> vk::ImageView {
  return _views.at(index).get();
}
