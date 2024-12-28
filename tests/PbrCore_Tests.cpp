#include <catch2/catch_test_macros.hpp>

#include "pbr/Vulkan.hpp"
#include "pbr/utils/Conversions.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/core/Swapchain.hpp"

#include "vkfw/vkfw.hpp"

#include <chrono>

TEST_CASE("Creating GpuHandle", "[pbr::core]") {
  auto const vkfw = vkfw::initUnique({
      .platform = vkfw::Platform::eX11,
  });
  auto const window = vkfw::createWindowUnique(1, 1, "");

  auto const gpuHandle = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });
}

TEST_CASE("Core tests", "[pbr::core]") {
  auto const vkfw = vkfw::initUnique({
      .platform = vkfw::Platform::eX11,
  });
  auto const window = vkfw::createWindowUnique(1, 1, "");

  auto const gpuHandle = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });

  SECTION("Swapchain") {
    auto const surface =
        vkfw::createWindowSurfaceUnique(gpuHandle->getInstance(), window.get());
    auto const extent = pbr::utils::toExtent(window->getFramebufferSize());
    pbr::core::Swapchain swapchain(gpuHandle, surface.get(), extent);

    swapchain.resize(surface.get(), extent);

    // Pass a semaphore into acquire next image to avoid validation errors
    auto const semaphore = gpuHandle->getDevice().createSemaphoreUnique({});
    auto const [result, index] =
        swapchain.acquireImageIndex(semaphore.get(), nullptr, std::chrono::nanoseconds(0));
    REQUIRE(result == vk::Result::eSuccess);

    auto const image = swapchain.getImage(index);
    REQUIRE(image != nullptr);
    auto const imageView = swapchain.getImageView(index);
    REQUIRE(imageView != nullptr);
  }
}
