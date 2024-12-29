#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/memory/Allocation.hpp"
#include <utility>

namespace pbr {
class Buffer {
  Allocation _allocation;
  vk::UniqueBuffer _buffer;

public:
  constexpr Buffer(std::pair<Allocation, vk::UniqueBuffer> allocatedPair) noexcept;
  constexpr Buffer(Allocation allocation, vk::UniqueBuffer buffer) noexcept;

  [[nodiscard]]
  constexpr auto map() const -> Allocation::Mapping;

  [[nodiscard]]
  constexpr auto getBuffer() const noexcept -> vk::Buffer;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr pbr::Buffer::Buffer(std::pair<Allocation, vk::UniqueBuffer> allocatedPair) noexcept
    : _allocation(std::move(allocatedPair.first)), _buffer(std::move(allocatedPair.second)) {}

constexpr pbr::Buffer::Buffer(Allocation allocation, vk::UniqueBuffer buffer) noexcept
    : _allocation(std::move(allocation)), _buffer(std::move(buffer)) {}

constexpr auto pbr::Buffer::map() const -> Allocation::Mapping {
  return _allocation.map();
}

constexpr auto pbr::Buffer::getBuffer() const noexcept -> vk::Buffer {
  return _buffer.get();
}
