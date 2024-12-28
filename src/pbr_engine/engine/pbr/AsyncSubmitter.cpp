#include "pbr/AsyncSubmitter.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include <cstdint>
#include <limits>
#include <optional>
#include <utility>

pbr::AsyncSubmitter::AsyncSubmitter(core::SharedGpuHandle gpu)
    : _gpu(std::move(gpu))
    , _fence(_gpu->getDevice().createFenceUnique(
          {.flags = vk::FenceCreateFlagBits::eSignaled})) {}

pbr::AsyncSubmitter::~AsyncSubmitter() noexcept {
  if(isSubmitted()) {
    wait();
  }
}

auto pbr::AsyncSubmitter::submit(AsyncSubmitInfo info) -> void {
  if (_submittedInfo.has_value()) {
    wait();
  }
  _gpu->getDevice().resetFences(_fence.get());

  _submittedInfo.emplace(std::move(info));

  auto submitInfo = vk::SubmitInfo {}.setCommandBuffers(_submittedInfo->cmdBuffer.get());
  if (_submittedInfo->waitSemaphore.has_value()) {
    submitInfo.setWaitSemaphores(_submittedInfo->waitSemaphore->semaphore.get());
    submitInfo.setWaitDstStageMask(_submittedInfo->waitSemaphore->waitDstStageMask);
  }
  if (_submittedInfo->signalSemaphore.has_value()) {
    submitInfo.setSignalSemaphores(_submittedInfo->signalSemaphore->get());
  }

  _gpu->getQueue().submit(submitInfo, _fence);
}

auto pbr::AsyncSubmitter::isSubmitted() const noexcept -> bool {
  return _submittedInfo.has_value();
}

auto pbr::AsyncSubmitter::isExecuting() const -> bool {
  if (!_submittedInfo.has_value()) {
    return false;
  } else {
    return _gpu->getDevice().waitForFences(_fence.get(), vk::False, 0)
           != vk::Result::eSuccess;
  }
}

auto pbr::AsyncSubmitter::wait() -> AsyncSubmitInfo {
  assert(_submittedInfo.has_value());
  assert(_gpu->getDevice().waitForFences(_fence.get(), vk::False,
                                         std::numeric_limits<std::uint64_t>::max())
         == vk::Result::eSuccess);

  return *std::exchange(_submittedInfo, std::nullopt);
}

auto pbr::AsyncSubmitter::wait(std::chrono::nanoseconds const timeout) -> std::optional<AsyncSubmitInfo> {
  assert(_submittedInfo.has_value());

  auto const result = _gpu->getDevice().waitForFences(_fence.get(), vk::False, timeout.count());
  if(result == vk::Result::eSuccess) {
    return std::exchange(_submittedInfo, std::nullopt);
  } else {
    return std::nullopt;
  }
}
