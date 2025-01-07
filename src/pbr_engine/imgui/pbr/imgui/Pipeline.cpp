#include "pbr/imgui/Pipeline.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/core/PipelineBuilder.hpp"
#include "pbr/core/VertexTraits.hpp"
#include "pbr/imgui/PushConstant.hpp"

#include "imgui.h"

#include <array>
#include <cassert>
#include <utility>

namespace pbr::core {
template <> struct VertexTraits<ImDrawVert> {
  static constexpr std::array attributeFormats {
      vk::Format::eR32G32Sfloat,
      vk::Format::eR32G32Sfloat,
      vk::Format::eR8G8B8A8Unorm,
  };
};
} // namespace pbr::core

namespace {
[[nodiscard]]
constexpr auto createFontSamplerLayout(pbr::core::GpuHandle const& gpu)
    -> vk::UniqueDescriptorSetLayout {
  vk::DescriptorSetLayoutBinding const binding {
      .binding = 0,
      .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 1,
      .stageFlags = vk::ShaderStageFlagBits::eFragment,
  };
  return gpu.getDevice().createDescriptorSetLayoutUnique(
      vk::DescriptorSetLayoutCreateInfo {}.setBindings(binding));
}
[[nodiscard]]
constexpr auto
createLayout(pbr::core::GpuHandle const& gpu,
             vk::DescriptorSetLayout fontSamplerLayout) -> vk::UniquePipelineLayout {
  vk::PushConstantRange const pcRange {
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .size = sizeof(pbr::imgui::PushConstant),
  };
  return gpu.getDevice().createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo {}
                                                        .setSetLayouts(fontSamplerLayout)
                                                        .setPushConstantRanges(pcRange));
}
[[nodiscard]]
constexpr auto createPipeline(pbr::core::GpuHandle const& gpu, vk::PipelineLayout layout,
                              pbr::imgui::PipelineCreateInfo info) -> vk::UniquePipeline {
  auto [result, pipeline] = gpu.getDevice().createGraphicsPipelineUnique(
      nullptr,
      pbr::core::PipelineBuilder()
          .addStage(info.vertexStage)
          .addStage(info.fragmentStage)
          .addVertexBinding<ImDrawVert>()
          .addOutputFormat(
              info.outputFormat,
              {
                  .blendEnable = vk::True,
                  .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
                  .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                  .colorBlendOp = vk::BlendOp::eAdd,
                  .srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
                  .dstAlphaBlendFactor = vk::BlendFactor::eZero,
                  .alphaBlendOp = vk::BlendOp::eAdd,
                  .colorWriteMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags,
              })
          .build(layout));
  assert(result == vk::Result::eSuccess);
  return std::move(pipeline);
}
} // namespace

pbr::imgui::Pipeline::Pipeline(core::GpuHandle const& gpu, PipelineCreateInfo info)
    : _fontSamplerLayout(::createFontSamplerLayout(gpu))
    , _layout(::createLayout(gpu, _fontSamplerLayout.get()))
    , _pipeline(::createPipeline(gpu, _layout.get(), info)) {}
