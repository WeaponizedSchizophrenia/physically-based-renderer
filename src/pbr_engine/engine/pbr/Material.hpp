#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/Uniform.hpp"

#include <glm/ext/vector_float4.hpp>
#include <memory>

namespace pbr {
struct MaterialData {
  glm::vec4 color {};
};

class Material {
  Uniform<MaterialData> _materialData;
  std::shared_ptr<Image2D> _colorTexture;
  std::shared_ptr<vk::UniqueSampler> _colorSampler;
  std::shared_ptr<Image2D> _normalTexture;
  std::shared_ptr<vk::UniqueSampler> _normalSampler;
  vk::UniqueDescriptorSet _descriptorSet;

public:
  Material(core::GpuHandle const& gpu, Uniform<MaterialData> matData,
           std::shared_ptr<Image2D> colorTexture,
           std::shared_ptr<vk::UniqueSampler> colorSampler,
           std::shared_ptr<Image2D> normalTexture,
           std::shared_ptr<vk::UniqueSampler> normalSampler,
           vk::UniqueDescriptorSet descSet);

  /* GETTERS */

  [[nodiscard]]
  constexpr auto getDescriptorSet() const noexcept -> vk::DescriptorSet;

private:
  auto writeDescriptorSet(core::GpuHandle const& gpu) -> void;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto pbr::Material::getDescriptorSet() const noexcept -> vk::DescriptorSet {
  return _descriptorSet.get();
}
