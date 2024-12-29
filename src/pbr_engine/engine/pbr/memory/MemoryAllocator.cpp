#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "pbr/memory/MemoryAllocator.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/memory/Allocation.hpp"
#include "pbr/memory/AllocationInfo.hpp"

#include <source_location>
#include <utility>

#include <vulkan/vulkan_core.h>

namespace {
[[nodiscard]]
constexpr auto convertAllocationInfo(pbr::AllocationInfo const info) noexcept
    -> VmaAllocationCreateInfo {
  VmaMemoryUsage usage {};
  switch (info.preference) {
    using enum pbr::AllocationPreference;
  case Device:
    usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    break;
  case Host:
    usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST;
    break;
  }
  VmaAllocationCreateFlags flags {};
  switch (info.priority) {
    using enum pbr::AllocationPriority;
  case Memory:
    flags |= VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT;
    break;
  case Time:
    flags |= VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT;
    break;
  }
  if (info.ableToBeMapped) {
    if (info.persistentlyMapped) {
      flags |= VMA_ALLOCATION_CREATE_MAPPED_BIT;
    }
    if (info.randomAccess) {
      flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
    } else {
      flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
    }
  }
  return {
      .flags = flags,
      .usage = usage,
      .requiredFlags =
          info.preference == pbr::AllocationPreference::Host
              ? VkMemoryPropertyFlags{VK_MEMORY_PROPERTY_HOST_COHERENT_BIT}
              : VkMemoryPropertyFlags{},
      .preferredFlags {},
      .memoryTypeBits {},
      .pool {},
      .pUserData {},
      .priority {},
  };
}
} // namespace

pbr::MemoryAllocator::MemoryAllocator(core::SharedGpuHandle gpu)
    : _gpu(std::move(gpu)), _allocator() {
  VmaAllocatorCreateInfo const info {
      .flags {},
      .physicalDevice = _gpu->getPhysicalDevice(),
      .device = _gpu->getDevice(),
      .preferredLargeHeapBlockSize {},
      .pAllocationCallbacks {},
      .pDeviceMemoryCallbacks {},
      .pHeapSizeLimit {},
      .pVulkanFunctions {},
      .instance = _gpu->getInstance(),
      .vulkanApiVersion = vk::ApiVersion13,
      .pTypeExternalMemoryHandleTypes {},
  };
  vk::Result const allocatorResult {vmaCreateAllocator(&info, &_allocator)};
  if(allocatorResult != vk::Result::eSuccess) {
    vk::detail::throwResultException(allocatorResult,
                                     std::source_location::current().function_name());
  }
}

pbr::MemoryAllocator::MemoryAllocator(MemoryAllocator&& other) noexcept
    : _gpu(std::exchange(other._gpu, nullptr))
    , _allocator(std::exchange(other._allocator, nullptr)) {}

auto pbr::MemoryAllocator::operator=(MemoryAllocator&& rhs) noexcept -> MemoryAllocator& {
  std::swap(_gpu, rhs._gpu);
  std::swap(_allocator, rhs._allocator);
  return *this;
}

pbr::MemoryAllocator::~MemoryAllocator() noexcept {
  if (_allocator != nullptr) {
    vmaDestroyAllocator(std::exchange(_allocator, nullptr));
  }
}

auto pbr::MemoryAllocator::allocateBuffer(vk::BufferCreateInfo const bufferInfo,
                                          AllocationInfo const allocInfo)
    -> std::pair<Allocation, vk::UniqueBuffer> {
  auto const bufferInfoRaw = static_cast<VkBufferCreateInfo>(bufferInfo);
  auto const allocInfoRaw = ::convertAllocationInfo(allocInfo);
  VkBuffer bufferRaw {};
  VmaAllocation allocation {};
  VmaAllocationInfo allocationInfo {};
  vk::Result const result {vmaCreateBuffer(_allocator, &bufferInfoRaw, &allocInfoRaw,
                                           &bufferRaw, &allocation, &allocationInfo)};

  if(result != vk::Result::eSuccess) {
    vk::detail::throwResultException(result,
                                     std::source_location::current().function_name());
  }

  return std::make_pair(Allocation(_allocator, allocation, allocationInfo),
                        vk::UniqueBuffer(bufferRaw, _gpu->getDevice()));
}

auto pbr::MemoryAllocator::allocateImage(vk::ImageCreateInfo const imageInfo,
                                         AllocationInfo const allocInfo)
    -> std::pair<Allocation, vk::UniqueImage> {
  auto const imageInfoRaw = static_cast<VkImageCreateInfo>(imageInfo);
  auto const allocInfoRaw = ::convertAllocationInfo(allocInfo);
  VkImage imageRaw {};
  VmaAllocation allocation {};
  VmaAllocationInfo allocationInfo {};
  vk::Result const result {vmaCreateImage(_allocator, &imageInfoRaw, &allocInfoRaw,
                                          &imageRaw, &allocation, &allocationInfo)};

  if(result != vk::Result::eSuccess) {
    vk::detail::throwResultException(result,
                                     std::source_location::current().function_name());
  }

  return std::make_pair(Allocation(_allocator, allocation, allocationInfo),
                        vk::UniqueImage(imageRaw, _gpu->getDevice()));
}
