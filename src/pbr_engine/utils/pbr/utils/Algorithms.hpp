#pragma once

#include "pbr/Vulkan.hpp"
#include <algorithm>

namespace pbr::utils {
constexpr auto clamp(vk::Extent2D value, vk::Extent2D min,
                     vk::Extent2D max) noexcept -> vk::Extent2D;
}

// the parameters are the but that is fine becuase i have never seen a
// clamp function take parameters in a different order NOLINTNEXTLINE
constexpr auto pbr::utils::clamp(vk::Extent2D value, vk::Extent2D min,
                                 vk::Extent2D max) noexcept -> vk::Extent2D {
  return {
      .width = std::clamp(value.width, min.width, max.width),
      .height = std::clamp(value.height, min.height, max.height),
  };
}
