#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/Buffer.hpp"
#include "pbr/memory/AllocationInfo.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <concepts>
#include <cstring>
#include <memory>
#include <type_traits>

namespace pbr {
template <typename T>
concept UniformValue = std::semiregular<T> && std::is_trivially_copyable_v<T>;

template <UniformValue T> class Uniform {
  Buffer _buffer;

public:
  constexpr Uniform(IAllocator& allocator, T init = {});

  [[nodiscard]]
  constexpr auto get() const -> T;

  constexpr auto set(T value) -> void;

  [[nodiscard]]
  constexpr auto getUniformBuffer() const noexcept -> Buffer const&;
};
} // namespace pbr

/* IMPLEMENTATIONS */

template <pbr::UniformValue T>
constexpr pbr::Uniform<T>::Uniform(IAllocator& allocator, T init)
    : _buffer(allocator.allocateBuffer(
          {
              .size = sizeof(T),
              .usage = vk::BufferUsageFlagBits::eUniformBuffer,
          },
          {
              .preference = AllocationPreference::Host,
              .ableToBeMapped = true,
              .persistentlyMapped = true,
          })) {
  set(init);
}

template <pbr::UniformValue T> constexpr auto pbr::Uniform<T>::get() const -> T {
  T value {};
  {
    auto const mapping = _buffer.map();
    std::memcpy(std::addressof(value), mapping.get(), sizeof(T));
  }
  return value;
}

template <pbr::UniformValue T> constexpr auto pbr::Uniform<T>::set(T value) -> void {
  auto const mapping = _buffer.map();
  std::memcpy(mapping.get(), std::addressof(value), sizeof(T));
}

template <pbr::UniformValue T>
constexpr auto pbr::Uniform<T>::getUniformBuffer() const noexcept -> Buffer const& {
  return _buffer;
}
