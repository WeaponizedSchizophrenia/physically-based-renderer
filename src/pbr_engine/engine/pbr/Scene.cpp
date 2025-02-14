#include "pbr/Scene.hpp"

#include "pbr/CameraUniform.hpp"
#include "pbr/Mesh.hpp"

#include <generator>
#include <memory>
#include <optional>
#include <utility>

pbr::Node::Node(Transform transform) noexcept : _transform(transform) {}

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

auto pbr::Scene::iterateNodes() const -> std::generator<Node const&> {
  for (auto const& node : _nodes) {
    co_yield std::ranges::elements_of(node.iterateChildren());
    co_yield node;
  }
}

auto pbr::Scene::iterateNodes() -> std::generator<Node&> {
  for (auto& node : _nodes) {
    co_yield std::ranges::elements_of(node.iterateChildren());
    co_yield node;
  }
}

auto pbr::Scene::findCamera() const -> std::optional<std::shared_ptr<CameraUniform>> {
  for (const auto& node : iterateNodes()) {
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
