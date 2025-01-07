#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/VertexTraits.hpp"

#include <array>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>

namespace pbr {
struct MeshVertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec4 tangent;
  glm::vec2 texCoords;
};
template <> struct core::VertexTraits<MeshVertex> {
  static constexpr std::array attributeFormats {
      vk::Format::eR32G32B32Sfloat,
      vk::Format::eR32G32B32Sfloat,
      vk::Format::eR32G32B32A32Sfloat,
      vk::Format::eR32G32Sfloat,
  };
};
} // namespace pbr

static_assert(pbr::core::Vertex<pbr::MeshVertex>, "pbr::MeshVertex does not satisfy pbr::core::Vertex");
