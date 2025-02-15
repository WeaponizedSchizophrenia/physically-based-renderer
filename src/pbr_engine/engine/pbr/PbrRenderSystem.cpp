#include "pbr/PbrRenderSystem.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/core/PipelineBuilder.hpp"

#include "pbr/GBuffer.hpp"
#include "pbr/Image2D.hpp"
#include "pbr/MeshVertex.hpp"
#include "pbr/ModelPushConstant.hpp"
#include "pbr/Scene.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <span>
#include <utility>

namespace constants {
static constexpr std::uint32_t MAX_G_BUFFER_DESCRIPTOR_SETS = 30;
}

namespace {
[[nodiscard]]
constexpr auto createSceneDescriptorSetLayout(pbr::core::GpuHandle const& gpu)
    -> vk::UniqueDescriptorSetLayout {
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
constexpr auto createMaterialDescriptorSetLayout(pbr::core::GpuHandle const& gpu)
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
constexpr auto createGBufferSampler(pbr::core::GpuHandle const& gpu)
    -> vk::UniqueSampler {
  return gpu.getDevice().createSamplerUnique({
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
  });
}
[[nodiscard]]
constexpr auto createDepthSampler(pbr::core::GpuHandle const& gpu) -> vk::UniqueSampler {
  return gpu.getDevice().createSamplerUnique({
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
  });
}
[[nodiscard]]
constexpr auto createGBufferDescriptorSetLayout(pbr::core::GpuHandle const& gpu,
                                                vk::Sampler gBufferSampler,
                                                vk::Sampler depthSampler)
    -> vk::UniqueDescriptorSetLayout {
  std::array const bindings {
      vk::DescriptorSetLayoutBinding {
          .binding = 0,
          .descriptorType = vk::DescriptorType::eSampledImage,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment,
      },
      vk::DescriptorSetLayoutBinding {
          .binding = 1,
          .descriptorType = vk::DescriptorType::eSampledImage,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment,
      },
      vk::DescriptorSetLayoutBinding {
          .binding = 2,
          .descriptorType = vk::DescriptorType::eSampledImage,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment,
      },
      vk::DescriptorSetLayoutBinding {
          .binding = 3,
          .descriptorType = vk::DescriptorType::eSampler,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment,
          .pImmutableSamplers = &gBufferSampler,
      },
      vk::DescriptorSetLayoutBinding {
          .binding = 4,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = 1,
          .stageFlags = vk::ShaderStageFlagBits::eFragment,
          .pImmutableSamplers = &depthSampler,
      },
  };
  return gpu.getDevice().createDescriptorSetLayoutUnique(
      vk::DescriptorSetLayoutCreateInfo {}.setBindings(bindings));
}
[[nodiscard]]
constexpr auto createGBufferDescriptorPool(pbr::core::GpuHandle const& gpu,
                                           std::uint32_t maxSetCount)
    -> vk::UniqueDescriptorPool {
  std::array const sizes {
      vk::DescriptorPoolSize {
          .type = vk::DescriptorType::eSampledImage,
          .descriptorCount = maxSetCount * 3,
      },
      vk::DescriptorPoolSize {
          .type = vk::DescriptorType::eSampler,
          .descriptorCount = maxSetCount,
      },
      vk::DescriptorPoolSize {
          .type = vk::DescriptorType::eCombinedImageSampler,
          .descriptorCount = maxSetCount,
      },
  };
  return gpu.getDevice().createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo {
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = maxSetCount,
  }
                                                        .setPoolSizes(sizes));
}
[[nodiscard]]
constexpr auto
createGeometryPipelineLayout(pbr::core::GpuHandle const& gpu,
                             std::span<vk::DescriptorSetLayout const> descLayouts)
    -> vk::UniquePipelineLayout {
  vk::PushConstantRange const pcRange {
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .size = sizeof(pbr::ModelPushConstant),
  };
  return gpu.getDevice().createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo {}
                                                        .setSetLayouts(descLayouts)
                                                        .setPushConstantRanges(pcRange));
}
[[nodiscard]]
constexpr auto
createLightingPipelineLayout(pbr::core::GpuHandle const& gpu,
                             std::span<vk::DescriptorSetLayout const> descLayouts)
    -> vk::UniquePipelineLayout {
  return gpu.getDevice().createPipelineLayoutUnique(
      vk::PipelineLayoutCreateInfo {}.setSetLayouts(descLayouts));
}
[[nodiscard]]
constexpr auto buildGeometryPipeline(pbr::PbrRenderSystemCreateInfo info)
    -> pbr::core::PipelineBuilder {
  return pbr::core::PipelineBuilder()
      .addStage(info.geometryVertexShader)
      .addStage(info.geometryFragmentShader)
      .addVertexBinding<pbr::MeshVertex>()
      .addOutputFormat(pbr::GBuffer::POSITIONS_FORMAT)
      .addOutputFormat(pbr::GBuffer::NORMALS_FORMAT)
      .addOutputFormat(pbr::GBuffer::ALBEDO_FORMAT)
      .enableDepthTesting(pbr::GBuffer::DEPTH_FORMAT)
      .enableBackFaceCulling(vk::FrontFace::eClockwise);
}
[[nodiscard]]
constexpr auto buildLightingPipeline(pbr::PbrRenderSystemCreateInfo info)
    -> pbr::core::PipelineBuilder {
  return pbr::core::PipelineBuilder()
      .addStage(info.lightingVertexShader)
      .addStage(info.lightingFragmentShader)
      .addOutputFormat(pbr::PbrRenderSystem::LIGHTING_PASS_OUTPUT_FORMAT);
}
[[nodiscard]]
constexpr auto getToColorAttachmentBarrier(vk::Image image) -> vk::ImageMemoryBarrier2 {
  return {
      .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
      // .srcAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
      .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
      // .oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .image = image,
      .subresourceRange {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .levelCount = 1,
          .layerCount = 1,
      },
  };
}
[[nodiscard]]
constexpr auto getToDepthAttachmentBarrier(vk::Image image) -> vk::ImageMemoryBarrier2 {
  return {
      .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
      // .srcAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
      .dstStageMask = vk::PipelineStageFlagBits2::eLateFragmentTests,
      .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
      // .oldLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      .newLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .image = image,
      .subresourceRange {
          .aspectMask = vk::ImageAspectFlagBits::eDepth,
          .levelCount = 1,
          .layerCount = 1,
      },
  };
}
[[nodiscard]]
constexpr auto getToColorReadBarrier(vk::Image image) -> vk::ImageMemoryBarrier2 {
  return {
      .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
      .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
      .dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
      .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      .image = image,
      .subresourceRange {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .levelCount = 1,
          .layerCount = 1,
      },
  };
}
[[nodiscard]]
constexpr auto getToDepthReadBarrier(vk::Image image) -> vk::ImageMemoryBarrier2 {
  return {
      .srcStageMask = vk::PipelineStageFlagBits2::eLateFragmentTests,
      .srcAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
      .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
      .dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
      .oldLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      .image = image,
      .subresourceRange {
          .aspectMask = vk::ImageAspectFlagBits::eDepth,
          .levelCount = 1,
          .layerCount = 1,
      },
  };
}
constexpr auto switchGBufferToAttachment(vk::CommandBuffer cmdBuffer,
                                         pbr::GBuffer const& gBuffer) -> void {
  std::array const imageBarriers {
      ::getToColorAttachmentBarrier(gBuffer.getPositions().getImage()),
      ::getToColorAttachmentBarrier(gBuffer.getNormals().getImage()),
      ::getToColorAttachmentBarrier(gBuffer.getAlbedo().getImage()),
      ::getToDepthAttachmentBarrier(gBuffer.getDepth().getImage()),
  };
  cmdBuffer.pipelineBarrier2(vk::DependencyInfo {
      .dependencyFlags = vk::DependencyFlagBits::eByRegion,
  }
                                 .setImageMemoryBarriers(imageBarriers));
}
constexpr auto switchGBufferToSampled(vk::CommandBuffer cmdBuffer,
                                      pbr::GBuffer const& gBuffer) -> void {
  std::array const imageBarriers {
      ::getToColorReadBarrier(gBuffer.getPositions().getImage()),
      ::getToColorReadBarrier(gBuffer.getNormals().getImage()),
      ::getToColorReadBarrier(gBuffer.getAlbedo().getImage()),
      ::getToDepthReadBarrier(gBuffer.getDepth().getImage()),
  };
  cmdBuffer.pipelineBarrier2(vk::DependencyInfo {
      .dependencyFlags = vk::DependencyFlagBits::eByRegion,
  }
                                 .setImageMemoryBarriers(imageBarriers));
}
constexpr auto switchRenderTargetToAttachment(vk::CommandBuffer cmdBuffer,
                                              pbr::Image2D const& renderTarget) -> void {
  vk::ImageMemoryBarrier2 const imageBarrier {
      .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
      .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
      .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .image = renderTarget.getImage(),
      .subresourceRange {
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .levelCount = 1,
          .layerCount = 1,
      },
  };
  cmdBuffer.pipelineBarrier2(vk::DependencyInfo {
      .dependencyFlags = vk::DependencyFlagBits::eByRegion,
  }
                                 .setImageMemoryBarriers(imageBarrier));
}
} // namespace

pbr::PbrRenderSystem::PbrRenderSystem(core::SharedGpuHandle gpu,
                                      PbrRenderSystemCreateInfo info)
    : _gpu(std::move(gpu))
    , _sceneDescSetLayout(::createSceneDescriptorSetLayout(*_gpu))
    , _materialDescSetLayout(::createMaterialDescriptorSetLayout(*_gpu))
    , _gBufferSampler(::createGBufferSampler(*_gpu))
    , _depthSampler(::createDepthSampler(*_gpu))
    , _gBufferDescSetLayout(::createGBufferDescriptorSetLayout(
          *_gpu, _gBufferSampler.get(), _depthSampler.get()))
    , _gBufferDescriptorPool(
          ::createGBufferDescriptorPool(*_gpu, constants::MAX_G_BUFFER_DESCRIPTOR_SETS))
    , _geometryLayout(::createGeometryPipelineLayout(
          *_gpu, std::array {_sceneDescSetLayout.get(), _materialDescSetLayout.get()}))
    , _geometryPipeline()
    , _lightingLayout(::createLightingPipelineLayout(
          *_gpu, std::array {_sceneDescSetLayout.get(), _gBufferDescSetLayout.get()}))
    , _lightingPipeline() {
  auto geometryBuilder = ::buildGeometryPipeline(info);
  auto const geometryInfo = geometryBuilder.build(_geometryLayout.get());
  auto lightingBuilder = ::buildLightingPipeline(info);
  auto const lightingInfo = lightingBuilder.build(_lightingLayout.get());

  auto [result, pipelines] = _gpu->getDevice().createGraphicsPipelinesUnique(
      nullptr, std::array {geometryInfo, lightingInfo});
  assert(result == vk::Result::eSuccess);
  assert(pipelines.size() == 2);

  _geometryPipeline = std::move(pipelines.front());
  _lightingPipeline = std::move(pipelines.back());
}

auto pbr::PbrRenderSystem::allocateGBuffer(IAllocator& allocator, vk::Extent2D extent)
    -> GBuffer {
  return {
      *_gpu,
      allocator,
      std::move(
          _gpu->getDevice()
              .allocateDescriptorSetsUnique(
                  vk::DescriptorSetAllocateInfo {.descriptorPool = _gBufferDescriptorPool}
                      .setSetLayouts(_gBufferDescSetLayout.get()))
              .front()),
      extent,
  };
}

auto pbr::PbrRenderSystem::render(vk::CommandBuffer cmdBuffer, Scene const& scene,
                                  GBuffer const& gBuffer, Image2D const& renderTarget,
                                  vk::Extent2D renderExtent) -> void {
  ::switchGBufferToAttachment(cmdBuffer, gBuffer);
  recordGeometryPass(cmdBuffer, scene, gBuffer);
  ::switchGBufferToSampled(cmdBuffer, gBuffer);
  ::switchRenderTargetToAttachment(cmdBuffer, renderTarget);
  recordLightingPass(cmdBuffer, scene, gBuffer, renderTarget, {.extent = renderExtent});
}

auto pbr::PbrRenderSystem::recordGeometryPass(vk::CommandBuffer cmdBuffer,
                                              Scene const& scene, GBuffer const& gBuffer)
    -> void {
  std::array const attachments {
      vk::RenderingAttachmentInfo {
          .imageView = gBuffer.getPositions().getImageView(),
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue {
              .color {
                  .float32 = {{0.0f, 0.0f, 0.0f, 0.0f}},
              },
          },
      },
      vk::RenderingAttachmentInfo {
          .imageView = gBuffer.getNormals().getImageView(),
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue {
              .color {
                  .float32 = {{0.0f, 0.0f, 0.0f, 0.0f}},
              },
          },
      },
      vk::RenderingAttachmentInfo {
          .imageView = gBuffer.getAlbedo().getImageView(),
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue {
              .color {
                  .float32 = {{0.0f, 0.0f, 0.0f, 0.0f}},
              },
          },
      },
  };
  vk::RenderingAttachmentInfo const depthAttachment {
      .imageView = gBuffer.getDepth().getImageView(),
      .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue {
          .depthStencil {
              .depth = 1.0f,
          },
      },
  };
  cmdBuffer.beginRendering(vk::RenderingInfo {
      .renderArea {
          .extent = gBuffer.getExtent(),
      },
      .layerCount = 1,
      .pDepthAttachment = &depthAttachment,
  }
                               .setColorAttachments(attachments));

  cmdBuffer.setScissor(0, vk::Rect2D {.extent = gBuffer.getExtent()});
  cmdBuffer.setViewport(0, vk::Viewport {
                               .width = static_cast<float>(gBuffer.getExtent().width),
                               .height = static_cast<float>(gBuffer.getExtent().height),
                               .maxDepth = 1.0f,
                           });
  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _geometryPipeline.get());

  auto const camera = scene.findCamera();

  if (camera) {
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _geometryLayout.get(),
                                 0, camera.value()->getDescriptorSet(), {});
  }

  for (auto const& node : scene.iterateAllNodes()) {
    auto const& mesh = node.getMesh();
    if (mesh) {

      auto const transform = node.getTransform();
      auto const pushConst = pbr::makeModelPushConstant(
          transform.position, transform.rotation, transform.scale);
      cmdBuffer.pushConstants<ModelPushConstant>(
          _geometryLayout.get(), vk::ShaderStageFlagBits::eVertex, 0, pushConst);

      cmdBuffer.bindVertexBuffers(0, mesh->getVertexBuffer().getBuffer(), {0});
      cmdBuffer.bindIndexBuffer(mesh->getIndexBuffer().getBuffer(), 0,
                                vk::IndexType::eUint16);

      for (auto const& primitive : mesh->getPrimitives()) {
        cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     _geometryLayout.get(), 1,
                                     primitive.material->getDescriptorSet(), {});
        cmdBuffer.drawIndexed(primitive.indexCount, 1, primitive.firstIndex,
                              static_cast<std::int32_t>(primitive.firstVertex), 0);
      }
    }
  }

  cmdBuffer.endRendering();
}
auto pbr::PbrRenderSystem::recordLightingPass(vk::CommandBuffer cmdBuffer,
                                              Scene const& scene, GBuffer const& gBuffer,
                                              Image2D const& renderTo,
                                              vk::Rect2D renderArea) -> void {
  vk::RenderingAttachmentInfo const attachment {
      .imageView = renderTo.getImageView(),
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue {
          .color {
              .float32 = {{0.0f, 0.0f, 0.0f, 0.0f}},
          },
      },
  };
  cmdBuffer.beginRendering(vk::RenderingInfo {
      .renderArea = renderArea,
      .layerCount = 1,
  }
                               .setColorAttachments(attachment));

  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _lightingPipeline.get());

  auto const camera = scene.findCamera();

  if (camera) {
    std::array const descSets {camera.value()->getDescriptorSet(),
                               gBuffer.getDescriptorSet()};
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _lightingLayout.get(),
                                 0, descSets, {});

    cmdBuffer.draw(3, 1, 0, 0);

    cmdBuffer.endRendering();
  }
}
