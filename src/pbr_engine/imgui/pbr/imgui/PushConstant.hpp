#pragma once

#include <glm/ext/vector_float2.hpp>

namespace pbr::imgui {
/**
 * Push constant for imgui vertex shader.
 */
struct PushConstant {
  glm::vec2 scale {};
  glm::vec2 translate {};
};
} // namespace pbr::imgui
