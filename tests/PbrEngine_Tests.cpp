#include <catch2/catch_test_macros.hpp>

#include "pbr/utils/Conversions.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/AsyncSubmitter.hpp"
#include "pbr/Surface.hpp"

#include "vkfw/vkfw.hpp"

#include <utility>

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

TEST_CASE("Async submit", "[pbr]") {
  [[maybe_unused]]
  auto const vkfw = vkfw::initUnique({.platform = vkfw::Platform::eX11});

  auto const gpu = pbr::core::makeGpuHandle({
      .extensions = vkfw::getRequiredInstanceExtensions(),
      .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
      .enableValidation = true,
  });

  auto const commandPool = gpu->getDevice().createCommandPoolUnique({
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = gpu->getPhysicalDeviceProperties().graphicsTransferPresentQueue,
  });
  auto cmdBuffers = gpu->getDevice().allocateCommandBuffersUnique({
      .commandPool = commandPool.get(),
      .commandBufferCount = 1,
  });
  auto const cmdBuffer = cmdBuffers.front().get();
  cmdBuffer.begin(vk::CommandBufferBeginInfo{});
  cmdBuffer.end();

  pbr::AsyncSubmitter submitter(gpu);
  pbr::AsyncSubmitInfo submitInfo {
    .cmdBuffer = std::move(cmdBuffers.front())
  };
  submitter.submit(std::move(submitInfo));
  REQUIRE(submitter.isSubmitted());

  submitInfo = submitter.wait();

  REQUIRE(submitInfo.cmdBuffer.get() == cmdBuffer);
}
