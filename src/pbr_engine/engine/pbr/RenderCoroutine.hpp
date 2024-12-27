#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include <cassert>
#include <coroutine>
#include <exception>
#include <optional>
#include <type_traits>

namespace pbr {
struct FrameData {
  core::SharedGpuHandle gpu;
  vk::Fence fence;
  vk::CommandBuffer cmdBuffer;
};
class FenceAwaiter {
  vk::Fence _fence;
  core::SharedGpuHandle _gpu;

public:
  constexpr FenceAwaiter(vk::Fence fence, core::SharedGpuHandle gpu);

  FenceAwaiter(const FenceAwaiter&) = delete;
  auto operator=(const FenceAwaiter&) -> FenceAwaiter& = delete;

  FenceAwaiter(FenceAwaiter&&) = default;
  auto operator=(FenceAwaiter&&) -> FenceAwaiter& = default;

  ~FenceAwaiter() = default;

  [[nodiscard]]
  auto await_ready() const -> bool;
  auto await_suspend(std::coroutine_handle<> caller) const noexcept -> void;
  auto await_resume() -> void;
};
class RenderCoroutine {
public:
  struct promise_type {
    std::exception_ptr exception {};
    core::SharedGpuHandle gpu {};
    vk::UniqueFence fence {};
    vk::UniqueCommandBuffer cmdBuffer {};

    [[nodiscard]]
    auto get_return_object() -> RenderCoroutine;

    [[nodiscard]]
    constexpr auto initial_suspend() const noexcept -> std::suspend_always;
    [[nodiscard]]
    constexpr auto final_suspend() const noexcept -> std::suspend_always;

    constexpr auto return_void() const noexcept -> void {}
    constexpr auto unhandled_exception() noexcept -> void;
  };
  using CoroHandle = std::coroutine_handle<promise_type>;

private:
  CoroHandle _handle;

public:
  constexpr explicit RenderCoroutine(CoroHandle handle);

  RenderCoroutine(const RenderCoroutine&) = delete;
  auto operator=(const RenderCoroutine&) -> RenderCoroutine& = delete;

  RenderCoroutine(RenderCoroutine&&) = default;
  auto operator=(RenderCoroutine&&) -> RenderCoroutine& = default;

  ~RenderCoroutine() noexcept;

  auto initializePromise(core::SharedGpuHandle gpu,
                         vk::UniqueCommandBuffer cmdBuffer) -> void;

  auto render() -> void;
  auto flush() -> void;
};
class FrameDataAwaiter {
  std::coroutine_handle<RenderCoroutine::promise_type> _handle {};

public:
  FrameDataAwaiter() = default;

  [[nodiscard]]
  constexpr auto await_ready() const noexcept -> std::false_type;
  constexpr auto await_suspend(
      std::coroutine_handle<RenderCoroutine::promise_type> handle) noexcept -> void;
  constexpr auto await_resume() -> std::optional<FrameData>;
};

[[nodiscard]]
auto renderFrameAsync() -> RenderCoroutine;
[[nodiscard]]
auto createRenderCoroutine(core::SharedGpuHandle gpu,
                           vk::UniqueCommandBuffer cmdBuffer) -> RenderCoroutine;
} // namespace pbr

/* IMPLEMENTATIONS */

constexpr pbr::FenceAwaiter::FenceAwaiter(vk::Fence fence, core::SharedGpuHandle gpu)
    : _fence(fence), _gpu(std::move(gpu)) {}

constexpr pbr::RenderCoroutine::RenderCoroutine(CoroHandle handle) : _handle(handle) {}

// NOLINTNEXTLINE, dont make this static
constexpr auto pbr::RenderCoroutine::promise_type::initial_suspend() const noexcept
    -> std::suspend_always {
  return {};
}

// NOLINTNEXTLINE, dont make this static
constexpr auto pbr::RenderCoroutine::promise_type::final_suspend() const noexcept
    -> std::suspend_always {
  return {};
}

constexpr auto
pbr::RenderCoroutine::promise_type::unhandled_exception() noexcept -> void {
  exception = std::current_exception();
}

// NOLINTNEXTLINE, dont make this static
constexpr auto pbr::FrameDataAwaiter::await_ready() const noexcept -> std::false_type {
  return {};
}

constexpr auto pbr::FrameDataAwaiter::await_suspend(
    std::coroutine_handle<RenderCoroutine::promise_type> handle) noexcept -> void {
  _handle = handle;
}

constexpr auto pbr::FrameDataAwaiter::await_resume() -> std::optional<FrameData> {
  assert(_handle);
  auto& promise = _handle.promise();
  if (promise.gpu != nullptr && promise.fence && promise.cmdBuffer) {
    return std::make_optional(FrameData {
        .gpu = promise.gpu,
        .fence = promise.fence.get(),
        .cmdBuffer = promise.cmdBuffer.get(),
    });
  } else {
    return std::nullopt;
  }
}
