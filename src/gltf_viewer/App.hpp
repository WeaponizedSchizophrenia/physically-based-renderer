#pragma once

#include "AppUi.hpp"
#include "pbr/Scene.hpp"
#include "pbr/Vulkan.hpp"

#include "vkfw/vkfw.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/AsyncSubmitter.hpp"
#include "pbr/PbrPipeline.hpp"
#include "pbr/Surface.hpp"
#include "pbr/SwapchainImageView.hpp"
#include "pbr/imgui/Renderer.hpp"
#include "pbr/memory/IAllocator.hpp"

#include "CameraController.hpp"

#include <filesystem>
#include <memory>

#include <spdlog/logger.h>

namespace app {
class App {
  std::shared_ptr<spdlog::logger> _logger;

  std::filesystem::path _path;
  vkfw::UniqueWindow _window;

  app::CameraController _controller;

  pbr::core::SharedGpuHandle _gpu;
  std::shared_ptr<pbr::IAllocator> _allocator;
  pbr::Surface _surface;

  vk::UniqueCommandPool _commandPool;
  vk::UniqueDescriptorPool _descPool;

  pbr::imgui::Renderer _imguiRenderer;
  AppUi _ui;

  pbr::PbrPipeline _pbrPipeline;

  pbr::Scene _scene;

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
  auto setupWindowCallbacks() -> void;
  auto makeAsyncSubmitInfo() -> pbr::AsyncSubmitInfo;
  auto recordCommands(vk::CommandBuffer, pbr::SwapchainImageView) -> void;
  auto renderAndPresent() -> void;
};
} // namespace app
