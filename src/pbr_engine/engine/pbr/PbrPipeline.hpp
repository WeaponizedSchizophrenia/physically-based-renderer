#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

namespace pbr {
struct PbrPipelineCreateInfo {
  vk::PipelineShaderStageCreateInfo vertexStage {};
  vk::PipelineShaderStageCreateInfo fragmentStage {};
  vk::Format outputFormat {};
};
class PbrPipeline {
  vk::UniqueDescriptorSetLayout _cameraSetLayout;
  vk::UniquePipelineLayout _layout;
  vk::UniquePipeline _pipeline;

public:
  PbrPipeline(core::GpuHandle const& gpu, PbrPipelineCreateInfo info);

  [[nodiscard]]
  constexpr auto getCameraSetLayout() const noexcept -> vk::DescriptorSetLayout;
  [[nodiscard]]
  constexpr auto getPipelineLayout() const noexcept -> vk::PipelineLayout;
  [[nodiscard]]
  constexpr auto getPipeline() const noexcept -> vk::Pipeline;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto
pbr::PbrPipeline::getCameraSetLayout() const noexcept -> vk::DescriptorSetLayout {
  return _cameraSetLayout.get();
}

constexpr auto
pbr::PbrPipeline::getPipelineLayout() const noexcept -> vk::PipelineLayout {
  return _layout.get();
}

constexpr auto pbr::PbrPipeline::getPipeline() const noexcept -> vk::Pipeline {
  return _pipeline.get();
}
