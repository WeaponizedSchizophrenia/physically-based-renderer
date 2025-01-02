#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace pbr::core {
/**
 * Manages a vulkan swapchain.
 */
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

  /**
   * Recreates this swapchain with the provided arguments.
   */
  auto recreate(vk::SurfaceKHR surface, vk::Extent2D extent) -> void;

  /**
   * Acquires the next image index.
   * @param semaphore the semaphore that gets passed into vkAcquireNextImageKHR.
   * @param fence the fence that gets passed into vkAcquireNextImageKHR.
   * @param timeout optional timeout if no value is provided than the timeout will be
   * numeric_limits<uint64_t>::max() nanoseconds.
   *
   * @returns the return value of vkAcquireNextImageKHR.
   */
  [[nodiscard]]
  auto acquireImageIndex(vk::Semaphore semaphore, vk::Fence fence = nullptr,
                         std::optional<std::chrono::nanoseconds> timeout = std::nullopt)
      -> vk::ResultValue<std::uint32_t>;

  [[nodiscard]]
  constexpr auto getGpuHandle() const noexcept -> SharedGpuHandle const&;

  [[nodiscard]]
  constexpr auto getExtent() const noexcept -> vk::Extent2D;

  [[nodiscard]]
  constexpr auto getFormat() const noexcept -> vk::SurfaceFormatKHR;

  [[nodiscard]]
  constexpr auto getSwapchain() const noexcept -> vk::SwapchainKHR;

  [[nodiscard]]
  constexpr auto getImage(std::size_t index) const noexcept -> vk::Image;

  [[nodiscard]]
  constexpr auto getImageView(std::size_t index) const noexcept -> vk::ImageView;

private:
  /**
   * Initializes the _views member with data from _images.
   */
  auto initializeViews() -> void;
  /**
   * Creates a vk::SwapchainCreateInfoKHR from the _format, _presentMode, _extent members
   * and the arguments.
   */
  [[nodiscard]]
  auto getSwapchainCreateInfo(vk::SurfaceKHR surface,
                              vk::SurfaceCapabilitiesKHR capabilities) const noexcept
      -> vk::SwapchainCreateInfoKHR;
};
} // namespace pbr::core

/* IMPLEMENTATIONS */

constexpr auto
pbr::core::Swapchain::getGpuHandle() const noexcept -> SharedGpuHandle const& {
  return _gpu;
}

constexpr auto pbr::core::Swapchain::getExtent() const noexcept -> vk::Extent2D {
  return _extent;
}

constexpr auto pbr::core::Swapchain::getFormat() const noexcept -> vk::SurfaceFormatKHR {
  return _format;
}

constexpr auto pbr::core::Swapchain::getSwapchain() const noexcept -> vk::SwapchainKHR {
  return _swapchain.get();
}
constexpr auto
pbr::core::Swapchain::getImage(std::size_t index) const noexcept -> vk::Image {
  return _images.at(index);
}

constexpr auto
pbr::core::Swapchain::getImageView(std::size_t index) const noexcept -> vk::ImageView {
  return _views.at(index).get();
}
