#pragma once

#include <glm/ext/vector_float2.hpp>

namespace pbr::imgui {
struct PushConstant {
  glm::vec2 scale {};
  glm::vec2 translate {};
};
} // namespace pbr::imgui
