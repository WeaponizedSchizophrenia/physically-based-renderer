#include "pbr/TransferStager.hpp"

#include "pbr/Buffer.hpp"
#include "pbr/Image.hpp"
#include "pbr/Vulkan.hpp"
#include "pbr/core/GpuHandle.hpp"
#include "pbr/memory/AllocationInfo.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

pbr::TransferStager::TransferStager(core::SharedGpuHandle gpu,
                                    std::shared_ptr<IAllocator> allocator)
    : _gpu(std::move(gpu)), _allocator(std::move(allocator)), _submitter(_gpu) {}

auto pbr::TransferStager::addTransfer(std::span<std::byte const> const data,
                                      vk::BufferUsageFlags const bufferUsage)
    -> BufferHandle {
  Buffer buffer = _allocator->allocateBuffer(
      {
          .size = data.size_bytes(),
          .usage = bufferUsage | vk::BufferUsageFlagBits::eTransferDst,
      },
      {});
  BufferHandle const handle(_bufferTransfers.size(), buffer.getBuffer());

  _bufferTransfers.emplace_back(data, std::move(buffer));

  return handle;
}

auto pbr::TransferStager::addTransfer(std::span<std::byte const> const data,
                                      vk::ImageCreateInfo imageInfo,
                                      vk::ImageAspectFlags const aspectMask,
                                      vk::PipelineStageFlags2 const dstStage,
                                      vk::AccessFlags2 const dstAccess) -> ImageHandle {
  imageInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
  Image image = _allocator->allocateImage(imageInfo, {});
  ImageHandle const handle(_imageTransfers.size(), image.getImage());

  _imageTransfers.emplace_back(data, std::move(image), aspectMask, imageInfo.extent,
                               dstStage, dstAccess);

  return handle;
}

auto pbr::TransferStager::submit(vk::CommandPool const cmdPool) -> void {
  auto const totalBuffersSize = std::ranges::fold_left(
      _bufferTransfers, 0uz, [](std::size_t acc, BufferTransfer const& transfer) {
        return acc + transfer.data.size_bytes();
      });
  auto const totalImagesSize = std::ranges::fold_left(
      _imageTransfers, 0uz, [](std::size_t acc, ImageTransfer const& transfer) {
        return acc + transfer.data.size_bytes();
      });

  _stagingBuffer.emplace(_allocator->allocateBuffer(
      {
          .size = totalBuffersSize + totalImagesSize,
          .usage = vk::BufferUsageFlagBits::eTransferSrc,
      },
      {
          .preference = AllocationPreference::Host,
          .priority = AllocationPriority::Time,
          .ableToBeMapped = true,
      }));

  {
    auto const dataGetter = [](auto const& transfer) { return transfer.data; };
    auto const allBufferData = _bufferTransfers | std::views::transform(dataGetter)
                               | std::views::join | std::ranges::to<std::vector>();
    auto const allImageData = _imageTransfers | std::views::transform(dataGetter)
                              | std::views::join | std::ranges::to<std::vector>();

    auto const mapping = _stagingBuffer->map();
    std::memcpy(mapping.get(), allBufferData.data(), allBufferData.size());
    std::memcpy(std::next(static_cast<std::byte*>(mapping.get()),
                          static_cast<std::ptrdiff_t>(allBufferData.size())),
                allImageData.data(), allImageData.size());
  }

  auto cmdBuffer = std::move(
      _gpu->getDevice()
          .allocateCommandBuffersUnique({.commandPool = cmdPool, .commandBufferCount = 1})
          .front());

  cmdBuffer->begin(vk::CommandBufferBeginInfo {
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  if (!_imageTransfers.empty()) {
    auto const imageMemoryBarriers =
        _imageTransfers | std::views::transform([](ImageTransfer const& transfer) {
          return vk::ImageMemoryBarrier2 {
              .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
              .srcAccessMask = vk::AccessFlagBits2::eNone,
              .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
              .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
              .oldLayout = vk::ImageLayout::eUndefined,
              .newLayout = vk::ImageLayout::eTransferDstOptimal,
              .image = transfer.image.getImage(),
              .subresourceRange {
                  .aspectMask = transfer.aspectMask,
                  .levelCount = 1,
                  .layerCount = 1,
              },
          };
        })
        | std::ranges::to<std::vector>();
    cmdBuffer->pipelineBarrier2(
        vk::DependencyInfo {.dependencyFlags = vk::DependencyFlagBits::eByRegion}
            .setImageMemoryBarriers(imageMemoryBarriers));
  }

  vk::DeviceSize stagingBufferOffset {};
  for (auto const& transfer : _bufferTransfers) {
    cmdBuffer->copyBuffer(_stagingBuffer->getBuffer(), transfer.buffer.getBuffer(),
                          vk::BufferCopy {
                              .srcOffset = stagingBufferOffset,
                              .size = transfer.data.size_bytes(),
                          });
    stagingBufferOffset += transfer.data.size_bytes();
  }
  for (auto const& transfer : _imageTransfers) {
    cmdBuffer->copyBufferToImage(_stagingBuffer->getBuffer(), transfer.image.getImage(),
                                 vk::ImageLayout::eTransferDstOptimal,
                                 vk::BufferImageCopy {
                                     .bufferOffset = stagingBufferOffset,
                                     .imageSubresource {
                                         .aspectMask = transfer.aspectMask,
                                         .layerCount = 1,
                                     },
                                     .imageExtent = transfer.extent,
                                 });
    stagingBufferOffset += transfer.data.size_bytes();
  }

  if (!_imageTransfers.empty()) {
    auto const imageMemoryBarriers =
        _imageTransfers | std::views::transform([](ImageTransfer const& transfer) {
          return vk::ImageMemoryBarrier2 {
              .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
              .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
              .dstStageMask = transfer.dstStage,
              .dstAccessMask = transfer.dstAccess,
              .oldLayout = vk::ImageLayout::eTransferDstOptimal,
              .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
              .image = transfer.image.getImage(),
              .subresourceRange {
                  .aspectMask = transfer.aspectMask,
                  .levelCount = 1,
                  .layerCount = 1,
              },
          };
        })
        | std::ranges::to<std::vector>();
    cmdBuffer->pipelineBarrier2(
        vk::DependencyInfo {.dependencyFlags = vk::DependencyFlagBits::eByRegion}
            .setImageMemoryBarriers(imageMemoryBarriers));
  }

  cmdBuffer->end();

  _submitter.submit({.cmdBuffer = std::move(cmdBuffer)});
}

auto pbr::TransferStager::wait() -> void {
  _submitter.wait();
  _stagingBuffer.reset();
}

auto pbr::TransferStager::get(BufferHandle const handle) -> Buffer {
  assert(!_submitter.isExecuting());
  return std::move(_bufferTransfers.at(handle.getIndex()).buffer);
}

auto pbr::TransferStager::get(ImageHandle const handle) -> Image {
  assert(!_submitter.isExecuting());
  return std::move(_imageTransfers.at(handle.getIndex()).image);
}