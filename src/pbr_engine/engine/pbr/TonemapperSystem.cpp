#include "pbr/TonemapperSystem.hpp"

#include "pbr/DescriptorSetAllocator.hpp"
#include "pbr/HdrImage.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>

namespace constants {
static constexpr std::uint32_t MAX_HDR_IMAGE_DESCRIPTOR_SETS = 30;
static constexpr std::uint32_t LOCAL_SIZE = 16;
} // namespace constants

namespace {
[[nodiscard]]
constexpr auto createDescriptorSetLayout(pbr::core::GpuHandle const& gpu)
    -> vk::UniqueDescriptorSetLayout {
  std::array const bindings {
      vk::DescriptorSetLayoutBinding {
          .binding = 0,
          .descriptorType = vk::DescriptorType::eStorageImage,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eCompute,
      },
      vk::DescriptorSetLayoutBinding {
          .binding = 1,
          .descriptorType = vk::DescriptorType::eStorageImage,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eCompute,
      },
  };
  return gpu.getDevice().createDescriptorSetLayoutUnique(
      vk::DescriptorSetLayoutCreateInfo {}.setBindings(bindings));
}
[[nodiscard]]
constexpr auto
createPipelineLayout(pbr::core::GpuHandle const& gpu,
                     vk::DescriptorSetLayout descLayout) -> vk::UniquePipelineLayout {
  return gpu.getDevice().createPipelineLayoutUnique(
      vk::PipelineLayoutCreateInfo {}.setSetLayouts(descLayout));
}
[[nodiscard]]
constexpr auto
createPipeline(pbr::core::GpuHandle const& gpu, vk::PipelineLayout layout,
               vk::PipelineShaderStageCreateInfo shader) -> vk::UniquePipeline {
  auto [result, pipeline] =
      gpu.getDevice().createComputePipelineUnique(nullptr, {
                                                               .stage = shader,
                                                               .layout = layout,
                                                           });
  assert(result == vk::Result::eSuccess);
  return std::move(pipeline);
}
[[nodiscard]]
constexpr auto createDescriptorPool(pbr::core::GpuHandle const& gpu,
                                    std::uint32_t setCount) -> vk::UniqueDescriptorPool {
  std::array const sizes {
      vk::DescriptorPoolSize {
          .type = vk::DescriptorType::eStorageImage,
          .descriptorCount = setCount * 2,
      },
  };
  return gpu.getDevice().createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo {
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = setCount,
  }
                                                        .setPoolSizes(sizes));
}
} // namespace

pbr::TonemapperSystem::TonemapperSystem(core::SharedGpuHandle gpu,
                                        vk::PipelineShaderStageCreateInfo shader)
    : _gpu(std::move(gpu))
    , _descLayout(::createDescriptorSetLayout(*_gpu))
    , _layout(::createPipelineLayout(*_gpu, _descLayout.get()))
    , _pipeline(::createPipeline(*_gpu, _layout, shader))
    , _descPool(::createDescriptorPool(*_gpu, constants::MAX_HDR_IMAGE_DESCRIPTOR_SETS)) {
}

auto pbr::TonemapperSystem::allocateHdrImage(IAllocator& allocator,
                                             vk::Extent2D extent) -> HdrImage {
  return {
      _gpu,
      allocator,
      DescriptorSetAllocator(_gpu, _descPool.get(), _descLayout.get()).allocate(),
      extent,
  };
}

auto pbr::TonemapperSystem::run(vk::CommandBuffer cmdBuffer,
                                HdrImage const& hdrImage) -> void {
  std::array const barriers {
      vk::ImageMemoryBarrier2 {
          .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
          .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader,
          .dstAccessMask = vk::AccessFlagBits2::eShaderStorageWrite,
          .newLayout = vk::ImageLayout::eGeneral,
          .image = hdrImage.getOutputImage(),
          .subresourceRange {
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .levelCount = 1,
              .layerCount = 1,
          },
      },
      vk::ImageMemoryBarrier2 {
          .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
          .dstStageMask = vk::PipelineStageFlagBits2::eComputeShader,
          .dstAccessMask = vk::AccessFlagBits2::eShaderStorageWrite,
          .newLayout = vk::ImageLayout::eGeneral,
          .image = hdrImage.getImage().getImage(),
          .subresourceRange {
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .levelCount = 1,
              .layerCount = 1,
          },
      },
  };
  cmdBuffer.pipelineBarrier2(vk::DependencyInfo {}.setImageMemoryBarriers(barriers));

  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, _pipeline.get());
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, _layout.get(), 0,
                               hdrImage.getDescriptorSet(), {});

  auto const extent = hdrImage.getExtent();
  cmdBuffer.dispatch(std::ceil(static_cast<float>(extent.width) / constants::LOCAL_SIZE),
                     std::ceil(static_cast<float>(extent.height) / constants::LOCAL_SIZE), 1);
}
