#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/HdrImage.hpp"
#include "pbr/memory/IAllocator.hpp"

namespace pbr {
class TonemapperSystem {
  core::SharedGpuHandle _gpu;

  vk::UniqueDescriptorSetLayout _descLayout;
  vk::UniquePipelineLayout _layout;
  vk::UniquePipeline _pipeline;

  vk::UniqueDescriptorPool _descPool;

public:
  TonemapperSystem(core::SharedGpuHandle gpu, vk::PipelineShaderStageCreateInfo shader);

  [[nodiscard]]
  auto allocateHdrImage(IAllocator& allocator, vk::Extent2D extent) -> HdrImage;

  auto run(vk::CommandBuffer cmdBuffer, HdrImage const& hdrImage) -> void;
};
}
