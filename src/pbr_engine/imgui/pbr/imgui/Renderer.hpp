#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/Buffer.hpp"
#include "pbr/Image2D.hpp"
#include "pbr/imgui/Pipeline.hpp"
#include "pbr/memory/IAllocator.hpp"

#include "imgui.h"

#include <memory>
#include <optional>

namespace pbr::imgui {
/**
 * Contains all the resources to render imgui.
 */
class Renderer {
  struct ImguiBuffer {
    Buffer buffer;
    vk::DeviceSize size;
  };

  core::SharedGpuHandle _gpu;
  std::shared_ptr<IAllocator> _allocator;

  pbr::Image2D _fontImage;
  vk::UniqueSampler _fontSampler;

  Pipeline _pipeline;

  vk::UniqueDescriptorPool _descPool;
  vk::DescriptorSet _descSet;

  std::optional<ImguiBuffer> _vertexBuffer = std::nullopt;
  std::optional<ImguiBuffer> _indexBuffer = std::nullopt;

public:
  /**
   * Synchronously initializes the renderer.
   *
   * @param gpu The gpu to use for the renderer.
   * @param allocator The allocator to use for the font image and vertex and index
   * buffers.
   * @param cmdPool The command pool to use for the command buffer that will stage the
   * font image copy.
   * @param info The info for the imgui render pipeline.
   *
   * @note This constructor blocks until font image gets copied on the gpu.
   */
  Renderer(core::SharedGpuHandle gpu, std::shared_ptr<IAllocator> allocator,
           vk::CommandPool cmdPool, PipelineCreateInfo info);

  /**
   * Records imgui render commands to the provided cmdBuffer.
   */
  auto render(vk::CommandBuffer cmdBuffer) -> void;

private:
  /**
   * Updates the contained vertex and index buffers with the provided data.
   */
  auto updateBuffers(ImDrawData const* data) -> void;
};
} // namespace pbr::imgui
