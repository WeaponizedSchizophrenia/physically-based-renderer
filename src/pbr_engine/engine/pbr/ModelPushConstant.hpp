#pragma once

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace pbr {
struct ModelPushConstant {
  glm::mat4x4 model;
  // This has to be a 3x4 matrix because of glsl alignment rules
  glm::mat3x4 normalModel;
};
/**
 * Creates a ModelPushConstant from a 4x4 matrix.
 */
[[nodiscard]]
constexpr auto makeModelPushConstant(glm::mat4x4 model) -> ModelPushConstant;
/**
 * Creates a ModelPushConstant from position, rotation and scale.
 */
[[nodiscard]]
constexpr auto makeModelPushConstant(glm::vec3 position, glm::quat rotation,
                                     glm::vec3 scale) -> ModelPushConstant;
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto pbr::makeModelPushConstant(glm::mat4x4 model) -> ModelPushConstant {
  return {
      .model = model,
      .normalModel {glm::mat3(glm::transpose(glm::inverse(model)))},
  };
}

constexpr auto pbr::makeModelPushConstant(glm::vec3 position, glm::quat rotation,
                                          glm::vec3 scale) -> ModelPushConstant {
  auto const mTranslation = glm::translate({1.0f}, position);
  auto const mRotation = glm::toMat4(rotation);
  auto const mScale = glm::scale({1.0f}, scale);
  return pbr::makeModelPushConstant(mTranslation * mRotation * mScale);
}
