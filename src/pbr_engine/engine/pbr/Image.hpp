#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/memory/Allocation.hpp"

#include <utility>

namespace pbr {
class Image {
  Allocation _allocation;
  vk::UniqueImage _buffer;

public:
  constexpr Image(std::pair<Allocation, vk::UniqueImage> allocatedPair) noexcept;
  constexpr Image(Allocation allocation, vk::UniqueImage buffer) noexcept;

  [[nodiscard]]
  constexpr auto getImage() const noexcept -> vk::Image;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr pbr::Image::Image(std::pair<Allocation, vk::UniqueImage> allocatedPair) noexcept
    : _allocation(std::move(allocatedPair.first)), _buffer(std::move(allocatedPair.second)) {}

constexpr pbr::Image::Image(Allocation allocation, vk::UniqueImage buffer) noexcept
    : _allocation(std::move(allocation)), _buffer(std::move(buffer)) {}

constexpr auto pbr::Image::getImage() const noexcept -> vk::Image {
  return _buffer.get();
}
