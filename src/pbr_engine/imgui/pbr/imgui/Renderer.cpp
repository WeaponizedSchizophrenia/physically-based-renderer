#include "pbr/imgui/Renderer.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Buffer.hpp"
#include "pbr/Image2D.hpp"
#include "pbr/TransferStager.hpp"
#include "pbr/imgui/Pipeline.hpp"
#include "pbr/imgui/PushConstant.hpp"
#include "pbr/memory/AllocationInfo.hpp"
#include "pbr/memory/IAllocator.hpp"

#include "imgui.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <print>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

namespace {
[[nodiscard]]
constexpr auto createFontImage(pbr::core::SharedGpuHandle const& gpu,
                               std::shared_ptr<pbr::IAllocator> allocator,
                               vk::CommandPool cmdPool) -> pbr::Image2D {
  // TODO: Make this async
  ImGuiIO const& imguiIo = ImGui::GetIO();
  std::uint8_t* fontPixels {};
  int width {};
  int height {};
  imguiIo.Fonts->GetTexDataAsRGBA32(&fontPixels, &width, &height);
  std::vector<std::byte> fontImage(static_cast<std::size_t>(width) * height * 4);
  std::memcpy(fontImage.data(), fontPixels, fontImage.size());

  pbr::TransferStager stager(gpu, std::move(allocator));
  auto imageHandle = stager.addTransfer(
      std::move(fontImage),
      vk::ImageCreateInfo {
          .imageType = vk::ImageType::e2D,
          .format = vk::Format::eR8G8B8A8Unorm,
          .extent {
              .width = static_cast<std::uint32_t>(width),
              .height = static_cast<std::uint32_t>(height),
              .depth = 1,
          },
          .mipLevels = 1,
          .arrayLayers = 1,
          .usage = vk::ImageUsageFlagBits::eSampled,
      },
      vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits2::eFragmentShader,
      vk::AccessFlagBits2::eShaderRead);
  stager.submit(cmdPool);
  stager.wait();
  return {
    *gpu,
    vk::Format::eR8G8B8A8Unorm,
    vk::ImageAspectFlagBits::eColor,
    stager.get(imageHandle),
  };
}
[[nodiscard]]
constexpr auto
createDescriptorPool(pbr::core::GpuHandle const& gpu) -> vk::UniqueDescriptorPool {
  vk::DescriptorPoolSize const size {
      .type = vk::DescriptorType::eCombinedImageSampler,
      .descriptorCount = 1,
  };
  return gpu.getDevice().createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo {
      .maxSets = 1,
  }
                                                        .setPoolSizes(size));
}
[[nodiscard]]
constexpr auto createDescriptorSet(pbr::core::GpuHandle const& gpu,
                                   pbr::imgui::Pipeline const& pipeline,
                                   vk::DescriptorPool descPool) -> vk::DescriptorSet {
  auto const layout = pipeline.getFontSamplerLayout();
  return gpu.getDevice()
      .allocateDescriptorSets(vk::DescriptorSetAllocateInfo {
          .descriptorPool = descPool,
      }
                                  .setSetLayouts(layout))
      .front();
}
} // namespace

pbr::imgui::Renderer::Renderer(core::SharedGpuHandle gpu,
                               std::shared_ptr<IAllocator> allocator,
                               vk::CommandPool const cmdPool,
                               PipelineCreateInfo const info)
    : _gpu(std::move(gpu))
    , _allocator(std::move(allocator))
    , _fontImage(::createFontImage(_gpu, _allocator, cmdPool))
    , _fontSampler(_gpu->getDevice().createSamplerUnique({
          .magFilter = vk::Filter::eLinear,
          .minFilter = vk::Filter::eLinear,
          .addressModeU = vk::SamplerAddressMode::eClampToEdge,
          .addressModeV = vk::SamplerAddressMode::eClampToEdge,
          .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      }))
    , _pipeline(*_gpu, info)
    , _descPool(::createDescriptorPool(*_gpu))
    , _descSet(::createDescriptorSet(*_gpu, _pipeline, _descPool.get())) {
  vk::DescriptorImageInfo const imageInfo {
      .sampler = _fontSampler.get(),
      .imageView = _fontImage.getImageView(),
      .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
  };
  _gpu->getDevice().updateDescriptorSets(
      vk::WriteDescriptorSet {
          .dstSet = _descSet,
          .descriptorType = vk::DescriptorType::eCombinedImageSampler,
      }
          .setImageInfo(imageInfo),
      {});
}

auto pbr::imgui::Renderer::render(vk::CommandBuffer cmdBuffer) -> void {
  auto const& imguiIo = ImGui::GetIO();
  auto const* const drawData = ImGui::GetDrawData();

  if (drawData->CmdListsCount == 0) {
    return;
  }

  updateBuffers(drawData);

  cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline.getPipeline());
  cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                               _pipeline.getPipelineLayout(), 0, _descSet, {});
  PushConstant const pushConstant {
      .scale {2.0f / imguiIo.DisplaySize.x, 2.0f / imguiIo.DisplaySize.y},
      .translate {-1.0f},
  };
  cmdBuffer.pushConstants<PushConstant>(
      _pipeline.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, pushConstant);

  std::int32_t vertexOffset {};
  std::uint32_t indexOffset {};

  cmdBuffer.bindVertexBuffers(0, _vertexBuffer->buffer.getBuffer(), {0});
  static_assert(sizeof(ImDrawIdx) == sizeof(std::uint16_t),
                "Imgui index size is unsuported");
  cmdBuffer.bindIndexBuffer(_indexBuffer->buffer.getBuffer(), 0, vk::IndexType::eUint16);

  for (auto const* const cmdList : drawData->CmdLists) {
    for (auto cmd : std::span(cmdList->CmdBuffer.Data, cmdList->CmdBuffer.Size)) {
      cmdBuffer.setScissor(
          0,
          vk::Rect2D {
              .offset {.x = std::max(static_cast<std::int32_t>(cmd.ClipRect.x),
                                     std::int32_t(0)),
                       .y = std::max(static_cast<std::int32_t>(cmd.ClipRect.y),
                                     std::int32_t(0))},
              .extent {
                  .width = static_cast<std::uint32_t>(cmd.ClipRect.z - cmd.ClipRect.x),
                  .height = static_cast<std::uint32_t>(cmd.ClipRect.w - cmd.ClipRect.y),
              },
          });
      cmdBuffer.drawIndexed(cmd.ElemCount, 1, indexOffset, vertexOffset, 0);
      indexOffset += cmd.ElemCount;
    }
    vertexOffset += cmdList->VtxBuffer.Size;
  }
}

auto pbr::imgui::Renderer::updateBuffers(ImDrawData const* const data) -> void {
  AllocationInfo const allocInfo {
      .preference = AllocationPreference::Host,
      .priority = AllocationPriority::Time,
      .ableToBeMapped = true,
  };
  auto const vbSize =
      static_cast<vk::DeviceSize>(data->TotalVtxCount * sizeof(ImDrawVert));
  Buffer vertexBuffer = _allocator->allocateBuffer(
      {
          .size = vbSize,
          .usage = vk::BufferUsageFlagBits::eVertexBuffer,
      },
      allocInfo);
  auto const ibSize =
      static_cast<vk::DeviceSize>(data->TotalIdxCount * sizeof(ImDrawIdx));
  Buffer indexBuffer = _allocator->allocateBuffer(
      {
          .size = ibSize,
          .usage = vk::BufferUsageFlagBits::eIndexBuffer,
      },
      allocInfo);

  { // copy vertices
    auto const vertices =
        data->CmdLists | std::views::transform([](ImDrawList const* const list) {
          return std::span(list->VtxBuffer.Data, list->VtxBuffer.Size);
        })
        | std::views::join | std::ranges::to<std::vector>();
    auto const vbMapping = vertexBuffer.map();
    std::memcpy(vbMapping.get(), vertices.data(), vbSize);
  }
  { // copy indices
    auto const indices = data->CmdLists
                         | std::views::transform([](ImDrawList const* const list) {
                             return std::span(list->IdxBuffer.Data, list->IdxBuffer.Size);
                           })
                         | std::views::join | std::ranges::to<std::vector>();
    auto const ibMapping = indexBuffer.map();
    std::memcpy(ibMapping.get(), indices.data(), ibSize);
  }

  _vertexBuffer.emplace(std::move(vertexBuffer), vbSize);
  _indexBuffer.emplace(std::move(indexBuffer), ibSize);
}
