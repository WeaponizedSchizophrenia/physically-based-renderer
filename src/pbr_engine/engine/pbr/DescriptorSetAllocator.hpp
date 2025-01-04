#pragma once

#include "pbr/Vulkan.hpp"
#include "pbr/core/GpuHandle.hpp"
#include <concepts>
#include <cstddef>
#include <utility>
#include <vector>

namespace pbr {
class DescriptorSetAllocator {
  core::SharedGpuHandle _gpu;
  vk::DescriptorPool _descPool;
  vk::DescriptorSetLayout _layout;

public:
  constexpr DescriptorSetAllocator(core::SharedGpuHandle gpu, vk::DescriptorPool descPool,
                                   vk::DescriptorSetLayout layout) noexcept
      : _gpu(std::move(gpu)), _descPool(descPool), _layout(layout) {}

  [[nodiscard]]
  constexpr auto allocate(std::unsigned_integral auto count) const
      -> std::vector<vk::UniqueDescriptorSet>;
  [[nodiscard]]
  constexpr auto allocate() const -> vk::UniqueDescriptorSet;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto pbr::DescriptorSetAllocator::allocate(
    std::unsigned_integral auto count) const -> std::vector<vk::UniqueDescriptorSet> {
  std::vector layouts(static_cast<std::size_t>(count), _layout);
  return _gpu->getDevice().allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo {
      .descriptorPool = _descPool,
  }
                                                            .setSetLayouts(layouts));
}

constexpr auto pbr::DescriptorSetAllocator::allocate() const -> vk::UniqueDescriptorSet {
  return std::move(_gpu->getDevice()
                       .allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo {
                           .descriptorPool = _descPool,
                       }
                                                         .setSetLayouts(_layout))
                       .front());
}
