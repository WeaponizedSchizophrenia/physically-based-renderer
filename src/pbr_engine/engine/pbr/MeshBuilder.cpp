#include "pbr/MeshBuilder.hpp"

#include "pbr/Mesh.hpp"
#include "pbr/MeshVertex.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

auto pbr::MeshBuilder::addPrimitive(Primitive primitive) -> MeshBuilder& {
  _primitives.emplace_back(std::move(primitive));
  return *this;
}

auto pbr::MeshBuilder::build() const -> BuiltMesh {
  std::vector<MeshVertex> vertices;
  vertices.reserve(std::ranges::fold_left(
      _primitives, 0uz, [](std::size_t acc, Primitive const& primitive) {
        return acc + primitive.vertices.size();
      }));
  std::vector<std::uint16_t> indices;
  indices.reserve(std::ranges::fold_left(_primitives, 0uz,
                                         [](std::size_t acc, Primitive const& primitive) {
                                           return acc + primitive.indices.size();
                                         }));
  std::vector<PrimitiveSpan> primitives;
  primitives.reserve(_primitives.size());

  std::uint32_t currentVertex {};
  std::uint32_t currentIndex {};
  for (auto const& primitive : _primitives) {
    vertices.insert(vertices.end(), primitive.vertices.begin(), primitive.vertices.end());
    indices.insert(indices.end(), primitive.indices.begin(), primitive.indices.end());
    auto const vertexCount = static_cast<std::uint32_t>(primitive.vertices.size());
    auto const indexCount = static_cast<std::uint32_t>(primitive.indices.size());

    primitives.push_back({
        .firstVertex = currentVertex,
        .vertexCount = vertexCount,
        .firstIndex = currentIndex,
        .indexCount = indexCount,
    });

    currentVertex += vertexCount;
    currentIndex += indexCount;
  }

  return {
      .vertices = std::move(vertices),
      .indices = std::move(indices),
      .primitives = std::move(primitives),
  };
}
