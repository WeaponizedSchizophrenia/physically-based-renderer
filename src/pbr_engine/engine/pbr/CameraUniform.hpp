#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/CameraData.hpp"
#include "pbr/Uniform.hpp"
#include "pbr/core/GpuHandle.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <utility>

namespace pbr {
class CameraUniform : public Uniform<CameraData> {
  vk::UniqueDescriptorSet _descriptorSet;

public:
  CameraUniform(core::GpuHandle const& gpu, IAllocator& allocator,
                vk::UniqueDescriptorSet descSet, CameraData init = {})
      : Uniform(allocator, init), _descriptorSet(std::move(descSet)) {
    vk::DescriptorBufferInfo const bufferInfo {
        .buffer = getUniformBuffer().getBuffer(),
        .range = sizeof(CameraData),
    };
    gpu.getDevice().updateDescriptorSets(
        vk::WriteDescriptorSet {
            .dstSet = _descriptorSet.get(),
            .descriptorType = vk::DescriptorType::eUniformBuffer,
        }
            .setBufferInfo(bufferInfo),
        {});
  }

  [[nodiscard]]
  constexpr auto getDescriptorSet() const noexcept -> vk::DescriptorSet;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto
pbr::CameraUniform::getDescriptorSet() const noexcept -> vk::DescriptorSet {
  return _descriptorSet.get();
}
