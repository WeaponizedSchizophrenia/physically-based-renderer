#pragma once

#include "pbr/Vulkan.hpp"

#include <cstddef>
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
[[nodiscard]]
consteval auto getMeshVertexAttributes() -> auto {
  return std::array {
      vk::VertexInputAttributeDescription {
          .location = 0,
          .binding = 0,
          .format = vk::Format::eR32G32B32Sfloat,
          .offset = offsetof(MeshVertex, position),
      },
      vk::VertexInputAttributeDescription {
          .location = 1,
          .binding = 0,
          .format = vk::Format::eR32G32B32Sfloat,
          .offset = offsetof(MeshVertex, normal),
      },
      vk::VertexInputAttributeDescription {
          .location = 2,
          .binding = 0,
          .format = vk::Format::eR32G32B32A32Sfloat,
          .offset = offsetof(MeshVertex, tangent),
      },
      vk::VertexInputAttributeDescription {
          .location = 3,
          .binding = 0,
          .format = vk::Format::eR32G32Sfloat,
          .offset = offsetof(MeshVertex, texCoords),
      },
  };
}
} // namespace pbr
