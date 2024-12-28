#include "pbr/Surface.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/SwapchainImageView.hpp"

#include <optional>
#include <utility>

pbr::Surface::Surface(core::SharedGpuHandle gpu, vk::UniqueSurfaceKHR surface,
                      vk::Extent2D const extent)
    : _surface(std::move(surface)), _swapchain(std::move(gpu), _surface, extent) {}

auto pbr::Surface::recreateSwapchain(vk::Extent2D const extent) -> void {
  _swapchain.recreate(_surface, extent);
}

auto pbr::Surface::acquireSwapchainImageView(vk::Semaphore const signalSemaphore, vk::Fence const fence,
                            std::optional<std::chrono::nanoseconds> const timeout)
    -> std::optional<SwapchainImageView> {
  auto const [result, index] =
      _swapchain.acquireImageIndex(signalSemaphore, fence, timeout);

  if (result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR) {
    return std::make_optional<SwapchainImageView>(_swapchain, index);
  } else {
    return std::nullopt;
  }
}
