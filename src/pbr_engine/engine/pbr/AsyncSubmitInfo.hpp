#pragma once

#include "pbr/Vulkan.hpp"
#include <optional>

namespace pbr {
// TODO: Allow batching
/**
 * Describes the async submit.
 */
struct AsyncSubmitInfo {
  /**
   * Describes a wait semaphore.
   */
  struct WaitSemaphore {
    vk::UniqueSemaphore semaphore;
    vk::PipelineStageFlags waitDstStageMask;
  };
  /// An optional semaphore to wait on before starting to execute the command buffers.
  std::optional<WaitSemaphore> waitSemaphore = std::nullopt;
  /// An optional semaphore to signal to signal after executing the command buffers.
  std::optional<vk::UniqueSemaphore> signalSemaphore = std::nullopt;
  /// The command buffers to execute.
  vk::UniqueCommandBuffer cmdBuffer;
};
} // namespace pbr
