#include <catch2/catch_test_macros.hpp>

#include "pbr/Buffer.hpp"
#include "pbr/TransferStager.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/utils/Conversions.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/AsyncSubmitter.hpp"
#include "pbr/Surface.hpp"
#include "pbr/memory/AllocationInfo.hpp"
#include "pbr/memory/MemoryAllocator.hpp"

#include "vkfw/vkfw.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <memory>
#include <span>
#include <utility>

TEST_CASE("Surface creation", "[pbr]") {
  [[maybe_unused]]
  auto const vkfw = vkfw::initUnique({.platform = vkfw::Platform::eX11});
  auto const window = vkfw::createWindowUnique(1, 1, "");

  auto const gpu = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });

  pbr::Surface const surface(
      gpu, vkfw::createWindowSurfaceUnique(gpu->getInstance(), window.get()),
      pbr::utils::toExtent(window->getFramebufferSize()));
}

TEST_CASE("Async submit", "[pbr]") {
  [[maybe_unused]]
  auto const vkfw = vkfw::initUnique({.platform = vkfw::Platform::eX11});

  auto const gpu = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });

  auto const commandPool = gpu->getDevice().createCommandPoolUnique({
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = gpu->getPhysicalDeviceProperties().graphicsTransferPresentQueue,
  });
  auto cmdBuffers = gpu->getDevice().allocateCommandBuffersUnique({
      .commandPool = commandPool.get(),
      .commandBufferCount = 1,
  });
  auto const cmdBuffer = cmdBuffers.front().get();
  cmdBuffer.begin(vk::CommandBufferBeginInfo {});
  cmdBuffer.end();

  pbr::AsyncSubmitter submitter(gpu);
  pbr::AsyncSubmitInfo submitInfo {.cmdBuffer = std::move(cmdBuffers.front())};
  submitter.submit(std::move(submitInfo));
  REQUIRE(submitter.isSubmitted());

  submitInfo = submitter.wait();

  REQUIRE(submitInfo.cmdBuffer.get() == cmdBuffer);
}

TEST_CASE("Engine tests", "[pbr]") {
  [[maybe_unused]]
  auto const vkfw = vkfw::initUnique({.platform = vkfw::Platform::eX11});
  auto const window = vkfw::createWindowUnique(1, 1, "");

  auto const gpu = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });

  pbr::Surface surface(gpu,
                       vkfw::createWindowSurfaceUnique(gpu->getInstance(), window.get()),
                       pbr::utils::toExtent(window->getFramebufferSize()));

  auto const commandPool = gpu->getDevice().createCommandPoolUnique({
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = gpu->getPhysicalDeviceProperties().graphicsTransferPresentQueue,
  });

  auto const allocator = std::make_shared<pbr::MemoryAllocator>(gpu);

  SECTION("Clear and present surface") {
    auto imageSemaphore = gpu->getDevice().createSemaphoreUnique({});
    auto clearSemaphore = gpu->getDevice().createSemaphoreUnique({});
    auto const clearSemaphoreHandle = clearSemaphore.get();

    auto imageView = surface.acquireSwapchainImageView(imageSemaphore.get());
    REQUIRE(imageView.has_value());
    auto cmdBuffer =
        std::move(gpu->getDevice()
                      .allocateCommandBuffersUnique(
                          {.commandPool = commandPool.get(), .commandBufferCount = 1})
                      .front());

    cmdBuffer->begin(vk::CommandBufferBeginInfo {});

    { // Rendering
      cmdBuffer->pipelineBarrier(
          vk::PipelineStageFlagBits::eTopOfPipe,
          vk::PipelineStageFlagBits::eColorAttachmentOutput,
          vk::DependencyFlagBits::eByRegion, {}, {},
          vk::ImageMemoryBarrier {
              .srcAccessMask = vk::AccessFlagBits::eNone,
              .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
              .oldLayout = vk::ImageLayout::eUndefined,
              .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
              .image = imageView->getImage(),
              .subresourceRange {
                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                  .levelCount = 1,
                  .layerCount = 1,
              },
          });

      vk::RenderingAttachmentInfo const attachmentInfo {
          .imageView = imageView->getImageView(),
          .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .loadOp = vk::AttachmentLoadOp::eClear,
          .storeOp = vk::AttachmentStoreOp::eStore,
          .clearValue = vk::ClearColorValue {}.setFloat32({1.0f, 1.0f, 1.0f, 1.0f}),
      };
      auto const renderInfo =
          vk::RenderingInfo {
              .renderArea {
                  .extent = imageView->getExtent(),
              },
              .layerCount = 1,
          }
              .setColorAttachments(attachmentInfo);

      cmdBuffer->beginRendering(renderInfo);
      cmdBuffer->endRendering();

      cmdBuffer->pipelineBarrier(
          vk::PipelineStageFlagBits::eColorAttachmentOutput,
          vk::PipelineStageFlagBits::eBottomOfPipe, vk::DependencyFlagBits::eByRegion, {},
          {},
          vk::ImageMemoryBarrier {
              .srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
              .dstAccessMask = vk::AccessFlagBits::eNone,
              .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
              .newLayout = vk::ImageLayout::ePresentSrcKHR,
              .image = imageView->getImage(),
              .subresourceRange {
                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                  .levelCount = 1,
                  .layerCount = 1,
              },
          });
    }

    cmdBuffer->end();

    pbr::AsyncSubmitter submitter(gpu);

    submitter.submit({
        .waitSemaphore =
            pbr::AsyncSubmitInfo::WaitSemaphore {
                .semaphore = std::move(imageSemaphore),
                .waitDstStageMask = vk::PipelineStageFlagBits::eTransfer,
            },
        .signalSemaphore = std::move(clearSemaphore),
        .cmdBuffer = std::move(cmdBuffer),
    });

    imageView->present(clearSemaphoreHandle);

    gpu->getQueue().waitIdle();
  }

  SECTION("Memory allocator") {
    std::array<std::byte, 64> const data {};
    pbr::Buffer const buffer = allocator->allocateBuffer(
        vk::BufferCreateInfo {
            .size = data.size(),
            .usage = vk::BufferUsageFlagBits::eStorageBuffer,
        },
        pbr::AllocationInfo {
            .preference = pbr::AllocationPreference::Host,
            .ableToBeMapped = true,
            .persistentlyMapped = true,
        });
    {
      auto mapping = buffer.map();
      std::memcpy(mapping.get(), data.data(), data.size());
    }
    {
      auto mapping = buffer.map();
      std::span<std::byte const> const mappedSpan(static_cast<std::byte*>(mapping.get()),
                                                  data.size());

      REQUIRE(std::ranges::equal(data, mappedSpan));
    }
  }

  SECTION("Transfer stager") {
    std::vector<std::byte> const bufferData(64);
    std::vector<std::byte> const imageData(15uz * 15);

    pbr::TransferStager stager(gpu, allocator);

    auto const buffer=
        stager.addTransfer(bufferData, vk::BufferUsageFlagBits::eStorageBuffer);
    auto const image= stager.addTransfer(
        imageData,
        {
            .imageType = vk::ImageType::e2D,
            .format = vk::Format::eR8Unorm,
            .extent {
                .width = 15,
                .height = 15,
                .depth = 1,
            },
            .mipLevels = 1,
            .arrayLayers = 1,
            .usage = vk::ImageUsageFlagBits::eSampled,
        },
        vk::ImageAspectFlagBits::eColor, vk::PipelineStageFlagBits2::eFragmentShader,
        vk::AccessFlagBits2::eShaderRead);

    stager.submit(commandPool.get());
    stager.wait();
  }
}
