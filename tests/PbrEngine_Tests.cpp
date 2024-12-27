#include <catch2/catch_test_macros.hpp>

#include "pbr/utils/Conversions.hpp"
#include "pbr/core/GpuHandle.hpp"

#include "pbr/Surface.hpp"

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

  pbr::Surface const surface(
      gpu, vkfw::createWindowSurfaceUnique(gpu->getInstance(), window.get()),
      pbr::utils::toExtent(window->getFramebufferSize()));
}
