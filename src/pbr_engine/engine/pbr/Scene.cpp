#include "pbr/Scene.hpp"

#include "pbr/CameraUniform.hpp"
#include "pbr/Mesh.hpp"

#include <generator>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <utility>

pbr::Node::Node(std::pmr::string name, allocator_type alloc)
    : _name(std::move(name), alloc), _children(alloc) {}

pbr::Node::Node(std::pmr::string name, Transform transform, allocator_type alloc)
    : Node(std::move(name), alloc) {
  _transform = transform;
}

pbr::Node::Node(Node&& node, allocator_type alloc) noexcept
    : _name(std::move(node._name), alloc)
    , _children(std::move(node._children), alloc)
    , _transform(node._transform)
    , _mesh(std::move(node._mesh))
    , _camera(std::move(node._camera)) {}

auto pbr::Node::getName() const noexcept -> std::pmr::string const& { return _name; }

auto pbr::Node::getChildren() const noexcept -> std::span<Node const> {
  return _children;
}

auto pbr::Node::getChildren() noexcept -> std::span<Node> { return _children; }

auto pbr::Node::getTransform() const noexcept -> Transform { return _transform; }

auto pbr::Node::setTransform(Transform transform) noexcept -> void {
  _transform = transform;
}

auto pbr::Node::getMesh() const noexcept -> std::shared_ptr<Mesh> const& { return _mesh; }

auto pbr::Node::setMesh(std::shared_ptr<Mesh> mesh) noexcept -> void {
  _mesh = std::move(mesh);
}

auto pbr::Node::getCamera() const noexcept -> std::shared_ptr<CameraUniform> const& {
  return _camera;
}

auto pbr::Node::setCamera(std::shared_ptr<CameraUniform> camera) noexcept -> void {
  _camera = std::move(camera);
}

auto pbr::Node::iterateChildren() const -> std::generator<Node const&> {
  for (auto const& child : _children) {
    co_yield std::ranges::elements_of(child.iterateChildren());
    co_yield child;
  }
}

auto pbr::Node::iterateChildren() -> std::generator<Node&> {
  for (auto& child : _children) {
    co_yield std::ranges::elements_of(child.iterateChildren());
    co_yield child;
  }
}

auto pbr::Node::addChild(Node node) -> Node& {
  _children.emplace_back(std::move(node));
  return _children.back();
}

pbr::Scene::Scene(allocator_type alloc) : _nodes(alloc) {}

auto pbr::Scene::iterateAllNodes() const -> std::generator<Node const&> {
  for (auto const& node : _nodes) {
    co_yield std::ranges::elements_of(node.iterateChildren());
    co_yield node;
  }
}

auto pbr::Scene::iterateAllNodes() -> std::generator<Node&> {
  for (auto& node : _nodes) {
    co_yield std::ranges::elements_of(node.iterateChildren());
    co_yield node;
  }
}

auto pbr::Scene::getTopLevelNodes() const noexcept -> std::span<Node const> {
  return _nodes;
}

auto pbr::Scene::getTopLevelNodes() noexcept -> std::span<Node> { return _nodes; }

auto pbr::Scene::findCamera() const -> std::optional<std::shared_ptr<CameraUniform>> {
  for (const auto& node : iterateAllNodes()) {
    if (auto const& camera = node.getCamera(); camera) {
      return camera;
    }
  }
  return std::nullopt;
}

auto pbr::Scene::addNode(Node node) -> Node& {
  _nodes.emplace_back(std::move(node));
  return _nodes.back();
}
