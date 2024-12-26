#pragma once

#include <cstdint>

namespace pbr::core {
struct PhysicalDeviceProperties {
  std::uint32_t physicalDeviceIndex;
  std::uint32_t graphicsTransferPresentQueue;
};
} // namespace pbr::core
