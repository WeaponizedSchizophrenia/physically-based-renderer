#pragma once

#include <cstdint>

namespace pbr::core {
/**
 * Contains physical device properties, important to the renderer/application.
 */
struct PhysicalDeviceProperties {
  /// The index of the physical device in the instance.
  std::uint32_t physicalDeviceIndex;
  /// The index of the queue that has graphics, transfer and present support.
  std::uint32_t graphicsTransferPresentQueue;
};
} // namespace pbr::core
