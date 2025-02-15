#pragma once

#include "pbr/DescriptorSetAllocator.hpp"
#include "pbr/MeshBuilder.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/Material.hpp"
#include "pbr/Mesh.hpp"
#include "pbr/Scene.hpp"
#include "pbr/TransferStager.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <cstddef>
#include <memory>
#include <memory_resource>
#include <string>
#include <unordered_map>

#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>

namespace pbr::gltf {
struct AssetDependencies {
  core::SharedGpuHandle gpu;
  std::shared_ptr<IAllocator> allocator;
  DescriptorSetAllocator cameraAllocator;
  DescriptorSetAllocator materialAllocator;
};
/**
 * Contains data for a gltf asset.
 */
class Asset {
  AssetDependencies _dependencies;

  fastgltf::GltfDataBuffer _dataBuffer;
  fastgltf::Asset _asset;

  template <typename T>
  using Cache = std::pmr::unordered_map<std::pmr::string, std::shared_ptr<T>>;
  Cache<vk::UniqueSampler> _samplerCache;
  Cache<Image2D> _imageCache;
  Cache<Material> _materialCache;
  Cache<Mesh> _meshCache;

public:
  Asset(fastgltf::GltfDataBuffer dataBuffer, fastgltf::Asset asset,
        AssetDependencies dependencies) noexcept;

  [[nodiscard]]
  auto loadSampler(std::size_t index) -> std::shared_ptr<vk::UniqueSampler>;

  [[nodiscard]]
  auto loadImage2D(TransferStager& stager, std::size_t index) -> std::shared_ptr<Image2D>;

  [[nodiscard]]
  auto loadMaterial(TransferStager& stager, std::size_t index)
      -> std::shared_ptr<Material>;

  [[nodiscard]]
  auto loadPrimitive(TransferStager& stager, fastgltf::Primitive const& primitive)
      -> MeshBuilder::Primitive;

  /**
   * Builds a mesh from the contained gltf asset mesh at index.
   */
  [[nodiscard]]
  auto loadMesh(TransferStager& stager, std::size_t index) -> std::shared_ptr<Mesh>;

  [[nodiscard]]
  auto loadNode(TransferStager& stager, std::size_t index,
                std::pmr::polymorphic_allocator<> alloc = {}) -> Node;

  [[nodiscard]]
  auto loadScene(TransferStager& stager, std::size_t index,
                 std::pmr::polymorphic_allocator<> alloc = {}) -> Scene;
};
} // namespace pbr::gltf
