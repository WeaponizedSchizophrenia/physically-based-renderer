#include "pbr/core/GpuHandle.hpp"

#include "pbr/Vulkan.hpp"
#include "pbr/core/GpuHandleCreateInfo.hpp"
#include "pbr/core/PhysicalDeviceProperties.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <ranges>
#include <stdexcept>

#include <vulkan/vulkan_core.h>

namespace constants {
constexpr static std::array VALIDATION_LAYERS {"VK_LAYER_KHRONOS_validation"};
constexpr static std::array DEVICE_EXTENSIONS {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
} // namespace constants

namespace {
[[nodiscard]]
constexpr auto
createInstance(pbr::core::GpuHandleCreateInfo const& info) -> vk::UniqueInstance {
  vk::ApplicationInfo const appInfo {
      .pEngineName = "Physically based renderer",
      .apiVersion = vk::ApiVersion13,
  };
  auto instanceInfo =
      vk::InstanceCreateInfo {
          .pApplicationInfo = &appInfo,
      }
          .setPEnabledExtensionNames(info.extensions);

  if (info.enableValidation) {
    instanceInfo.setPEnabledLayerNames(constants::VALIDATION_LAYERS);
  }

  return vk::createInstanceUnique(instanceInfo);
}
[[nodiscard]]
constexpr auto findPhysicalDevice(vk::Instance const instance,
                                  pbr::core::GpuHandleCreateInfo const& info)
    -> pbr::core::PhysicalDeviceProperties {
  assert(info.presentPredicate != nullptr && "info.presentPredicate cannot be null");

  for (auto const [deviceIdx, physicalDevice] :
       instance.enumeratePhysicalDevices() | std::views::enumerate) {
    std::optional<std::uint32_t> graphicsTransferPresentQueueIndex = std::nullopt;

    for (auto const [idx, queueFamilyProp] :
         physicalDevice.getQueueFamilyProperties() | std::views::enumerate) {
      bool const isGraphics {queueFamilyProp.queueFlags & vk::QueueFlagBits::eGraphics};
      bool const isTransfer {queueFamilyProp.queueFlags & vk::QueueFlagBits::eTransfer};
      auto const isPresent = info.presentPredicate(instance, physicalDevice, idx);

      if (isGraphics && isTransfer && isPresent) {
        graphicsTransferPresentQueueIndex.emplace(idx);
      }
    }

    if (graphicsTransferPresentQueueIndex.has_value()) {
      return {
          .physicalDeviceIndex = static_cast<std::uint32_t>(deviceIdx),
          .graphicsTransferPresentQueue = *graphicsTransferPresentQueueIndex,
      };
    }
  }

  throw std::runtime_error("Could not find a suitable physical device");
}
[[nodiscard]]
constexpr auto
createDevice(vk::PhysicalDevice const physicalDevice,
             pbr::core::PhysicalDeviceProperties deviceProps) -> vk::UniqueDevice {
  constexpr auto QUEUE_PRIORITY = 1.0f;
  vk::DeviceQueueCreateInfo const queueInfo {
      .queueFamilyIndex = deviceProps.graphicsTransferPresentQueue,
      .queueCount = 1,
      .pQueuePriorities = &QUEUE_PRIORITY,
  };
  auto const deviceInfo =
      vk::DeviceCreateInfo {}.setQueueCreateInfos(queueInfo).setPEnabledExtensionNames(
          constants::DEVICE_EXTENSIONS);
  vk::PhysicalDeviceSynchronization2Features const sync2 {.synchronization2 = vk::True};
  vk::PhysicalDeviceDynamicRenderingFeatures const dynRendering {.dynamicRendering =
                                                                     vk::True};

  return physicalDevice.createDeviceUnique(
      vk::StructureChain {deviceInfo, sync2, dynRendering}.get());
}
} // namespace

pbr::core::GpuHandle::GpuHandle(GpuHandleCreateInfo const& info)
    : _instance(::createInstance(info))
    , _physicalDeviceProperties(::findPhysicalDevice(_instance, info))
    , _physicalDevice(_instance->enumeratePhysicalDevices().at(
          _physicalDeviceProperties.physicalDeviceIndex))
    , _device(::createDevice(_physicalDevice, _physicalDeviceProperties))
    , _queue(
          _device->getQueue(_physicalDeviceProperties.graphicsTransferPresentQueue, 0)) {}
