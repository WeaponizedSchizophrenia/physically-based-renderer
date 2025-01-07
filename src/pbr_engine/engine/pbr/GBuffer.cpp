#include "pbr/GBuffer.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/memory/IAllocator.hpp"
#include <array>
#include <utility>

namespace {
[[nodiscard]]
constexpr auto
allocateImage2D(pbr::core::GpuHandle const& gpu, pbr::IAllocator& allocator,
                vk::Extent2D extent, vk::Format format,
                vk::ImageAspectFlags aspect = vk::ImageAspectFlagBits::eColor,
                vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eColorAttachment)
    -> pbr::Image2D {
  return {
      gpu,
      format,
      aspect,
      allocator.allocateImage(
          {
              .imageType = vk::ImageType::e2D,
              .format = format,
              .extent {
                  .width = extent.width,
                  .height = extent.height,
                  .depth = 1,
              },
              .mipLevels = 1,
              .arrayLayers = 1,
              .usage = usage | vk::ImageUsageFlagBits::eSampled,
          },
          {}),
  };
}
} // namespace

pbr::GBuffer::GBuffer(core::GpuHandle const& gpu, IAllocator& allocator,
                      vk::UniqueDescriptorSet descSet, vk::Extent2D extent)
    : _positions(::allocateImage2D(gpu, allocator, extent, POSITIONS_FORMAT))
    , _normals(::allocateImage2D(gpu, allocator, extent, NORMALS_FORMAT))
    , _albedo(::allocateImage2D(gpu, allocator, extent, ALBEDO_FORMAT))
    , _depth(::allocateImage2D(gpu, allocator, extent, DEPTH_FORMAT,
                               vk::ImageAspectFlagBits::eDepth,
                               vk::ImageUsageFlagBits::eDepthStencilAttachment))
    , _extent(extent)
    , _descSet(std::move(descSet)) {
  vk::DescriptorImageInfo const positionsInfo {
      .imageView = _positions.getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
  };
  vk::DescriptorImageInfo const normalsInfo {
      .imageView = _normals.getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
  };
  vk::DescriptorImageInfo const albedoInfo {
      .imageView = _albedo.getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
  };
  vk::DescriptorImageInfo const depthInfo {
      .imageView = _depth.getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
  };
  gpu.getDevice().updateDescriptorSets(
      std::array {
          vk::WriteDescriptorSet {
              .dstSet = _descSet,
              .dstBinding = 0,
              .descriptorType = vk::DescriptorType::eSampledImage,
          }
              .setImageInfo(positionsInfo),
          vk::WriteDescriptorSet {
              .dstSet = _descSet,
              .dstBinding = 1,
              .descriptorType = vk::DescriptorType::eSampledImage,
          }
              .setImageInfo(normalsInfo),
          vk::WriteDescriptorSet {
              .dstSet = _descSet,
              .dstBinding = 2,
              .descriptorType = vk::DescriptorType::eSampledImage,
          }
              .setImageInfo(albedoInfo),
          vk::WriteDescriptorSet {
              .dstSet = _descSet,
              .dstBinding = 4,
              .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          }
              .setImageInfo(depthInfo),
      },
      {});
}
