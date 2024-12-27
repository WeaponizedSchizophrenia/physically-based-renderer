#pragma once

#include "pbr/Vulkan.hpp"

#include <concepts>
#include <cstdint>
#include <tuple>

namespace pbr::utils {
template<std::integral T>
[[nodiscard]]
constexpr auto toExtent(std::tuple<T, T> tuple) noexcept -> vk::Extent2D;
}

/* IMPLEMENTATIONS */

template<std::integral T>
constexpr auto pbr::utils::toExtent(std::tuple<T, T> tuple) noexcept -> vk::Extent2D {
  return {
    .width = static_cast<std::uint32_t>(std::get<0>(tuple)),
    .height = static_cast<std::uint32_t>(std::get<1>(tuple)),
  };
}
