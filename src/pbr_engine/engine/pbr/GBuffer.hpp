#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/memory/IAllocator.hpp"

namespace pbr {
class GBuffer {
public:
  static constexpr auto POSITIONS_FORMAT = vk::Format::eR16G16B16A16Sfloat;
  static constexpr auto NORMALS_FORMAT = vk::Format::eR16G16B16A16Sfloat;
  static constexpr auto ALBEDO_FORMAT = vk::Format::eR16G16B16A16Unorm;
  static constexpr auto DEPTH_FORMAT = vk::Format::eD32Sfloat;

private:
  Image2D _positions;
  Image2D _normals;
  Image2D _albedo;
  Image2D _depth;
  vk::Extent2D _extent;
  vk::UniqueDescriptorSet _descSet;

public:
  GBuffer(core::GpuHandle const& gpu, IAllocator& allocator, vk::UniqueDescriptorSet descSet,
                   vk::Extent2D extent);

  [[nodiscard]]
  constexpr auto getPositions() const noexcept -> Image2D const&;
  [[nodiscard]]
  constexpr auto getNormals() const noexcept -> Image2D const&;
  [[nodiscard]]
  constexpr auto getAlbedo() const noexcept -> Image2D const&;
  [[nodiscard]]
  constexpr auto getDepth() const noexcept -> Image2D const&;
  [[nodiscard]]
  constexpr auto getExtent() const noexcept -> vk::Extent2D;
  [[nodiscard]]
  constexpr auto getDescriptorSet() const noexcept -> vk::DescriptorSet;
};
} // namespace pbr

/* IMPLEMENTATIONS */
constexpr auto pbr::GBuffer::getPositions() const noexcept -> Image2D const& {
  return _positions;
}
constexpr auto pbr::GBuffer::getNormals() const noexcept -> Image2D const& {
  return _normals;
}
constexpr auto pbr::GBuffer::getAlbedo() const noexcept -> Image2D const& {
  return _albedo;
}
constexpr auto pbr::GBuffer::getDepth() const noexcept -> Image2D const& {
  return _depth;
}
constexpr auto pbr::GBuffer::getExtent() const noexcept -> vk::Extent2D {
  return _extent;
}
constexpr auto pbr::GBuffer::getDescriptorSet() const noexcept -> vk::DescriptorSet {
  return _descSet.get();
}
