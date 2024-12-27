#pragma once

#include "pbr/Vulkan.hpp"
#include "pbr/core/GpuHandle.hpp"
#include "pbr/core/Swapchain.hpp"

namespace pbr {
class Surface {
  vk::UniqueSurfaceKHR _surface;
  core::Swapchain _swapchain;

public:
  Surface(core::SharedGpuHandle gpu, vk::UniqueSurfaceKHR surface, vk::Extent2D extent);

  auto recreateSwapchain(vk::Extent2D extent) -> void;
};
}
