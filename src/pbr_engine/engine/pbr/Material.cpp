#include "pbr/Material.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Uniform.hpp"

#include <memory>
#include <utility>

pbr::Material::Material(core::GpuHandle const& gpu, Uniform<MaterialData> matData,
                        std::shared_ptr<Image2D> colorTexture, std::shared_ptr<vk::UniqueSampler> sampler,
                        vk::UniqueDescriptorSet descSet)
    : _materialData(std::move(matData))
    , _colorTexture(std::move(colorTexture))
    , _colorSampler(std::move(sampler))
    , _descriptorSet(std::move(descSet)) {
  writeDescriptorSet(gpu);
}

auto pbr::Material::writeDescriptorSet(core::GpuHandle const& gpu) -> void {
  vk::DescriptorBufferInfo const bufferInfo {
      .buffer = _materialData.getUniformBuffer().getBuffer(),
      .range = sizeof(MaterialData),
  };
  vk::DescriptorImageInfo const imageInfo {
      .sampler = _colorSampler->get(),
      .imageView = _colorTexture->getImageView(),
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
              .setImageInfo(imageInfo),
      },
      {});
}
