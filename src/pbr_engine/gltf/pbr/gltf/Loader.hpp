#pragma once

#include "pbr/gltf/Asset.hpp"

#include <fastgltf/core.hpp>

#include <filesystem>

namespace pbr::gltf {
class Loader {
  fastgltf::Parser _parser;

public:
  Loader() = default;

  [[nodiscard]]
  auto loadAsset(std::filesystem::path const& path) -> Asset;
};
}
