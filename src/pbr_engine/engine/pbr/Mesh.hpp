#pragma once

#include "pbr/Buffer.hpp"

#include <cstdint>
#include <span>
#include <vector>

namespace pbr {
/**
  * Describes a primitive inside a vertex and index buffers.
*/
struct PrimitiveSpan {
  std::uint32_t firstVertex;
  std::uint32_t vertexCount;
  std::uint32_t firstIndex;
  std::uint32_t indexCount;
};
/**
 * Represents a single mesh or a collection of primitives. (Modelled of gltf)
 */
class Mesh {
  Buffer _vertexBuffer;
  Buffer _indexBuffer;
  std::vector<PrimitiveSpan> _primitives;

public:
  constexpr Mesh(Buffer vertexBuffer, Buffer indexBuffer,
       std::vector<PrimitiveSpan> primitives) noexcept;

  [[nodiscard]]
  constexpr auto getVertexBuffer() const noexcept -> Buffer const&;

  [[nodiscard]]
  constexpr auto getIndexBuffer() const noexcept -> Buffer const&;

  [[nodiscard]]
  constexpr auto getPrimitives() const noexcept -> std::span<PrimitiveSpan const>;
};
}

/* IMPLEMENTATIONS */

constexpr pbr::Mesh::Mesh(Buffer vertexBuffer, Buffer indexBuffer,
                std::vector<PrimitiveSpan> primitives) noexcept
    : _vertexBuffer(std::move(vertexBuffer))
    , _indexBuffer(std::move(indexBuffer))
    , _primitives(std::move(primitives)) {}

constexpr auto pbr::Mesh::getVertexBuffer() const noexcept -> Buffer const& {
  return _vertexBuffer;
}

constexpr auto pbr::Mesh::getIndexBuffer() const noexcept -> Buffer const& {
  return _indexBuffer;
}

constexpr auto pbr::Mesh::getPrimitives() const noexcept -> std::span<PrimitiveSpan const> {
  return _primitives;
}
