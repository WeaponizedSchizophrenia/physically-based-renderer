#include "pbr/gltf/Asset.hpp"

#include "pbr/MeshBuilder.hpp"
#include "pbr/MeshVertex.hpp"

#include <cassert>
#include <cstddef>
#include <format>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>

#include <glm/ext/vector_float3.hpp>

namespace constants {
static constexpr auto POSITION_NAME = "POSITION";
static constexpr auto NORMAL_NAME = "NORMAL";
[[maybe_unused]]
static constexpr auto TANGENT_NAME = "TANGENT";
[[maybe_unused]]
static constexpr auto TEX_COORD_NAME = "TEXCOORD_0";
} // namespace constants

namespace {
[[nodiscard]]
constexpr auto
loadPrimitive(fastgltf::Asset const& asset,
              fastgltf::Primitive const& primitive) -> pbr::MeshBuilder::Primitive {
  auto const getAccessor = [&asset, &primitive](std::string_view attr) {
    auto const* const iter = primitive.findAttribute(constants::POSITION_NAME);
    if (iter != primitive.attributes.end()) {
      return asset.accessors.at(iter->accessorIndex);
    }
    throw std::runtime_error(std::format("Primitive does not have {} attribute", attr));
  };

  auto const positionAcc = getAccessor(constants::POSITION_NAME);
  auto const normalAcc = getAccessor(constants::NORMAL_NAME);

  std::vector<pbr::MeshVertex> vertices(positionAcc.count);

  fastgltf::iterateAccessorWithIndex<glm::vec3>(
      asset, positionAcc,
      [&vertices](glm::vec3 pos, std::size_t idx) { vertices.at(idx).position = pos; });
  fastgltf::iterateAccessorWithIndex<glm::vec3>(
      asset, normalAcc,
      [&vertices](glm::vec3 nor, std::size_t idx) { vertices.at(idx).normal = nor; });

  assert(primitive.indicesAccessor.has_value());
  auto const indexAcc = asset.accessors.at(primitive.indicesAccessor.value());

  std::vector<std::uint16_t> indices;
  indices.reserve(indexAcc.count);

  fastgltf::iterateAccessor<std::uint16_t>(
      asset, indexAcc, [&indices](std::size_t idx) { indices.push_back(idx); });

  return {
      .vertices = std::move(vertices),
      .indices = std::move(indices),
  };
}
} // namespace

pbr::gltf::Asset::Asset(fastgltf::GltfDataBuffer dataBuffer,
                        fastgltf::Asset asset) noexcept
    : _dataBuffer(std::move(dataBuffer)), _asset(std::move(asset)) {}

auto pbr::gltf::Asset::buildMesh(std::size_t index) -> MeshBuilder::BuiltMesh {
  auto const& mesh = _asset.meshes.at(index);
  MeshBuilder meshBuilder;

  for (auto const& primitive : mesh.primitives) {
    meshBuilder.addPrimitive(::loadPrimitive(_asset, primitive));
  }

  return meshBuilder.build();
}
