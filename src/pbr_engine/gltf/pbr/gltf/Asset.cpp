#include "pbr/gltf/Asset.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/CameraUniform.hpp"
#include "pbr/DescriptorSetAllocator.hpp"
#include "pbr/Image2D.hpp"
#include "pbr/Material.hpp"
#include "pbr/Mesh.hpp"
#include "pbr/MeshBuilder.hpp"
#include "pbr/MeshVertex.hpp"
#include "pbr/Scene.hpp"
#include "pbr/TransferStager.hpp"
#include "pbr/Uniform.hpp"
#include "pbr/image/LoadImage.hpp"

#include <cassert>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <format>
#include <iterator>
#include <memory>
#include <memory_resource>
#include <print>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/util.hpp>

#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/constants.hpp>

namespace constants {
static constexpr auto POSITION_NAME = "POSITION";
static constexpr auto NORMAL_NAME = "NORMAL";
[[maybe_unused]]
static constexpr auto TANGENT_NAME = "TANGENT";
[[maybe_unused]]
static constexpr auto TEX_COORDS_NAME = "TEXCOORD_0";
} // namespace constants

class ImageDataSourceVisitor {
  fastgltf::Asset const* _asset;
  pbr::core::GpuHandle const* _gpu;
  pbr::TransferStager* _stager;

public:
  ImageDataSourceVisitor(fastgltf::Asset const& asset, pbr::core::GpuHandle const& gpu,
                         pbr::TransferStager& stager) noexcept
      : _asset(&asset), _gpu(&gpu), _stager(&stager) {}

  constexpr auto operator()(auto&... /**/) -> pbr::Image2D {
    assert("Not implemented" && false);
    std::unreachable();
  }

  [[nodiscard]]
  constexpr auto operator()(fastgltf::sources::BufferView view) -> pbr::Image2D {
    auto const& bufView = _asset->bufferViews.at(view.bufferViewIndex);
    auto const& buffer = _asset->buffers.at(bufView.bufferIndex);
    return std::visit(fastgltf::visitor {
                          [](auto&...) -> pbr::Image2D {
                            assert("Not implemented" && false);
                            std::unreachable();
                          },
                          [&](fastgltf::sources::Array const& array) {
                            std::span const bufferSpan(
                                // reinterpret cast is fine here because it is a
                                // cast to std::uint8_t NOLINTNEXTLINE
                                reinterpret_cast<std::uint8_t const*>(std::next(
                                    array.bytes.data(),
                                    static_cast<std::ptrdiff_t>(bufView.byteOffset))),
                                bufView.byteLength);
                            return pbr::image::loadImage2D(*_gpu, *_stager, bufferSpan);
                          },
                      },
                      buffer.data);
  }

  [[nodiscard]]
  constexpr auto operator()(fastgltf::sources::URI const& uri) -> pbr::Image2D {
    if (uri.fileByteOffset != 0) {
      throw std::runtime_error("URI fileByteOffset is not supported");
    }

    return pbr::image::loadImage2D(*_gpu, *_stager, uri.uri.path());
  }
};

pbr::gltf::Asset::Asset(fastgltf::GltfDataBuffer dataBuffer, fastgltf::Asset asset,
                        AssetDependencies dependencies) noexcept
    : _dependencies(std::move(dependencies))
    , _dataBuffer(std::move(dataBuffer))
    , _asset(std::move(asset)) {}

auto pbr::gltf::Asset::loadSampler(std::size_t index)
    -> std::shared_ptr<vk::UniqueSampler> {
  [[maybe_unused]]
  auto const& samplerInfo = _asset.samplers.at(index);
  if (auto iter = _samplerCache.find(samplerInfo.name); iter != _samplerCache.end()) {
    return iter->second;
  }
  auto sampler = std::make_shared<vk::UniqueSampler>(
      _dependencies.gpu->getDevice().createSamplerUnique({}));
  _samplerCache[samplerInfo.name] = sampler;
  return sampler;
}

auto pbr::gltf::Asset::loadImage2D(TransferStager& stager, std::size_t index)
    -> std::shared_ptr<Image2D> {
  auto const& image = _asset.images.at(index);
  if (auto iter = _imageCache.find(image.name); iter != _imageCache.end()) {
    return iter->second;
  }

  auto image2D = std::make_shared<Image2D>(
      std::visit(ImageDataSourceVisitor(_asset, *_dependencies.gpu, stager), image.data));
  _imageCache[image.name] = image2D;
  return image2D;
}

auto pbr::gltf::Asset::loadMaterial(TransferStager& stager, std::size_t index)
    -> std::shared_ptr<Material> {
  auto const& matInfo = _asset.materials.at(index);
  if (auto iter = _materialCache.find(matInfo.name); iter != _materialCache.end()) {
    return iter->second;
  }

  auto const baseColorFactor = matInfo.pbrData.baseColorFactor;
  MaterialData const matData {
      .color {baseColorFactor.x(), baseColorFactor.y(), baseColorFactor.z(),
              baseColorFactor.w()},
  };
  auto const& colorTexture =
      _asset.textures.at(matInfo.pbrData.baseColorTexture.value().textureIndex);
  auto const& normalTexture =
      _asset.textures.at(matInfo.normalTexture.value().textureIndex);
  auto material = std::make_shared<Material>(
      *_dependencies.gpu, Uniform<MaterialData>(*_dependencies.allocator, matData),
      loadImage2D(stager, colorTexture.imageIndex.value()),
      loadSampler(colorTexture.samplerIndex.value()),
      loadImage2D(stager, normalTexture.imageIndex.value()),
      loadSampler(normalTexture.samplerIndex.value()),
      _dependencies.materialAllocator.allocate());
  _materialCache[matInfo.name] = material;
  return material;
}

auto pbr::gltf::Asset::loadPrimitive(TransferStager& stager,
                                     fastgltf::Primitive const& primitive)
    -> pbr::MeshBuilder::Primitive {
  auto const getAccessor = [&](std::string_view attr) {
    auto const* const iter = primitive.findAttribute(attr);
    if (iter != primitive.attributes.end()) {
      return _asset.accessors.at(iter->accessorIndex);
    }
    throw std::runtime_error(std::format("Primitive does not have {} attribute", attr));
  };

  auto const positionAcc = getAccessor(constants::POSITION_NAME);
  auto const normalAcc = getAccessor(constants::NORMAL_NAME);
  auto const tangentAcc = getAccessor(constants::TANGENT_NAME);
  auto const texCoordsAcc = getAccessor(constants::TEX_COORDS_NAME);

  std::vector<pbr::MeshVertex> vertices(positionAcc.count);

  fastgltf::iterateAccessorWithIndex<glm::vec3>(
      _asset, positionAcc,
      [&vertices](glm::vec3 pos, std::size_t idx) { vertices.at(idx).position = pos; });
  fastgltf::iterateAccessorWithIndex<glm::vec3>(
      _asset, normalAcc,
      [&vertices](glm::vec3 nor, std::size_t idx) { vertices.at(idx).normal = nor; });
  fastgltf::iterateAccessorWithIndex<glm::vec4>(
      _asset, tangentAcc,
      [&vertices](glm::vec4 tan, std::size_t idx) { vertices.at(idx).tangent = tan; });
  fastgltf::iterateAccessorWithIndex<glm::vec2>(
      _asset, texCoordsAcc, [&vertices](glm::vec2 texCoords, std::size_t idx) {
        vertices.at(idx).texCoords = texCoords;
      });

  assert(primitive.indicesAccessor.has_value());
  auto const indexAcc = _asset.accessors.at(primitive.indicesAccessor.value());

  std::vector<std::uint16_t> indices;
  indices.reserve(indexAcc.count);

  fastgltf::iterateAccessor<std::uint16_t>(
      _asset, indexAcc, [&indices](std::size_t idx) { indices.push_back(idx); });

  return {
      .material = loadMaterial(stager, primitive.materialIndex.value()),
      .vertices = std::move(vertices),
      .indices = std::move(indices),
  };
}

auto pbr::gltf::Asset::loadMesh(TransferStager& stager, std::size_t index)
    -> std::shared_ptr<Mesh> {
  auto const& meshInfo = _asset.meshes.at(index);
  if (auto iter = _meshCache.find(meshInfo.name); iter != _meshCache.end()) {
    return iter->second;
  }

  MeshBuilder meshBuilder;
  for (auto const& primitive : meshInfo.primitives) {
    meshBuilder.addPrimitive(loadPrimitive(stager, primitive));
  }
  auto builtMesh = meshBuilder.build();

  std::vector<std::byte> vertices(builtMesh.vertices.size() * sizeof(MeshVertex));
  std::memcpy(vertices.data(), builtMesh.vertices.data(), vertices.size());
  std::vector<std::byte> indices(builtMesh.indices.size() * sizeof(std::uint16_t));
  std::memcpy(indices.data(), builtMesh.indices.data(), indices.size());

  auto mesh = std::make_shared<Mesh>(
      stager.addTransfer(std::move(vertices), vk::BufferUsageFlagBits::eVertexBuffer),
      stager.addTransfer(std::move(indices), vk::BufferUsageFlagBits::eIndexBuffer),
      std::move(builtMesh.primitives));
  _meshCache[meshInfo.name] = mesh;
  return mesh;
}

auto pbr::gltf::Asset::loadNode(TransferStager& stager, std::size_t index,
                                std::pmr::polymorphic_allocator<> alloc) -> Node {
  auto const& gltfNode = _asset.nodes.at(index);
  auto const trs = std::get<fastgltf::TRS>(gltfNode.transform);
  Transform const transform {
      .position {trs.translation.x(), trs.translation.y(), trs.translation.z()},
      .rotation {trs.rotation.w(), trs.rotation.x(), trs.rotation.y(), trs.rotation.z()},
      .scale {trs.scale.x(), trs.scale.y(), trs.scale.z()},
  };

  Node node(transform, alloc);

  for (auto const childIdx : gltfNode.children) {
    node.addChild(loadNode(stager, childIdx, alloc));
  }

  if (gltfNode.meshIndex) {
    node.setMesh(loadMesh(stager, *gltfNode.meshIndex));
  }

  return node;
}

auto pbr::gltf::Asset::loadScene(TransferStager& stager, std::size_t index,
                                 std::pmr::polymorphic_allocator<> alloc) -> Scene {
  Scene scene(alloc);
  scene.addNode(Node()).setCamera(
      std::make_shared<CameraUniform>(*_dependencies.gpu, *_dependencies.allocator,
                                      _dependencies.cameraAllocator.allocate()));

  auto const& gltfScene = _asset.scenes.at(index);
  for (auto const nodeIdx : gltfScene.nodeIndices) {
    scene.addNode(loadNode(stager, nodeIdx, alloc));
  }

  return scene;
}
