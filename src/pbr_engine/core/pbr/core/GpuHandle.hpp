#pragma once

#include "pbr/Vulkan.hpp"
#include "pbr/core/GpuHandleCreateInfo.hpp"
#include "pbr/core/PhysicalDeviceProperties.hpp"

#include <memory>
#include <vulkan/vulkan_handles.hpp>

namespace pbr::core {
class GpuHandle {
  vk::UniqueInstance _instance;
  PhysicalDeviceProperties _physicalDeviceProperties;
  vk::PhysicalDevice _physicalDevice;
  vk::UniqueDevice _device;
  vk::Queue _queue;

public:
  explicit GpuHandle(GpuHandleCreateInfo const& info);
};

using SharedGpuHandle = std::shared_ptr<GpuHandle>;
[[nodiscard]]
constexpr auto makeGpuHandle(GpuHandleCreateInfo const& info) -> SharedGpuHandle;
} // namespace pbr::core

/* IMPLEMENTATIONS */

constexpr auto
pbr::core::makeGpuHandle(GpuHandleCreateInfo const& info) -> SharedGpuHandle {
  return std::make_shared<GpuHandle>(info);
}
