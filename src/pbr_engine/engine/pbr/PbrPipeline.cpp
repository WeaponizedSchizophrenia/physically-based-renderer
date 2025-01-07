#include "pbr/PbrPipeline.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/MeshVertex.hpp"
#include "pbr/ModelPushConstant.hpp"
#include "pbr/core/PipelineBuilder.hpp"

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
      vk::DescriptorSetLayoutBinding {
          .binding = 2,
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
  auto [result, pipeline] = gpu.getDevice().createGraphicsPipelineUnique(
      nullptr, pbr::core::PipelineBuilder()
                   .addStage(pbrInfo.vertexStage)
                   .addStage(pbrInfo.fragmentStage)
                   .addVertexBinding<pbr::MeshVertex>()
                   .enableBackFaceCulling(vk::FrontFace::eClockwise)
                   .addOutputFormat(pbrInfo.outputFormat)
                   .build(layout));
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
