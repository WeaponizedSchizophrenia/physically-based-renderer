#pragma once

#include "pbr/Material.hpp"
#include "pbr/Mesh.hpp"
#include "pbr/MeshVertex.hpp"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace pbr {
class MeshBuilder {
public:
  struct Primitive {
    std::shared_ptr<Material> material {};
    std::vector<MeshVertex> vertices {};
    std::vector<std::uint16_t> indices {};
  };
  struct BuiltMesh {
    std::vector<MeshVertex> vertices;
    std::vector<std::uint16_t> indices;
    std::vector<PrimitiveSpan> primitives;
  };

private:
  std::vector<Primitive> _primitives {};

public:
  MeshBuilder() = default;

  auto addPrimitive(Primitive primitive) -> MeshBuilder&;
  constexpr auto addPrimitive(std::vector<MeshVertex> vertices,
                              std::vector<std::uint16_t> indices) -> MeshBuilder&;

  [[nodiscard]]
  auto build() const -> BuiltMesh;
};
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr auto
pbr::MeshBuilder::addPrimitive(std::vector<MeshVertex> vertices,
                               std::vector<std::uint16_t> indices) -> MeshBuilder& {
  return addPrimitive({.vertices = std::move(vertices), .indices = std::move(indices)});
}
