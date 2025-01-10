#include "pbr/HdrImage.hpp"

#include "pbr/PbrRenderSystem.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/memory/IAllocator.hpp"
#include <utility>

pbr::HdrImage::HdrImage(core::SharedGpuHandle gpu, IAllocator& allocator,
                        vk::UniqueDescriptorSet descSet, vk::Extent2D extent)
    : _gpu(std::move(gpu))
    , _image(*_gpu, PbrRenderSystem::LIGHTING_PASS_OUTPUT_FORMAT,
             vk::ImageAspectFlagBits::eColor,
             allocator.allocateImage(
                 {
                     .imageType = vk::ImageType::e2D,
                     .format = PbrRenderSystem::LIGHTING_PASS_OUTPUT_FORMAT,
                     .extent {
                         .width = extent.width,
                         .height = extent.height,
                         .depth = 1,
                     },
                     .mipLevels = 1,
                     .arrayLayers = 1,
                     .usage = vk::ImageUsageFlagBits::eStorage
                              | vk::ImageUsageFlagBits::eColorAttachment,
                 },
                 {}))
    , _extent(extent)
    , _descSet(std::move(descSet)) {
  vk::DescriptorImageInfo const hdrImageInfo {
      .imageView = _image.getImageView(),
      .imageLayout = vk::ImageLayout::eGeneral,
  };
  _gpu->getDevice().updateDescriptorSets(
      vk::WriteDescriptorSet {
          .dstSet = _descSet.get(),
          .descriptorType = vk::DescriptorType::eStorageImage,
      }
          .setImageInfo(hdrImageInfo),
      {});
}

auto pbr::HdrImage::updateOutputTexture(vk::Image image, vk::ImageView imageView) -> void {
  if(_outputImage == image) {
    return;
  }

  _outputImage = image;
  vk::DescriptorImageInfo const imageInfo {
      .imageView = imageView,
      .imageLayout = vk::ImageLayout::eGeneral,
  };
  _gpu->getDevice().updateDescriptorSets(
      vk::WriteDescriptorSet {
          .dstSet = _descSet.get(),
          .dstBinding = 1,
          .descriptorType = vk::DescriptorType::eStorageImage,
      }
          .setImageInfo(imageInfo),
      {});
}
