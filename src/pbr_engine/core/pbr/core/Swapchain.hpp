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
public:
  Swapchain(SharedGpuHandle gpu, vk::SurfaceKHR surface, vk::Extent2D extent);

  auto resize(vk::SurfaceKHR surface, vk::Extent2D extent) -> void;

  [[nodiscard]]
  auto acquireImageIndex(vk::Semaphore semaphore, vk::Fence fence = nullptr,
                         std::optional<std::chrono::nanoseconds> timeout = std::nullopt)
      -> vk::ResultValue<std::uint32_t>;
  [[nodiscard]]
  auto operator[](std::size_t index) const -> std::pair<vk::Image, vk::ImageView>;

private:
  auto initializeViews() -> void;
  [[nodiscard]]
  auto getSwapchainCreateInfo(vk::SurfaceKHR surface,
                              vk::SurfaceCapabilitiesKHR capabilities) const noexcept
      -> vk::SwapchainCreateInfoKHR;

  SharedGpuHandle _gpu;

  vk::SurfaceFormatKHR _format;
  vk::PresentModeKHR _presentMode;
  vk::Extent2D _extent;

  vk::UniqueSwapchainKHR _swapchain;

  std::vector<vk::Image> _images;
  std::vector<vk::UniqueImageView> _views;
};
} // namespace pbr::core
