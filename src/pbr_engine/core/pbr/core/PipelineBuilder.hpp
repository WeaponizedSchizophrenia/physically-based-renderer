#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/VertexTraits.hpp"

#include <cstdint>
#include <optional>
#include <ranges>
#include <vector>

#include <vulkan/vulkan_format_traits.hpp>

namespace pbr::core {
class PipelineBuilder {
  std::vector<vk::PipelineShaderStageCreateInfo> _stages {};
  std::vector<vk::VertexInputBindingDescription> _vertexBindings {};
  std::vector<vk::VertexInputAttributeDescription> _vertexAttributes {};
  vk::PipelineVertexInputStateCreateInfo _vertexInput {};
  vk::PipelineInputAssemblyStateCreateInfo _inputAssembly {
      .topology = vk::PrimitiveTopology::eTriangleList,
  };
  vk::PipelineViewportStateCreateInfo _viewport {
      .viewportCount = 1,
      .scissorCount = 1,
  };
  vk::PipelineRasterizationStateCreateInfo _rasterization {
      .lineWidth = 1.0f,
  };
  vk::PipelineMultisampleStateCreateInfo _multisample {};
  std::optional<vk::PipelineDepthStencilStateCreateInfo> _depthStencil = std::nullopt;
  std::vector<vk::PipelineColorBlendAttachmentState> _colorBlendAttachments {};
  vk::PipelineColorBlendStateCreateInfo _colorBlend {};
  std::vector<vk::DynamicState> _dynamicStates {
      vk::DynamicState::eViewport,
      vk::DynamicState::eScissor,
  };
  vk::PipelineDynamicStateCreateInfo _dynamicState {};
  std::vector<vk::Format> _outputFormats {};
  std::optional<vk::Format> _depthFormat = std::nullopt;
  vk::PipelineRenderingCreateInfo _renderInfo {};

public:
  constexpr auto addStage(vk::PipelineShaderStageCreateInfo stages) -> PipelineBuilder&;
  template <Vertex T> constexpr auto addVertexBinding() -> PipelineBuilder&;
  constexpr auto
  enableBackFaceCulling(vk::FrontFace frontFace = {}) noexcept -> PipelineBuilder&;
  constexpr auto enableDepthTesting(vk::Format format) noexcept -> PipelineBuilder&;
  constexpr auto addOutputFormat(
      vk::Format format,
      vk::PipelineColorBlendAttachmentState blend = {
          .colorWriteMask =
              vk::FlagTraits<vk::ColorComponentFlagBits>::allFlags}) -> PipelineBuilder&;
  constexpr auto addDynamicState(vk::DynamicState state) -> PipelineBuilder&;

  [[nodiscard]]
  constexpr auto
  build(vk::PipelineLayout layout) noexcept -> vk::GraphicsPipelineCreateInfo;

private:
  constexpr auto setPointers() noexcept -> void;
};
} // namespace pbr::core

/* IMPLEMENTATIONS */

constexpr auto pbr::core::PipelineBuilder::addStage(
    vk::PipelineShaderStageCreateInfo stages) -> PipelineBuilder& {
  _stages.push_back(stages);
  return *this;
}

template <pbr::core::Vertex T>
constexpr auto pbr::core::PipelineBuilder::addVertexBinding() -> PipelineBuilder& {
  auto const binding = _vertexBindings.size();
  _vertexBindings.emplace_back(binding, sizeof(T), vk::VertexInputRate::eVertex);

  std::uint32_t startLocation =
      _vertexAttributes.empty() ? 0 : _vertexAttributes.back().location;
  std::uint32_t offset {};
  for (auto const [idx, format] :
       VertexTraits<T>::attributeFormats | std::views::enumerate) {
    _vertexAttributes.emplace_back(startLocation + idx, binding, format, offset);
    offset += vk::blockSize(format);
  }

  _vertexInput.setVertexBindingDescriptions(_vertexBindings)
      .setVertexAttributeDescriptions(_vertexAttributes);
  return *this;
}

constexpr auto pbr::core::PipelineBuilder::enableBackFaceCulling(
    vk::FrontFace frontFace) noexcept -> PipelineBuilder& {
  _rasterization.setCullMode(vk::CullModeFlagBits::eBack).setFrontFace(frontFace);
  return *this;
}

constexpr auto pbr::core::PipelineBuilder::enableDepthTesting(vk::Format format) noexcept
    -> PipelineBuilder& {
  _renderInfo.setDepthAttachmentFormat(format);
  _depthStencil = vk::PipelineDepthStencilStateCreateInfo {
      .depthTestEnable = vk::True,
      .depthWriteEnable = vk::True,
      .depthCompareOp = vk::CompareOp::eLess,
  };
  return *this;
}

constexpr auto
pbr::core::PipelineBuilder::addOutputFormat(vk::Format format, vk::PipelineColorBlendAttachmentState blend) -> PipelineBuilder& {
  _colorBlendAttachments.push_back(blend);
  _colorBlend.setAttachments(_colorBlendAttachments);
  _outputFormats.push_back(format);
  _renderInfo.setColorAttachmentFormats(_outputFormats);
  return *this;
}

constexpr auto
pbr::core::PipelineBuilder::addDynamicState(vk::DynamicState state) -> PipelineBuilder& {
  _dynamicStates.push_back(state);
  _dynamicState.setDynamicStates(_dynamicStates);
  return *this;
}

constexpr auto pbr::core::PipelineBuilder::build(vk::PipelineLayout layout) noexcept
    -> vk::GraphicsPipelineCreateInfo {
  setPointers();
  return vk::GraphicsPipelineCreateInfo {
      .pNext = &_renderInfo,
      .pVertexInputState = &_vertexInput,
      .pInputAssemblyState = &_inputAssembly,
      .pViewportState = &_viewport,
      .pRasterizationState = &_rasterization,
      .pMultisampleState = &_multisample,
      .pDepthStencilState =
          _depthStencil.transform([](auto& state) { return &state; }).value_or(nullptr),
      .pColorBlendState = &_colorBlend,
      .pDynamicState = &_dynamicState,
      .layout = layout,
  }
      .setStages(_stages);
}

constexpr auto pbr::core::PipelineBuilder::setPointers() noexcept -> void {
  _vertexInput.setVertexBindingDescriptions(_vertexBindings)
      .setVertexAttributeDescriptions(_vertexAttributes);
  _colorBlend.setAttachments(_colorBlendAttachments);
  _dynamicState.setDynamicStates(_dynamicStates);
  _renderInfo.setColorAttachmentFormats(_outputFormats);
}
