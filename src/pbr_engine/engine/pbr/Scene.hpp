#pragma once

#include "pbr/CameraUniform.hpp"
#include "pbr/Mesh.hpp"

#include <generator>
#include <memory>
#include <optional>
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
class Node {
  std::vector<Node> _children;
  Transform _transform {};
  std::shared_ptr<Mesh> _mesh = nullptr;
  std::shared_ptr<CameraUniform> _camera = nullptr;

public:
  explicit Node(Transform transform = {}) noexcept;

  [[nodiscard]]
  auto getTransform() const noexcept -> Transform;

  auto setTransform(Transform) noexcept -> void;

  [[nodiscard]]
  auto getMesh() const noexcept -> std::shared_ptr<Mesh> const&;

  auto setMesh(std::shared_ptr<Mesh>) noexcept -> void;

  [[nodiscard]]
  auto getCamera() const noexcept -> std::shared_ptr<CameraUniform> const&;

  auto setCamera(std::shared_ptr<CameraUniform>) noexcept -> void;

  [[nodiscard]]
  auto iterateChildren() const -> std::generator<Node const&>;

  [[nodiscard]]
  auto iterateChildren() -> std::generator<Node&>;

  /**
   * Adds a new node to the tree that will be this nodes child.
   * @returns A reference to the newly added node.
   *
   * @note Every time this is called anything that references any child of this node will
   * be invalidated.
   */
  auto addChild(Node) -> Node&;
};
class Scene {
  std::pmr::vector<Node> _nodes;

public:
  [[nodiscard]]
  auto iterateNodes() const -> std::generator<Node const&>;

  [[nodiscard]]
  auto iterateNodes() -> std::generator<Node&>;

  [[nodiscard]]
  auto findCamera() const -> std::optional<std::shared_ptr<CameraUniform>>;

  /**
   * Adds a new top level node to the scene.
   * @returns A reference to the newly added node.
   *
   * @note Every time this is called anything that references any top level node will
   * be invalidated.
   */
  auto addNode(Node) -> Node&;
};
} // namespace pbr
