#pragma once

#include "pbr/Vulkan.hpp"
#include "pbr/core/GpuHandle.hpp"

namespace pbr::imgui {
/**
 * Contains the dependencies and options for the imgui pipeline creation.
 */
struct PipelineCreateInfo {
  vk::PipelineShaderStageCreateInfo vertexStage{};
  vk::PipelineShaderStageCreateInfo fragmentStage{};
  /// The format of the pipeline output.
  vk::Format outputFormat{};
};
class Pipeline {
  vk::UniqueDescriptorSetLayout _fontSamplerLayout;
  vk::UniquePipelineLayout _layout;
  vk::UniquePipeline _pipeline;

public:
  explicit Pipeline(core::GpuHandle const& gpu, PipelineCreateInfo info);

  /* GETTERS */

  [[nodiscard]]
  constexpr auto getFontSamplerLayout() const noexcept -> vk::DescriptorSetLayout;
  [[nodiscard]]
  constexpr auto getPipelineLayout() const noexcept -> vk::PipelineLayout;
  [[nodiscard]]
  constexpr auto getPipeline() const noexcept -> vk::Pipeline;
};
} // namespace pbr::imgui

/* IMPLEMENTATIONS */

constexpr auto
pbr::imgui::Pipeline::getFontSamplerLayout() const noexcept -> vk::DescriptorSetLayout {
  return _fontSamplerLayout.get();
}
constexpr auto
pbr::imgui::Pipeline::getPipelineLayout() const noexcept -> vk::PipelineLayout {
  return _layout.get();
}
constexpr auto pbr::imgui::Pipeline::getPipeline() const noexcept -> vk::Pipeline {
  return _pipeline.get();
}
