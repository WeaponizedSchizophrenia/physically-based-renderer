#include <catch2/catch_test_macros.hpp>

#include "pbr/Surface.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "vkfw/vkfw.hpp"

TEST_CASE("Surface creation", "[pbr]") {
  [[maybe_unused]]
  auto const vkfw = vkfw::initUnique({.platform = vkfw::Platform::eX11});
  auto const window = vkfw::createWindowUnique(1, 1, "");

  auto const gpu = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });

  auto const [fbWidth, fbHeight] = window->getFramebufferSize();
  pbr::Surface const surface(
      gpu, vkfw::createWindowSurfaceUnique(gpu->getInstance(), window.get()),
      {
          .width = static_cast<std::uint32_t>(fbWidth),
          .height = static_cast<std::uint32_t>(fbHeight),
      });
}
