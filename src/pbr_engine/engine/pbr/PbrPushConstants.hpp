#pragma once

#include "glm/ext/vector_float3.hpp"

namespace pbr {
struct PbrPushConstants {
  glm::vec3 color;
  float mixFactor;
};
}
