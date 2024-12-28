#include "pbr/SwapchainImageView.hpp"

#include "pbr/Vulkan.hpp"

#include <tuple>

pbr::SwapchainImageView::SwapchainImageView(core::Swapchain const& swapchain, std::uint32_t imageIndex)
    : _swapchain(&swapchain), _imageIndex(imageIndex) {}

auto pbr::SwapchainImageView::present(vk::Semaphore waitSemaphore) -> void {
  auto const swapchain = _swapchain->getSwapchain();
  std::ignore = _swapchain->getGpuHandle()->getQueue().presentKHR(
      vk::PresentInfoKHR {}
          .setWaitSemaphores(waitSemaphore)
          .setSwapchains(swapchain)
          .setImageIndices(_imageIndex));
}
