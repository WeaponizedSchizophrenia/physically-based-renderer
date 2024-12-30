#pragma once

#include "pbr/Vulkan.hpp"

#include "vkfw/vkfw.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/AsyncSubmitter.hpp"
#include "pbr/Buffer.hpp"
#include "pbr/PbrPipeline.hpp"
#include "pbr/Surface.hpp"
#include "pbr/SwapchainImageView.hpp"
#include "pbr/memory/IAllocator.hpp"

#include <filesystem>
#include <memory>

namespace app {
class App {
  std::filesystem::path _path;
  vkfw::UniqueWindow _window;

  pbr::core::SharedGpuHandle _gpu;
  std::shared_ptr<pbr::IAllocator> _allocator;
  pbr::Surface _surface;

  vk::UniqueCommandPool _commandPool;

  pbr::PbrPipeline _pbrPipeline;

  pbr::Buffer _vertexBuffer;

  // Frame data
  pbr::AsyncSubmitter _submitter;

public:
  explicit App(std::filesystem::path path);

  App(const App&) = delete;
  auto operator=(const App&) -> App& = delete;
  App(App&&) = default;
  auto operator=(App&&) -> App& = default;

  ~App() noexcept;

  auto run() -> void;

private:
  auto makeAsyncSubmitInfo() -> pbr::AsyncSubmitInfo;
  auto recordCommands(vk::CommandBuffer, pbr::SwapchainImageView) -> void;
  auto renderAndPresent() -> void;
};
} // namespace app
