#include "pbr/core/Swapchain.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdint>
#include <limits>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

namespace constants {
constexpr static vk::SurfaceFormatKHR PREFERRED_FORMAT {
    .format = vk::Format::eB8G8R8A8Srgb,
    .colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear,
};
constexpr static auto PREFERRED_PRESENT_MODE = vk::PresentModeKHR::eMailbox;
} // namespace constants

namespace {
[[nodiscard]]
constexpr auto chooseFormat(std::span<vk::SurfaceFormatKHR const> const formats)
    -> vk::SurfaceFormatKHR {
  assert(!formats.empty());
  auto const iter = std::ranges::find(formats, constants::PREFERRED_FORMAT);
  return iter != formats.end() ? *iter : formats.front();
}
[[nodiscard]]
constexpr auto choosePresentMode(std::span<vk::PresentModeKHR const> const presentModes)
    -> vk::PresentModeKHR {
  auto const iter = std::ranges::find(presentModes, constants::PREFERRED_PRESENT_MODE);
  return iter != presentModes.end() ? *iter : vk::PresentModeKHR::eFifo;
}
} // namespace

pbr::core::Swapchain::Swapchain(SharedGpuHandle gpu, vk::SurfaceKHR const surface,
                                vk::Extent2D const extent)
    : _gpu(std::move(gpu))
    , _format(::chooseFormat(_gpu->getPhysicalDevice().getSurfaceFormatsKHR(surface)))
    , _presentMode(::choosePresentMode(
          _gpu->getPhysicalDevice().getSurfacePresentModesKHR(surface)))
    , _extent(extent)
    , _swapchain(_gpu->getDevice().createSwapchainKHRUnique(getSwapchainCreateInfo(
          surface, _gpu->getPhysicalDevice().getSurfaceCapabilitiesKHR(surface))))
    , _images(_gpu->getDevice().getSwapchainImagesKHR(_swapchain)) {
  initializeViews();
}

auto pbr::core::Swapchain::resize(vk::SurfaceKHR const surface,
                                  vk::Extent2D const extent) -> void {
  _format = ::chooseFormat(_gpu->getPhysicalDevice().getSurfaceFormatsKHR(surface));
  _presentMode =
      ::choosePresentMode(_gpu->getPhysicalDevice().getSurfacePresentModesKHR(surface));
  _extent = extent;

  _swapchain = _gpu->getDevice().createSwapchainKHRUnique(getSwapchainCreateInfo(
      surface, _gpu->getPhysicalDevice().getSurfaceCapabilitiesKHR(surface)));
  _images = _gpu->getDevice().getSwapchainImagesKHR(_swapchain);

  initializeViews();
}

auto pbr::core::Swapchain::acquireImageIndex(
    vk::Semaphore const semaphore, vk::Fence const fence,
    std::optional<std::chrono::nanoseconds> const timeout)
    -> vk::ResultValue<std::uint32_t> {
  return _gpu->getDevice().acquireNextImageKHR(
      _swapchain,
      timeout.transform(&std::chrono::nanoseconds::count)
          .value_or(std::numeric_limits<std::uint64_t>::max()),
      semaphore, fence);
}

auto pbr::core::Swapchain::operator[](std::size_t const index) const
    -> std::pair<vk::Image, vk::ImageView> {
  return std::make_pair(_images.at(index), _views.at(index).get());
}

auto pbr::core::Swapchain::initializeViews() -> void {
  _views = _images | std::views::transform([this](vk::Image image) {
             return _gpu->getDevice().createImageViewUnique({
                 .image = image,
                 .viewType = vk::ImageViewType::e2D,
                 .format = _format.format,
                 .subresourceRange {
                     .aspectMask = vk::ImageAspectFlagBits::eColor,
                     .levelCount = 1,
                     .layerCount = 1,
                 },
             });
           })
           | std::ranges::to<std::vector>();
}

auto pbr::core::Swapchain::getSwapchainCreateInfo(
    vk::SurfaceKHR const surface, vk::SurfaceCapabilitiesKHR const capabilities)
    const noexcept -> vk::SwapchainCreateInfoKHR {
  return {
      .surface = surface,
      .minImageCount = capabilities.minImageCount,
      .imageFormat = _format.format,
      .imageColorSpace = _format.colorSpace,
      .imageExtent = _extent,
      .imageArrayLayers = 1,
      .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
      .preTransform = capabilities.currentTransform,
      .presentMode = _presentMode,
  };
}
