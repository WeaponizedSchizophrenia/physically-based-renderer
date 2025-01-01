#include "pbr/imgui/Renderer.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image.hpp"
#include "pbr/TransferStager.hpp"
#include "pbr/imgui/Pipeline.hpp"
#include "pbr/memory/IAllocator.hpp"

#include "imgui.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace {
[[nodiscard]]
constexpr auto createFontImage(pbr::core::SharedGpuHandle gpu,
                               std::shared_ptr<pbr::IAllocator> allocator,
                               vk::CommandPool cmdPool) -> pbr::Image {
  // TODO: Make this async
  ImGuiIO const& imguiIo = ImGui::GetIO();
  std::uint8_t* fontPixels {};
  int width {};
  int height {};
  imguiIo.Fonts->GetTexDataAsRGBA32(&fontPixels, &width, &height);
  std::vector<std::byte> fontImage(static_cast<std::size_t>(width) * height * 4);
  std::memcpy(fontImage.data(), fontPixels, fontImage.size());

  pbr::TransferStager stager(std::move(gpu), std::move(allocator));
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
  return stager.get(imageHandle);
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
constexpr auto
createDescriptorSet(pbr::core::GpuHandle const& gpu, pbr::imgui::Pipeline const& pipeline,
                    vk::DescriptorPool descPool) -> vk::UniqueDescriptorSet {
  auto const layout = pipeline.getFontSamplerLayout();
  return std::move(gpu.getDevice()
                       .allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo {
                           .descriptorPool = descPool,
                       }
                                                         .setSetLayouts(layout))
                       .front());
}
} // namespace

pbr::imgui::Renderer::Renderer(core::SharedGpuHandle gpu,
                               std::shared_ptr<IAllocator> allocator,
                               vk::CommandPool cmdPool, PipelineCreateInfo info)
    : _gpu(std::move(gpu))
    , _fontImage(::createFontImage(_gpu, std::move(allocator), cmdPool))
    , _fontImageView(_gpu->getDevice().createImageViewUnique({
          .image = _fontImage.getImage(),
          .viewType = vk::ImageViewType::e2D,
          .format = vk::Format::eR8G8B8A8Unorm,
          .subresourceRange {
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .levelCount = 1,
              .layerCount = 1,
          },
      }))
    , _fontSampler(_gpu->getDevice().createSamplerUnique({
          .magFilter = vk::Filter::eLinear,
          .minFilter = vk::Filter::eLinear,
          .addressModeU = vk::SamplerAddressMode::eClampToEdge,
          .addressModeV = vk::SamplerAddressMode::eClampToEdge,
          .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      }))
    , _pipeline(*_gpu, info)
    , _descPool(::createDescriptorPool(*_gpu))
    , _descSet(::createDescriptorSet(*_gpu, _pipeline, _descPool.get())) {}
