#pragma once

#include <glm/ext/matrix_float4x4.hpp>

namespace pbr {
struct ModelPushConstant {
  glm::mat4x4 model;
};
}
