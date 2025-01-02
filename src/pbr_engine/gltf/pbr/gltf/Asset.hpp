#pragma once

#include "pbr/MeshBuilder.hpp"

#include <cstddef>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

namespace pbr::gltf {
/**
 * Contains data for a gltf asset.
 */
class Asset {
  fastgltf::GltfDataBuffer _dataBuffer;
  fastgltf::Asset _asset;

public:
  Asset(fastgltf::GltfDataBuffer dataBuffer, fastgltf::Asset asset) noexcept;

  /**
   * Builds a mesh from the contained gltf asset mesh at index.
   */
  [[nodiscard]]
  auto buildMesh(std::size_t index) -> MeshBuilder::BuiltMesh;
};
} // namespace pbr::gltf
