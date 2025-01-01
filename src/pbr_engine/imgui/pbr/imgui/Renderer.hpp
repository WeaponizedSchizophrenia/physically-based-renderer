#pragma once

#include "pbr/Image.hpp"
#include "pbr/Vulkan.hpp"

#include "pbr/core/GpuHandle.hpp"
#include "pbr/imgui/Pipeline.hpp"
#include "pbr/memory/IAllocator.hpp"
#include <memory>

namespace pbr::imgui {
class Renderer {
  core::SharedGpuHandle _gpu;

  pbr::Image _fontImage;
  vk::UniqueImageView _fontImageView;
  vk::UniqueSampler _fontSampler;

  Pipeline _pipeline;

  vk::UniqueDescriptorPool _descPool;
  vk::DescriptorSet _descSet;

public:
  Renderer(core::SharedGpuHandle gpu, std::shared_ptr<IAllocator> allocator,
                    vk::CommandPool cmdPool, PipelineCreateInfo info);
};
} // namespace pbr::imgui
