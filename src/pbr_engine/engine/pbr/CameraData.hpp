#pragma once

#include "pbr/Uniform.hpp"

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace pbr {
struct alignas(sizeof(glm::vec4)) CameraData {
  glm::mat4x4 view {};
  glm::mat4x4 proj {};
  glm::vec3 position {};
};
static_assert(UniformValue<CameraData>,
              "pbr::CameraData does not satisfy pbr::UniformValue");

[[nodiscard]]
constexpr auto makeCameraData(glm::vec3 position, glm::vec3 target, float fov,
                              float aspect) noexcept -> CameraData {
  static constexpr auto ZNEAR = 0.01f;
  static constexpr auto ZFAR = 1024.0f;
  return {
      .view = glm::lookAtRH(position, target, {0.0f, 1.0f, 0.0f}),
      .proj = glm::perspectiveRH_NO(fov, aspect, ZNEAR, ZFAR),
      .position = position,
  };
}
} // namespace pbr
