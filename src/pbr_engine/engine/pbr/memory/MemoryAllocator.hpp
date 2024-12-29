#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/memory/Allocation.hpp"
#include "pbr/memory/AllocationInfo.hpp"
#include "pbr/memory/IAllocator.hpp"

#include "vk_mem_alloc.h"

#include <utility>

namespace pbr {
class MemoryAllocator : public IAllocator {
  core::SharedGpuHandle _gpu;
  VmaAllocator _allocator;

public:
  explicit MemoryAllocator(core::SharedGpuHandle gpu);

  MemoryAllocator(const MemoryAllocator&) = delete;
  auto operator=(const MemoryAllocator&) -> MemoryAllocator& = delete;
  MemoryAllocator(MemoryAllocator&&) noexcept;
  auto operator=(MemoryAllocator&&) noexcept -> MemoryAllocator&;

  ~MemoryAllocator() noexcept override;

  [[nodiscard]]
  auto allocateBuffer(vk::BufferCreateInfo bufferInfo, AllocationInfo allocInfo)
      -> std::pair<Allocation, vk::UniqueBuffer> override;

  [[nodiscard]]
  auto allocateImage(vk::ImageCreateInfo imageInfo,
                     AllocationInfo allocInfo) -> std::pair<Allocation, vk::UniqueImage> override;
};
} // namespace pbr
