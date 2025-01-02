#pragma once

#include "pbr/Vulkan.hpp"

#include <concepts>
#include <cstdint>
#include <tuple>

namespace pbr::utils {
/**
 * Converts a tuple of some integral type to a vk::Extent2D.
 */
template <std::integral T>
[[nodiscard]]
constexpr auto toExtent(std::tuple<T, T> tuple) noexcept -> vk::Extent2D;
/**
 * Converts two values of some integral type to a vk::Extent2D.
 */
template <std::integral T>
[[nodiscard]]
constexpr auto toExtent(T width, T height) noexcept -> vk::Extent2D;
} // namespace pbr::utils

/* IMPLEMENTATIONS */

template <std::integral T>
constexpr auto pbr::utils::toExtent(std::tuple<T, T> tuple) noexcept -> vk::Extent2D {
  return {
      .width = static_cast<std::uint32_t>(std::get<0>(tuple)),
      .height = static_cast<std::uint32_t>(std::get<1>(tuple)),
  };
}

template <std::integral T>
constexpr auto pbr::utils::toExtent(T width, T height) noexcept -> vk::Extent2D {
  return {
      .width = static_cast<std::uint32_t>(width),
      .height = static_cast<std::uint32_t>(height),
  };
}
