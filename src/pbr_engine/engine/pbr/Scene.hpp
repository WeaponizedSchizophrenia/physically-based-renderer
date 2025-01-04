#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/PbrPipeline.hpp"
#include "pbr/CameraUniform.hpp"
#include "pbr/Mesh.hpp"

#include <memory>
#include <vector>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/vector_float3.hpp>

namespace pbr {
struct Transform {
  glm::vec3 position {};
  glm::quat rotation = glm::identity<glm::quat>();
  glm::vec3 scale {1.0f};
};
struct MeshInstance {
  std::shared_ptr<Mesh> mesh {};
  Transform transform {};
};
struct Scene {
  std::vector<MeshInstance> meshes;
  std::shared_ptr<CameraUniform> camera;
};
auto renderScene(vk::CommandBuffer cmdBuffer, PbrPipeline const& pbrPipeline, Scene const& scene) -> void;
} // namespace pbr
