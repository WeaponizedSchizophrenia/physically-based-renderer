#pragma once

#include "pbr/AsyncSubmitter.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Buffer.hpp"
#include "pbr/Image.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace pbr {
class TransferStager {
  struct BufferTransfer {
    std::span<std::byte const> data;
    Buffer buffer;
  };
  struct ImageTransfer {
    std::span<std::byte const> data;
    Image image;
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
  template <typename T> class StagerHandle {
    std::size_t _index;
    T _resource;

  public:
    constexpr StagerHandle(std::size_t index, T resource) noexcept;

    [[nodiscard]]
    constexpr auto getIndex() const noexcept -> std::size_t;
    [[nodiscard]]
    constexpr auto getVkHandle() const noexcept -> T;
  };
  using BufferHandle = StagerHandle<vk::Buffer>;
  using ImageHandle = StagerHandle<vk::Image>;

  TransferStager(core::SharedGpuHandle gpu, std::shared_ptr<IAllocator> allocator);

  [[nodiscard]]
  auto addTransfer(std::span<std::byte const> data,
                   vk::BufferUsageFlags bufferUsage) -> BufferHandle;
  [[nodiscard]]
  auto addTransfer(std::span<std::byte const> data, vk::ImageCreateInfo imageInfo,
                   vk::ImageAspectFlags aspectMask, vk::PipelineStageFlags2 dstStage,
                   vk::AccessFlags2 dstAccess) -> ImageHandle;

  auto submit(vk::CommandPool cmdPool) -> void;
  auto wait() -> void;

  [[nodiscard]]
  auto get(BufferHandle handle) -> Buffer;
  [[nodiscard]]
  auto get(ImageHandle handle) -> Image;
};
} // namespace pbr

/* IMPLEMENTATION */

template <typename T>
constexpr pbr::TransferStager::StagerHandle<T>::StagerHandle(std::size_t index,
                                                             T resource) noexcept
    : _index(index), _resource(resource) {}

template <typename T>
constexpr auto
pbr::TransferStager::StagerHandle<T>::getIndex() const noexcept -> std::size_t {
  return _index;
}

template <typename T>
constexpr auto pbr::TransferStager::StagerHandle<T>::getVkHandle() const noexcept -> T {
  return _resource;
}
