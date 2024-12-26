#include <catch2/catch_test_macros.hpp>

#include "vkfw/vkfw.hpp"

#include "pbr/core/GpuHandle.hpp"

TEST_CASE("Creating GpuHandle", "[pbr::core]") {
  auto vkfw = vkfw::initUnique({
    .platform = vkfw::Platform::eX11,
  });
  auto window = vkfw::createWindowUnique(1, 1, "");

  auto const gpuHandle = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });
}
