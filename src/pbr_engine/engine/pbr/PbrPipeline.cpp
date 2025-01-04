#include "pbr/PbrPipeline.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/MeshVertex.hpp"
#include "pbr/ModelPushConstant.hpp"

#include <array>
#include <cassert>
#include <span>

namespace {
[[nodiscard]]
constexpr auto
createCameraSetLayout(pbr::core::GpuHandle const& gpu) -> vk::UniqueDescriptorSetLayout {
  std::array const bindings {
      vk::DescriptorSetLayoutBinding {
          .binding = 0,
          .descriptorType = vk::DescriptorType::eUniformBuffer,
          .descriptorCount = 1,
          .stageFlags =
              vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
      },
  };
  return gpu.getDevice().createDescriptorSetLayoutUnique(
      vk::DescriptorSetLayoutCreateInfo {}.setBindings(bindings));
}
[[nodiscard]]
constexpr auto createMaterialSetLayout(pbr::core::GpuHandle const& gpu)
    -> vk::UniqueDescriptorSetLayout {
  std::array const bindings {
      vk::DescriptorSetLayoutBinding {
          .binding = 0,
          .descriptorType = vk::DescriptorType::eUniformBuffer,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment,
      },
      vk::DescriptorSetLayoutBinding {
          .binding = 1,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment,
      },
  };
  return gpu.getDevice().createDescriptorSetLayoutUnique(
      vk::DescriptorSetLayoutCreateInfo {}.setBindings(bindings));
}
[[nodiscard]]
constexpr auto createLayout(pbr::core::GpuHandle const& gpu,
                            std::span<vk::DescriptorSetLayout const> setLayouts)
    -> vk::UniquePipelineLayout {
  std::array const pushConstantRanges {
      vk::PushConstantRange {
          .stageFlags = vk::ShaderStageFlagBits::eVertex,
          .offset = 0,
          .size = sizeof(pbr::ModelPushConstant),
      },
  };
  return gpu.getDevice().createPipelineLayoutUnique(
      vk::PipelineLayoutCreateInfo {}
          .setPushConstantRanges(pushConstantRanges)
          .setSetLayouts(setLayouts));
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
  auto const attributes = pbr::getMeshVertexAttributes();
  auto const vertexInput =
      vk::PipelineVertexInputStateCreateInfo {}
          .setVertexBindingDescriptions(meshVertexBinding)
          .setVertexAttributeDescriptions(attributes);
  vk::PipelineInputAssemblyStateCreateInfo const inputAssembly {
      .topology = vk::PrimitiveTopology::eTriangleList,
  };
  vk::PipelineTessellationStateCreateInfo const tesellation {};
  vk::PipelineViewportStateCreateInfo const viewport {
      .viewportCount = 1,
      .scissorCount = 1,
  };
  vk::PipelineRasterizationStateCreateInfo const rasterization {
      .cullMode = vk::CullModeFlagBits::eBack,
      .frontFace = vk::FrontFace::eClockwise,
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
  auto const renderInfo =
      vk::PipelineRenderingCreateInfo {}.setColorAttachmentFormats(pbrInfo.outputFormat);

  auto [result, pipeline] = gpu.getDevice().createGraphicsPipelineUnique(
      nullptr, vk::StructureChain {info, renderInfo}.get());
  assert(result == vk::Result::eSuccess);
  return std::move(pipeline);
}
} // namespace

pbr::PbrPipeline::PbrPipeline(core::GpuHandle const& gpu, PbrPipelineCreateInfo info)
    : _cameraSetLayout(::createCameraSetLayout(gpu))
    , _materialSetLayout(::createMaterialSetLayout(gpu))
    , _layout(::createLayout(
          gpu, std::array {_cameraSetLayout.get(), _materialSetLayout.get()}))
    , _pipeline(::createPipeline(gpu, _layout, info)) {}
