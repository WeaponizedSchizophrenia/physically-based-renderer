#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/AsyncSubmitter.hpp"
#include "pbr/Buffer.hpp"
#include "pbr/Image.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace pbr {
class TransferStager {
  struct BufferTransfer {
    std::vector<std::byte> data;
    vk::Buffer buffer;
  };
  struct ImageTransfer {
    std::vector<std::byte> data;
    vk::Image image;
    vk::ImageAspectFlags aspectMask;
    vk::Extent3D extent;
    vk::PipelineStageFlags2 dstStage;
    vk::AccessFlags2 dstAccess;
  };

  core::SharedGpuHandle _gpu;
  std::shared_ptr<IAllocator> _allocator;
  std::vector<BufferTransfer> _bufferTransfers {};
  std::vector<ImageTransfer> _imageTransfers {};
  std::optional<Buffer> _stagingBuffer = std::nullopt;
  AsyncSubmitter _submitter;

public:
  TransferStager(core::SharedGpuHandle gpu, std::shared_ptr<IAllocator> allocator);

  [[nodiscard]]
  auto addTransfer(std::vector<std::byte> data,
                   vk::BufferUsageFlags bufferUsage) -> Buffer;
  [[nodiscard]]
  auto addTransfer(std::vector<std::byte> data, vk::ImageCreateInfo imageInfo,
                   vk::ImageAspectFlags aspectMask, vk::PipelineStageFlags2 dstStage,
                   vk::AccessFlags2 dstAccess) -> Image;

  auto submit(vk::CommandPool cmdPool) -> void;
  auto wait() -> void;
};
} // namespace pbr
