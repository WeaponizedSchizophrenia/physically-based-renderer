#include "pbr/RenderCoroutine.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include <cassert>
#include <coroutine>
#include <cstdint>
#include <limits>
#include <utility>

auto pbr::FenceAwaiter::await_ready() const -> bool {
  return _gpu->getDevice().waitForFences(_fence, vk::False, 0) == vk::Result::eSuccess;
}

auto pbr::FenceAwaiter::await_suspend(std::coroutine_handle<> /*unused*/) const noexcept
    -> void {}

auto pbr::FenceAwaiter::await_resume() -> void {
  assert(_gpu->getDevice().waitForFences(_fence, vk::False,
                                         std::numeric_limits<std::uint64_t>::max())
         == vk::Result::eSuccess);
  _gpu->getDevice().resetFences(_fence);
}

auto pbr::RenderCoroutine::promise_type::get_return_object() -> RenderCoroutine {
  return RenderCoroutine(RenderCoroutine::CoroHandle::from_promise(*this));
}

pbr::RenderCoroutine::~RenderCoroutine() noexcept {
  if (_handle) {
    _handle.destroy();
  }
}

auto pbr::RenderCoroutine::initializePromise(core::SharedGpuHandle gpu,
                                             vk::UniqueCommandBuffer cmdBuffer) -> void {
  assert(_handle);

  auto& promise = _handle.promise();
  promise.fence =
      gpu->getDevice().createFenceUnique({.flags = vk::FenceCreateFlagBits::eSignaled});
  promise.cmdBuffer = std::move(cmdBuffer);

  promise.gpu = std::move(gpu);
}

auto pbr::RenderCoroutine::render() -> void {
  assert(_handle);

  _handle.resume();
}

auto pbr::RenderCoroutine::flush() -> void {
  assert(_handle);

  auto& promise = _handle.promise();
  assert(promise.gpu->getDevice().waitForFences(promise.fence.get(), vk::False,
                                                std::numeric_limits<std::uint64_t>::max())
         == vk::Result::eSuccess);
}

auto pbr::renderFrameAsync() -> RenderCoroutine {
  auto frameData = co_await FrameDataAwaiter {};
  if (!frameData.has_value()) {
    co_return;
  }

  while (true) {
    co_await FenceAwaiter(frameData->fence, frameData->gpu);

    frameData->cmdBuffer.reset();
    frameData->cmdBuffer.begin(vk::CommandBufferBeginInfo {
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    frameData->cmdBuffer.end();

    frameData->gpu->getQueue().submit(
        vk::SubmitInfo {}.setCommandBuffers(frameData->cmdBuffer), frameData->fence);
  }
}

auto pbr::createRenderCoroutine(core::SharedGpuHandle gpu,
                                vk::UniqueCommandBuffer cmdBuffer) -> RenderCoroutine {
  auto coro = renderFrameAsync();
  coro.initializePromise(std::move(gpu), std::move(cmdBuffer));
  return coro;
}
