#pragma once

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/TransferStager.hpp"

#include <cstdint>
#include <filesystem>
#include <span>

namespace pbr::image {
[[nodiscard]]
auto loadImage2D(core::GpuHandle const& gpu, TransferStager& stager,
                 std::filesystem::path const& path) -> Image2D;
[[nodiscard]]
auto loadImage2D(core::GpuHandle const& gpu, TransferStager& stager, std::span<std::uint8_t const> buffer) -> Image2D;
}
