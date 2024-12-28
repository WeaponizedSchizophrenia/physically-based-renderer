#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/core/GpuHandle.hpp"

#include <chrono>
#include <optional>

namespace pbr {
/**
 * Type for managing the lifetime of data submitted to the GPU.
 * @note On destruction this type blocks untill the GPU is done processing the submitted
 * data.
 */
class AsyncSubmitter {
  core::SharedGpuHandle _gpu;
  vk::UniqueFence _fence;
  std::optional<AsyncSubmitInfo> _submittedInfo = std::nullopt;

public:
  explicit AsyncSubmitter(core::SharedGpuHandle gpu);

  AsyncSubmitter(const AsyncSubmitter&) = delete;
  auto operator=(const AsyncSubmitter&) -> AsyncSubmitter& = delete;

  AsyncSubmitter(AsyncSubmitter&&) = default;
  auto operator=(AsyncSubmitter&&) -> AsyncSubmitter& = default;

  ~AsyncSubmitter() noexcept;

  /**
   * Submits the provided data to the queue.
   * This function takes ownership of the provided info, to get back the ownership of the
   * data call wait().
   */
  auto submit(AsyncSubmitInfo info) -> void;
  /**
   * @returns true if there is something submitted.
   */
  [[nodiscard]]
  auto isSubmitted() const noexcept -> bool;
  /**
   * Checks if the submitted command buffers are still being executed.
   * @returns false if there was nothing submitted or if the execution is complete.
   */
  [[nodiscard]]
  auto isExecuting() const -> bool;
  /**
   * Waits for the execution to be complete.
   * @returns the ownership of the submitted info.
   * @note This should never be called if there is nothing submitted.
   */
  auto wait() -> AsyncSubmitInfo;
  /**
   * Waits for the execution to be complete with a timeout.
   * @param timeout The timeout that will be passed to vkWaitForFences.
   * @returns the ownership of AsyncSubmittedInfo if it completes before the timeout,
   * std::nullopt if it timees out.
   */
  auto wait(std::chrono::nanoseconds timeout) -> std::optional<AsyncSubmitInfo>;
};
} // namespace pbr
