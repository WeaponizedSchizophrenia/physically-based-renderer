#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Uniform.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <glm/ext/vector_float4.hpp>

namespace pbr {
struct MaterialData {
  glm::vec4 color {};
};

class Material {
  Uniform<MaterialData> _materialData;
  vk::UniqueDescriptorSet _descriptorSet;

public:
  Material(core::GpuHandle const& gpu, IAllocator& allocator, vk::DescriptorPool descPool,
           vk::DescriptorSetLayout setLayout, MaterialData materialData);

  /* GETTERS */

  [[nodiscard]]
  constexpr auto getDescriptorSet() const noexcept -> vk::DescriptorSet;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto pbr::Material::getDescriptorSet() const noexcept -> vk::DescriptorSet {
  return _descriptorSet.get();
}
