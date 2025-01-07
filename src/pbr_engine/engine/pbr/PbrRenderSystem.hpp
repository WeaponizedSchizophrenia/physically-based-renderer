#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/GBuffer.hpp"
#include "pbr/Image2D.hpp"
#include "pbr/Scene.hpp"
#include "pbr/memory/IAllocator.hpp"

namespace pbr {
struct PbrRenderSystemCreateInfo {
  vk::PipelineShaderStageCreateInfo geometryVertexShader {};
  vk::PipelineShaderStageCreateInfo geometryFragmentShader {};
  vk::PipelineShaderStageCreateInfo lightingVertexShader {};
  vk::PipelineShaderStageCreateInfo lightingFragmentShader {};
};
class PbrRenderSystem {
public:
  static constexpr auto LIGHTING_PASS_OUTPUT_FORMAT = vk::Format::eR16G16B16A16Unorm;

private:
  core::SharedGpuHandle _gpu;

  vk::UniqueDescriptorSetLayout _sceneDescSetLayout;
  vk::UniqueDescriptorSetLayout _materialDescSetLayout;
  vk::UniqueSampler _gBufferSampler;
  vk::UniqueSampler _depthSampler;
  vk::UniqueDescriptorSetLayout _gBufferDescSetLayout;

  vk::UniqueDescriptorPool _gBufferDescriptorPool;

  vk::UniquePipelineLayout _geometryLayout;
  vk::UniquePipeline _geometryPipeline;

  vk::UniquePipelineLayout _lightingLayout;
  vk::UniquePipeline _lightingPipeline;

public:
  PbrRenderSystem(core::SharedGpuHandle gpu, PbrRenderSystemCreateInfo info);

  [[nodiscard]]
  auto allocateGBuffer(IAllocator& allocator, vk::Extent2D extent) -> GBuffer;

  auto render(vk::CommandBuffer cmdBuffer, Scene const& scene, GBuffer const& gBuffer,
              Image2D const& renderTarget, vk::Extent2D renderExtent) -> void;

private:
  auto recordGeometryPass(vk::CommandBuffer cmdBuffer, Scene const& scene,
                          GBuffer const& gBuffer) -> void;
  auto recordLightingPass(vk::CommandBuffer cmdBuffer, Scene const& scene, GBuffer const& gBuffer,
                          Image2D const& renderTo, vk::Rect2D renderArea) -> void;
};
} // namespace pbr
