#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

namespace pbr {
struct PbrPipelineCreateInfo {
  vk::PipelineShaderStageCreateInfo vertexStage;
  vk::PipelineShaderStageCreateInfo fragmentStage;
};
class PbrPipeline {
  vk::UniquePipelineLayout _layout;
  vk::UniquePipeline _pipeline;

public:
  PbrPipeline(core::GpuHandle const& gpu, PbrPipelineCreateInfo info);

  [[nodiscard]]
  constexpr auto getPipelineLayout() const noexcept -> vk::PipelineLayout;
  [[nodiscard]]
  constexpr auto getPipeline() const noexcept -> vk::Pipeline;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto pbr::PbrPipeline::getPipelineLayout() const noexcept -> vk::PipelineLayout {
  return _layout.get();
}

constexpr auto pbr::PbrPipeline::getPipeline() const noexcept -> vk::Pipeline {
  return _pipeline.get();
}
