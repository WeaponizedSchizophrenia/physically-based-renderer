#pragma once

#include "pbr/MeshBuilder.hpp"

#include <cstddef>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

namespace pbr::gltf {
class Asset {
  fastgltf::GltfDataBuffer _dataBuffer;
  fastgltf::Asset _asset;

public:
  Asset(fastgltf::GltfDataBuffer dataBuffer, fastgltf::Asset asset) noexcept;

  [[nodiscard]]
  auto buildMesh(std::size_t index) -> MeshBuilder::BuiltMesh;
};
}
