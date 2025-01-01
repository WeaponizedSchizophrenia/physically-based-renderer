#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/core/Swapchain.hpp"

#include "pbr/SwapchainImageView.hpp"
#include <chrono>
#include <optional>

namespace pbr {
class Surface {
  vk::UniqueSurfaceKHR _surface;
  core::Swapchain _swapchain;

public:
  Surface(core::SharedGpuHandle gpu, vk::UniqueSurfaceKHR surface, vk::Extent2D extent);

  auto recreateSwapchain(vk::Extent2D extent) -> void;

  [[nodiscard]]
  auto acquireSwapchainImageView(vk::Semaphore signalSemaphore, vk::Fence fence = nullptr,
                                 std::optional<std::chrono::nanoseconds> timeout =
                                     std::nullopt) -> std::optional<SwapchainImageView>;

  [[nodiscard]]
  constexpr auto getFormat() const noexcept -> vk::SurfaceFormatKHR;
};
} // namespace pbr

/* IMPLEMENTATION */

constexpr auto pbr::Surface::getFormat() const noexcept -> vk::SurfaceFormatKHR {
  return _swapchain.getFormat();
}
