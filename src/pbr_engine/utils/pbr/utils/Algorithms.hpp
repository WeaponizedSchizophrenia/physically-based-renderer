#pragma once

#include "pbr/Vulkan.hpp"
#include <algorithm>

namespace pbr::utils {
/**
 * Clamps an vk::Extent2D to a min and a max value.
 *
 * @param value The value that gets clamped.
 * @param min The minimum value.
 * @param max The maximum value.
 */
constexpr auto clamp(vk::Extent2D value, vk::Extent2D min,
                     vk::Extent2D max) noexcept -> vk::Extent2D;
} // namespace pbr::utils

/* IMPLEMENTATIONS */

// the parameters are the same type but that is fine becuase i have never seen a
// clamp function take parameters in a different order NOLINTNEXTLINE
constexpr auto pbr::utils::clamp(vk::Extent2D value, vk::Extent2D min,
                                 vk::Extent2D max) noexcept -> vk::Extent2D {
  return {
      .width = std::clamp(value.width, min.width, max.width),
      .height = std::clamp(value.height, min.height, max.height),
  };
}
