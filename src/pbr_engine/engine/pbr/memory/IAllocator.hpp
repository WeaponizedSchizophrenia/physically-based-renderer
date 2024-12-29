#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/memory/Allocation.hpp"
#include "pbr/memory/AllocationInfo.hpp"
#include <utility>

namespace pbr {
class IAllocator {
public:
  virtual ~IAllocator() = default;

  [[nodiscard]]
  virtual auto allocateBuffer(vk::BufferCreateInfo, AllocationInfo)
      -> std::pair<Allocation, vk::UniqueBuffer> = 0;
  [[nodiscard]]
  virtual auto allocateImage(vk::ImageCreateInfo, AllocationInfo)
      -> std::pair<Allocation, vk::UniqueImage> = 0;
};
} // namespace pbr
