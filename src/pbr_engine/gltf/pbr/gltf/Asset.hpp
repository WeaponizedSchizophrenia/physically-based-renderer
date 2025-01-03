#pragma once

#include "pbr/Mesh.hpp"
#include "pbr/TransferStager.hpp"

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
  auto loadMesh(TransferStager& stager, std::size_t index) -> Mesh;
};
} // namespace pbr::gltf
