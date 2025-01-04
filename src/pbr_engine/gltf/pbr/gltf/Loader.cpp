#include "pbr/gltf/Loader.hpp"

#include "pbr/gltf/Asset.hpp"

#include <fastgltf/core.hpp>
#include <filesystem>
#include <format>
#include <stdexcept>

auto pbr::gltf::Loader::loadAsset(std::filesystem::path const& path,
                                  AssetDependencies dependencies) -> Asset {
  auto buffer = fastgltf::GltfDataBuffer::FromPath(path);
  if (buffer.error() != fastgltf::Error::None) {
    throw std::runtime_error(std::format("Cant load gltf buffer from {} error: {}",
                                         path.c_str(),
                                         fastgltf::getErrorMessage(buffer.error())));
  }
  auto asset = _parser.loadGltf(buffer.get(), path.parent_path(),
                                fastgltf::Options::LoadExternalBuffers
                                    | fastgltf::Options::DecomposeNodeMatrices
                                    | fastgltf::Options::GenerateMeshIndices);
  if (asset.error() != fastgltf::Error::None) {
    throw std::runtime_error(std::format("Cant load gltf asset from {} error: {}",
                                         path.c_str(),
                                         fastgltf::getErrorMessage(asset.error())));
  }
  return {
      std::move(buffer.get()),
      std::move(asset.get()),
      std::move(dependencies),
  };
}
