#include "pbr/Material.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Uniform.hpp"

#include <memory>
#include <utility>

pbr::Material::Material(core::GpuHandle const& gpu, Uniform<MaterialData> matData,
                        std::shared_ptr<Image2D> colorTexture, std::shared_ptr<vk::UniqueSampler> colorSampler,
                        std::shared_ptr<Image2D> normalTexture, std::shared_ptr<vk::UniqueSampler> normalSampler,
                        vk::UniqueDescriptorSet descSet)
    : _materialData(std::move(matData))
    , _colorTexture(std::move(colorTexture))
    , _colorSampler(std::move(colorSampler))
    , _normalTexture(std::move(normalTexture))
    , _normalSampler(std::move(normalSampler))
    , _descriptorSet(std::move(descSet)) {
  writeDescriptorSet(gpu);
}

auto pbr::Material::writeDescriptorSet(core::GpuHandle const& gpu) -> void {
  vk::DescriptorBufferInfo const bufferInfo {
      .buffer = _materialData.getUniformBuffer().getBuffer(),
      .range = sizeof(MaterialData),
  };
  vk::DescriptorImageInfo const colorImageInfo {
      .sampler = _colorSampler->get(),
      .imageView = _colorTexture->getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
  };
  vk::DescriptorImageInfo const normalImageInfo {
      .sampler = _normalSampler->get(),
      .imageView = _normalTexture->getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
  };
  gpu.getDevice().updateDescriptorSets(
      {
          vk::WriteDescriptorSet {
              .dstSet = _descriptorSet.get(),
              .dstBinding = 0,
              .descriptorType = vk::DescriptorType::eUniformBuffer,
          }
              .setBufferInfo(bufferInfo),
          vk::WriteDescriptorSet {
              .dstSet = _descriptorSet.get(),
              .dstBinding = 1,
              .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          }
              .setImageInfo(colorImageInfo),
          vk::WriteDescriptorSet {
              .dstSet = _descriptorSet.get(),
              .dstBinding = 2,
              .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          }
              .setImageInfo(normalImageInfo),
      },
      {});
}
