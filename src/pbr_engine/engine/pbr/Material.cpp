#include "pbr/Material.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Uniform.hpp"
#include "pbr/memory/IAllocator.hpp"
#include <utility>

pbr::Material::Material(core::GpuHandle const& gpu, IAllocator& allocator,
                        vk::DescriptorPool descPool, vk::DescriptorSetLayout setLayout,
                        MaterialData materialData)
    : _materialData(allocator, materialData)
    , _descriptorSet(
          std::move(gpu.getDevice()
                        .allocateDescriptorSetsUnique(
                            vk::DescriptorSetAllocateInfo {.descriptorPool = descPool}
                                .setSetLayouts(setLayout))
                        .front())) {
  vk::DescriptorBufferInfo const bufferInfo {
      .buffer = _materialData.getUniformBuffer().getBuffer(),
      .range = sizeof(MaterialData),
  };
  gpu.getDevice().updateDescriptorSets(
      vk::WriteDescriptorSet {
          .dstSet = _descriptorSet.get(),
          .dstBinding = 0,
          .descriptorType = vk::DescriptorType::eUniformBuffer,
      }
          .setBufferInfo(bufferInfo),
      {});
}
