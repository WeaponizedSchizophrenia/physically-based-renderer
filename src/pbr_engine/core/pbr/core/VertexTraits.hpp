#pragma once

#include "pbr/Vulkan.hpp"

#include <concepts>
#include <span>
#include <type_traits>

namespace pbr::core {
template <typename T> struct VertexTraits;

template <typename T>
concept Vertex = std::semiregular<T> && std::is_trivially_copyable_v<T> && requires {
  {
    VertexTraits<T>::attributeFormats
  } -> std::convertible_to<std::span<vk::Format const>>;
};
} // namespace pbr::core
