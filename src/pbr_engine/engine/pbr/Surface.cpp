#include "pbr/Surface.hpp"

#include "pbr/Vulkan.hpp"
#include <utility>

pbr::Surface::Surface(core::SharedGpuHandle gpu, vk::UniqueSurfaceKHR surface, vk::Extent2D extent)
  : _surface(std::move(surface))
  , _swapchain(std::move(gpu), _surface, extent)
{}

auto pbr::Surface::recreateSwapchain(vk::Extent2D extent) -> void {
  _swapchain.resize(_surface, extent);
}
