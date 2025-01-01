#pragma once

#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/imgui/Pipeline.hpp"
#include "pbr/memory/IAllocator.hpp"
#include "pbr/Buffer.hpp"
#include "pbr/Image.hpp"

#include "imgui.h"

#include <memory>
#include <optional>

namespace pbr::imgui {
class Renderer {
  struct ImguiBuffer {
    Buffer buffer;
    vk::DeviceSize size;
  };

  core::SharedGpuHandle _gpu;
  std::shared_ptr<IAllocator> _allocator;

  pbr::Image _fontImage;
  vk::UniqueImageView _fontImageView;
  vk::UniqueSampler _fontSampler;

  Pipeline _pipeline;

  vk::UniqueDescriptorPool _descPool;
  vk::DescriptorSet _descSet;

  std::optional<ImguiBuffer> _vertexBuffer = std::nullopt;
  std::optional<ImguiBuffer> _indexBuffer = std::nullopt; 

public:
  Renderer(core::SharedGpuHandle gpu, std::shared_ptr<IAllocator> allocator,
                    vk::CommandPool cmdPool, PipelineCreateInfo info);

  auto render(vk::CommandBuffer cmdBuffer) -> void;

private:
  auto updateBuffers(ImDrawData const* data) -> void;
};
} // namespace pbr::imgui
