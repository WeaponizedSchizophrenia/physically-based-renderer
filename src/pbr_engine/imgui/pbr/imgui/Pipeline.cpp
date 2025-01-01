#include "pbr/imgui/Pipeline.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/imgui/PushConstant.hpp"

#include "imgui.h"

#include <array>
#include <cassert>
#include <cstddef>
#include <utility>

namespace constants {
static constexpr vk::VertexInputBindingDescription VERTEX_BINDING {
    .stride = sizeof(ImDrawVert),
    .inputRate = vk::VertexInputRate::eVertex,
};
static constexpr std::array VERTEX_ATTRIBUTES {
    vk::VertexInputAttributeDescription {
        .location = 0,
        .format = vk::Format::eR32G32Sfloat,
        .offset = offsetof(ImDrawVert, pos),
    },
    vk::VertexInputAttributeDescription {
        .location = 1,
        .format = vk::Format::eR32G32Sfloat,
        .offset = offsetof(ImDrawVert, uv),
    },
    vk::VertexInputAttributeDescription {
        .location = 2,
        .format = vk::Format::eR8G8B8A8Unorm,
        .offset = offsetof(ImDrawVert, col),
    },
};
} // namespace constants

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
  std::array const stages {info.vertexStage, info.fragmentStage};
  auto const vertexInput =
      vk::PipelineVertexInputStateCreateInfo {}
          .setVertexBindingDescriptions(constants::VERTEX_BINDING)
          .setVertexAttributeDescriptions(constants::VERTEX_ATTRIBUTES);
  vk::PipelineInputAssemblyStateCreateInfo const inputAssembly {
      .topology = vk::PrimitiveTopology::eTriangleList,
  };
  vk::PipelineViewportStateCreateInfo const viewport {
      .viewportCount = 1,
      .scissorCount = 1,
  };
  vk::PipelineRasterizationStateCreateInfo const rasterization {.lineWidth = 1.0f};
  vk::PipelineMultisampleStateCreateInfo const multisample {};
  vk::PipelineColorBlendAttachmentState const colorBlendAttachment {
      .blendEnable = vk::True,
      .srcColorBlendFactor = vk::BlendFactor::eSrcAlpha,
      .dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
      .colorBlendOp = vk::BlendOp::eAdd,
      .srcAlphaBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha,
      .dstAlphaBlendFactor = vk::BlendFactor::eZero,
      .alphaBlendOp = vk::BlendOp::eAdd,
      .colorWriteMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags,
  };
  auto const colorBlend =
      vk::PipelineColorBlendStateCreateInfo {}.setAttachments(colorBlendAttachment);
  std::array const dynamicStates {vk::DynamicState::eViewport,
                                  vk::DynamicState::eScissor};
  auto const dynamicState =
      vk::PipelineDynamicStateCreateInfo {}.setDynamicStates(dynamicStates);

  auto const pipelineInfo =
      vk::GraphicsPipelineCreateInfo {
          .pVertexInputState = &vertexInput,
          .pInputAssemblyState = &inputAssembly,
          .pViewportState = &viewport,
          .pRasterizationState = &rasterization,
          .pMultisampleState = &multisample,
          .pColorBlendState = &colorBlend,
          .pDynamicState = &dynamicState,
          .layout = layout,
      }
          .setStages(stages);
  auto const renderInfo =
      vk::PipelineRenderingCreateInfo {}.setColorAttachmentFormats(info.outputFormat);

  auto [result, pipeline] = gpu.getDevice().createGraphicsPipelineUnique(
      nullptr, vk::StructureChain {pipelineInfo, renderInfo}.get());
  assert(result == vk::Result::eSuccess);
  return std::move(pipeline);
}
} // namespace

pbr::imgui::Pipeline::Pipeline(core::GpuHandle const& gpu, PipelineCreateInfo info)
    : _fontSamplerLayout(::createFontSamplerLayout(gpu))
    , _layout(::createLayout(gpu, _fontSamplerLayout.get()))
    , _pipeline(::createPipeline(gpu, _layout.get(), info)) {}
