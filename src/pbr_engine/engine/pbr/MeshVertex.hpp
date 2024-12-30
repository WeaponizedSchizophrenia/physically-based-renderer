#pragma once

#include "pbr/Vulkan.hpp"

#include "glm/ext/vector_float3.hpp"

#include <array>

namespace pbr {
 struct MeshVertex {
  glm::vec3 position;
  glm::vec3 color;

  static constexpr std::array attributes {
      vk::VertexInputAttributeDescription {
          .location = 0,
          .binding = 0,
          .format = vk::Format::eR32G32B32Sfloat,
          .offset = 0,
      },
      vk::VertexInputAttributeDescription {
          .location = 1,
          .binding = 0,
          .format = vk::Format::eR32G32B32Sfloat,
          // This is fine because MeshVertex has 4 byte alignment
          .offset = sizeof(glm::vec3),
      },
  };
};
} // namespace pbr
