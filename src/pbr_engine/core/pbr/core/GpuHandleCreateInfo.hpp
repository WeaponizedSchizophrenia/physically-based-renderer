#pragma once

#include "pbr/Vulkan.hpp"

#include <cstdint>
#include <functional>
#include <span>

namespace pbr::core {
struct GpuHandleCreateInfo {
  using QueueFamilyPresentPredicate =
      std::function<bool(vk::Instance, vk::PhysicalDevice, std::uint32_t)>;

  std::span<char const* const> extensions;
  QueueFamilyPresentPredicate presentPredicate;
  bool enableValidation;
};
} // namespace pbr::core
