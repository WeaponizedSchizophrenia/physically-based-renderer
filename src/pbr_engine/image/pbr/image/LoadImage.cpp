#include "pbr/image/LoadImage.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/Image2D.hpp"
#include "pbr/TransferStager.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "stb/stb_image.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <span>
#include <utility>
#include <vector>

namespace {
[[nodiscard]]
constexpr auto stageImage2D(pbr::core::GpuHandle const& gpu, pbr::TransferStager& stager,
                            std::uint8_t* const image, int width,
                            int height) -> pbr::Image2D {
  std::vector<std::byte> pixels(static_cast<std::size_t>(width * height * 4));
  std::memcpy(pixels.data(), image, pixels.size());

  auto const format = vk::Format::eR8G8B8A8Unorm;
  auto const aspect = vk::ImageAspectFlagBits::eColor;
  auto image2D = stager.addTransfer(std::move(pixels),
                                    vk::ImageCreateInfo {
                                        .imageType = vk::ImageType::e2D,
                                        .format = format,
                                        .extent {
                                            .width = static_cast<std::uint32_t>(width),
                                            .height = static_cast<std::uint32_t>(height),
                                            .depth = 1,
                                        },
                                        .mipLevels = 1,
                                        .arrayLayers = 1,
                                        .usage = vk::ImageUsageFlagBits::eSampled,
                                    },
                                    aspect, vk::PipelineStageFlagBits2::eFragmentShader,
                                    vk::AccessFlagBits2::eShaderRead);

  stbi_image_free(image);

  return {
      gpu,
      format,
      aspect,
      std::move(image2D),
  };
}
} // namespace

auto pbr::image::loadImage2D(core::GpuHandle const& gpu, TransferStager& stager,
                             std::filesystem::path const& path) -> Image2D {
  int width {};
  int height {};
  int channels {};
  auto* const image = stbi_load(path.c_str(), &width, &height, &channels, 4);
  return ::stageImage2D(gpu, stager, image, width, height);
}

auto pbr::image::loadImage2D(core::GpuHandle const& gpu, TransferStager& stager,
                             std::span<std::uint8_t const> buffer) -> Image2D {
  int width {};
  int height {};
  int channels {};
  auto* const image = stbi_load_from_memory(
      buffer.data(), static_cast<int>(buffer.size()), &width, &height, &channels, 4);
  return ::stageImage2D(gpu, stager, image, width, height);
}
