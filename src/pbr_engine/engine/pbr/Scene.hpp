#pragma once

#include "pbr/CameraUniform.hpp"
#include "pbr/Mesh.hpp"

#include <generator>
#include <memory>
#include <memory_resource>
#include <optional>
#include <span>
#include <string>
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
  std::pmr::string _name;
  std::pmr::vector<Node> _children;
  Transform _transform {};
  std::shared_ptr<Mesh> _mesh = nullptr;
  std::shared_ptr<CameraUniform> _camera = nullptr;

public:
  using allocator_type = std::pmr::polymorphic_allocator<>;

  explicit Node(std::pmr::string name, allocator_type alloc = {});
  explicit Node(std::pmr::string name, Transform transform, allocator_type alloc = {});

  Node(const Node&) = delete;
  auto operator=(const Node&) -> Node& = delete;

  Node(Node&& node, allocator_type alloc = {}) noexcept;
  auto operator=(Node&&) -> Node& = default;

  ~Node() = default;

  [[nodiscard]]
  auto getName() const noexcept -> std::pmr::string const&;

  [[nodiscard]]
  auto getChildren() const noexcept -> std::span<Node const>;

  [[nodiscard]]
  auto getChildren() noexcept -> std::span<Node>;

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
  using allocator_type = std::pmr::polymorphic_allocator<>;

  explicit Scene(allocator_type alloc = {});

  [[nodiscard]]
  auto iterateAllNodes() const -> std::generator<Node const&>;

  [[nodiscard]]
  auto iterateAllNodes() -> std::generator<Node&>;

  [[nodiscard]]
  auto getTopLevelNodes() const noexcept -> std::span<Node const>;

  [[nodiscard]]
  auto getTopLevelNodes() noexcept -> std::span<Node>;

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
