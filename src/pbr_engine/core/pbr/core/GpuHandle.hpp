#pragma once

#include "pbr/Vulkan.hpp"
#include "pbr/core/GpuHandleCreateInfo.hpp"
#include "pbr/core/PhysicalDeviceProperties.hpp"

#include <memory>

namespace pbr::core {
/**
 * Contains all the necessary objects to communicate with the gpu.
 */
class GpuHandle {
  vk::UniqueInstance _instance;
  PhysicalDeviceProperties _physicalDeviceProperties;
  vk::PhysicalDevice _physicalDevice;
  vk::UniqueDevice _device;
  vk::Queue _queue;

public:
  explicit GpuHandle(GpuHandleCreateInfo const& info);

  /* GETTERS */

  [[nodiscard]]
  constexpr auto getInstance() const noexcept -> vk::Instance;

  [[nodiscard]]
  constexpr auto getPhysicalDeviceProperties() const noexcept -> PhysicalDeviceProperties;

  [[nodiscard]]
  constexpr auto getPhysicalDevice() const noexcept -> vk::PhysicalDevice;

  [[nodiscard]]
  constexpr auto getDevice() const noexcept -> vk::Device;

  [[nodiscard]]
  constexpr auto getQueue() const noexcept -> vk::Queue;
};

using SharedGpuHandle = std::shared_ptr<GpuHandle const>;
/**
 * Creates a SharedGpuHandle.
 */
[[nodiscard]]
constexpr auto makeGpuHandle(GpuHandleCreateInfo const& info) -> SharedGpuHandle;
} // namespace pbr::core

/* IMPLEMENTATIONS */

constexpr auto pbr::core::GpuHandle::getInstance() const noexcept -> vk::Instance {
  return _instance.get();
}
constexpr auto pbr::core::GpuHandle::getPhysicalDeviceProperties() const noexcept
    -> PhysicalDeviceProperties {
  return _physicalDeviceProperties;
}
constexpr auto
pbr::core::GpuHandle::getPhysicalDevice() const noexcept -> vk::PhysicalDevice {
  return _physicalDevice;
}
constexpr auto pbr::core::GpuHandle::getDevice() const noexcept -> vk::Device {
  return _device.get();
}
constexpr auto pbr::core::GpuHandle::getQueue() const noexcept -> vk::Queue {
  return _queue;
}

constexpr auto
pbr::core::makeGpuHandle(GpuHandleCreateInfo const& info) -> SharedGpuHandle {
  return std::make_shared<GpuHandle>(info);
}
