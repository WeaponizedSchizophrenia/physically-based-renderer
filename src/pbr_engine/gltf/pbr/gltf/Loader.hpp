#pragma once

#include "pbr/gltf/Asset.hpp"

#include <fastgltf/core.hpp>

#include <filesystem>

namespace pbr::gltf {
/**
 * Type that caches the fastgltf::Parser for asset loading.
 */
class Loader {
  fastgltf::Parser _parser;

public:
  /**
   * Loads the gltf asset from the specified path.
   */
  [[nodiscard]]
  auto loadAsset(std::filesystem::path const& path, AssetDependencies dependencies) -> Asset;
};
} // namespace pbr::gltf
