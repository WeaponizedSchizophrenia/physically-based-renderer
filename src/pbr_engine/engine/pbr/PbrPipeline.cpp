#include "pbr/PbrPipeline.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/MeshVertex.hpp"
#include <array>
#include <cassert>

namespace {
[[nodiscard]]
constexpr auto createLayout(pbr::core::GpuHandle const& gpu) -> vk::UniquePipelineLayout {
  return gpu.getDevice().createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo {});
}
[[nodiscard]]
constexpr auto createPipeline(pbr::core::GpuHandle const& gpu, vk::PipelineLayout layout,
                              pbr::PbrPipelineCreateInfo pbrInfo) -> vk::UniquePipeline {
  std::array const shaderStages {
      pbrInfo.vertexStage,
      pbrInfo.fragmentStage,
  };
  vk::VertexInputBindingDescription const meshVertexBinding {
      .stride = sizeof(pbr::MeshVertex),
  };
  auto const vertexInput =
      vk::PipelineVertexInputStateCreateInfo {}
          .setVertexBindingDescriptions(meshVertexBinding)
          .setVertexAttributeDescriptions(pbr::MeshVertex::attributes);
  vk::PipelineInputAssemblyStateCreateInfo const inputAssembly {
      .topology = vk::PrimitiveTopology::eTriangleList,
  };
  vk::PipelineTessellationStateCreateInfo const tesellation {};
  vk::PipelineViewportStateCreateInfo const viewport {
      .viewportCount = 1,
      .scissorCount = 1,
  };
  vk::PipelineRasterizationStateCreateInfo const rasterization {
      .lineWidth = 1.0f,
  };
  vk::PipelineMultisampleStateCreateInfo const multisample {};
  vk::PipelineColorBlendAttachmentState const colorBlendAttachment {
      .colorWriteMask = vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags,
  };
  auto const colorBlend =
      vk::PipelineColorBlendStateCreateInfo {}.setAttachments(colorBlendAttachment);
  std::array const dynamicStates {
      vk::DynamicState::eScissor,
      vk::DynamicState::eViewport,
  };
  auto const dynamicState =
      vk::PipelineDynamicStateCreateInfo {}.setDynamicStates(dynamicStates);

  auto const info =
      vk::GraphicsPipelineCreateInfo {
          .pVertexInputState = &vertexInput,
          .pInputAssemblyState = &inputAssembly,
          .pTessellationState = &tesellation,
          .pViewportState = &viewport,
          .pRasterizationState = &rasterization,
          .pMultisampleState = &multisample,
          .pColorBlendState = &colorBlend,
          .pDynamicState = &dynamicState,
          .layout = layout,
      }
          .setStages(shaderStages);
  std::array const attachmentFormats {
      vk::Format::eB8G8R8A8Srgb,
  };
  auto const renderInfo =
      vk::PipelineRenderingCreateInfo {}.setColorAttachmentFormats(attachmentFormats);

  auto [result, pipeline] = gpu.getDevice().createGraphicsPipelineUnique(
      nullptr, vk::StructureChain {info, renderInfo}.get());
  assert(result == vk::Result::eSuccess);
  return std::move(pipeline);
}
} // namespace

pbr::PbrPipeline::PbrPipeline(core::GpuHandle const& gpu, PbrPipelineCreateInfo info)
    : _layout(::createLayout(gpu)), _pipeline(::createPipeline(gpu, _layout, info)) {}
