#pragma once

#include "pbr/Vulkan.hpp"

#include <cstdint>
#include <functional>
#include <span>

namespace pbr::core {
/**
 * Contains dependencies and settings for a GpuHandle.
 */
struct GpuHandleCreateInfo {
  using QueueFamilyPresentPredicate =
      std::function<bool(vk::Instance, vk::PhysicalDevice, std::uint32_t)>;

  /// The required instance extensions.
  std::span<char const* const> extensions {};
  /// The predicate that returns whether the passed in physical device has support for
  /// presentation for the platform.
  /// @note This predicate is REQUIRED.
  QueueFamilyPresentPredicate presentPredicate {};
  /// Value indicating whether validation layers should be enabled for the instance or
  /// not.
  bool enableValidation {};
};
} // namespace pbr::core
