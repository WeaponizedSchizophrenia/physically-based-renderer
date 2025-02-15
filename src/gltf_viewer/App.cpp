#include "App.hpp"

#include "pbr/TonemapperSystem.hpp"
#include "pbr/Vulkan.hpp"

#include "vkfw/vkfw.hpp"

#include "pbr/utils/Conversions.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/GBuffer.hpp"
#include "pbr/PbrPipeline.hpp"
#include "pbr/PbrRenderSystem.hpp"
#include "pbr/Scene.hpp"
#include "pbr/SwapchainImageView.hpp"
#include "pbr/TransferStager.hpp"
#include "pbr/gltf/Asset.hpp"
#include "pbr/gltf/Loader.hpp"
#include "pbr/imgui/Pipeline.hpp"
#include "pbr/imgui/Renderer.hpp"
#include "pbr/memory/IAllocator.hpp"
#include "pbr/memory/MemoryAllocator.hpp"

#include "CameraController.hpp"

#include "backends/imgui_impl_glfw.h"
#include "imgui.h"

#include <array>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/ext/vector_float3.hpp>

namespace constants {
constexpr static auto DEFAULT_WINDOW_WIDTH = 1280uz;
constexpr static auto DEFAULT_WINDOW_HEIGHT = 720uz;
} // namespace constants

namespace {
[[nodiscard]]
constexpr auto loadBinary(std::filesystem::path const& path)
    -> std::vector<std::uint32_t> {
  std::ifstream file(path, std::ios::in | std::ios::binary);
  auto const size = std::filesystem::file_size(path);
  std::vector<std::uint32_t> binary(size / 4);

  // NOLINTNEXTLINE casting to char* is not UB
  file.read(reinterpret_cast<char*>(binary.data()), static_cast<std::streamsize>(size));

  return binary;
}
[[nodiscard]]
constexpr auto createLogger() -> std::shared_ptr<spdlog::logger> {
  return spdlog::stdout_color_mt("app");
}
[[nodiscard]]
constexpr auto validatePath(std::filesystem::path path) -> std::filesystem::path {
  if (!std::filesystem::exists(path)) {
    throw std::runtime_error(std::format("No such file: {}", path.c_str()));
  }
  if (!std::filesystem::is_regular_file(path)) {
    throw std::runtime_error(std::format("{} is not a regular file", path.c_str()));
  }
  return path;
}
[[nodiscard]]
constexpr auto loadShader(pbr::core::GpuHandle const& gpu, std::string_view name)
    -> vk::UniqueShaderModule {
  auto const spv = loadBinary(std::filesystem::path("assets/shaders/compiled") / name);
  return gpu.getDevice().createShaderModuleUnique(
      vk::ShaderModuleCreateInfo {}.setCode(spv));
}
struct ShaderNames {
  std::string_view vertexName {};
  std::string_view fragmentName {};
};
[[nodiscard]]
constexpr auto loadShaders(pbr::core::GpuHandle const& gpu, ShaderNames names)
    -> std::pair<vk::UniqueShaderModule, vk::UniqueShaderModule> {
  return std::make_pair(loadShader(gpu, names.vertexName),
                        loadShader(gpu, names.fragmentName));
}
[[nodiscard]]
constexpr auto createPbrPipeline(pbr::core::GpuHandle const& gpu, vk::Format outputFormat)
    -> pbr::PbrPipeline {
  auto const [vertexModule, fragmentModule] = loadShaders(
      gpu, {.vertexName = "pbr_vertex.spv", .fragmentName = "pbr_fragment.spv"});
  return {
      gpu,
      pbr::PbrPipelineCreateInfo {
          .vertexStage {
              .stage = vk::ShaderStageFlagBits::eVertex,
              .module = vertexModule.get(),
              .pName = "main",
          },
          .fragmentStage {
              .stage = vk::ShaderStageFlagBits::eFragment,
              .module = fragmentModule.get(),
              .pName = "main",
          },
          .outputFormat = outputFormat,
      },
  };
}
[[nodiscard]]
constexpr auto createPbrRenderSystem(pbr::core::SharedGpuHandle gpu)
    -> pbr::PbrRenderSystem {
  auto const [geometryVertex, geometryFragment] =
      loadShaders(*gpu, {.vertexName = "geometry_pass_vertex.spv",
                         .fragmentName = "geometry_pass_fragment.spv"});
  auto const [lightingVertex, lightingFragment] = loadShaders(
      *gpu, {.vertexName = "fullscreen_quad.spv", .fragmentName = "pbr_lighting.spv"});
  return {
      std::move(gpu),
      pbr::PbrRenderSystemCreateInfo {
          .geometryVertexShader {
              .stage = vk::ShaderStageFlagBits::eVertex,
              .module = geometryVertex.get(),
              .pName = "main",
          },
          .geometryFragmentShader {
              .stage = vk::ShaderStageFlagBits::eFragment,
              .module = geometryFragment.get(),
              .pName = "main",
          },
          .lightingVertexShader {
              .stage = vk::ShaderStageFlagBits::eVertex,
              .module = lightingVertex.get(),
              .pName = "main",
          },
          .lightingFragmentShader {
              .stage = vk::ShaderStageFlagBits::eFragment,
              .module = lightingFragment.get(),
              .pName = "main",
          },
      },
  };
}
[[nodiscard]]
constexpr auto loadScene(std::filesystem::path const& path,
                         pbr::gltf::AssetDependencies dependencies,
                         vk::CommandPool cmdPool, std::pmr::polymorphic_allocator<> alloc) -> pbr::Scene {
  pbr::TransferStager stager(dependencies.gpu, dependencies.allocator);

  pbr::gltf::Loader loader;
  auto asset = loader.loadAsset(path, std::move(dependencies));

  auto scene = asset.loadScene(stager, 0, alloc);

  stager.submit(cmdPool);
  stager.wait();

  return scene;
}
[[nodiscard]]
constexpr auto createImguiRenderer(pbr::core::SharedGpuHandle gpu,
                                   std::shared_ptr<pbr::IAllocator> allocator,
                                   vkfw::Window const& window, vk::CommandPool cmdPool,
                                   vk::Format outputFormat) -> pbr::imgui::Renderer {
  ImGui::CreateContext();
  ImGui_ImplGlfw_InitForOther(window, false);
  auto const [vertexModule, fragmentModule] = loadShaders(
      *gpu, {.vertexName = "imgui_vertex.spv", .fragmentName = "imgui_fragment.spv"});
  return {
      std::move(gpu),
      std::move(allocator),
      cmdPool,
      pbr::imgui::PipelineCreateInfo {
          .vertexStage {
              .stage = vk::ShaderStageFlagBits::eVertex,
              .module = vertexModule.get(),
              .pName = "main",
          },
          .fragmentStage {
              .stage = vk::ShaderStageFlagBits::eFragment,
              .module = fragmentModule.get(),
              .pName = "main",
          },
          .outputFormat = outputFormat,
      },
  };
}
[[nodiscard]]
constexpr auto createTonemapper(pbr::core::SharedGpuHandle gpu) -> pbr::TonemapperSystem {
  auto const shader = loadShader(*gpu, "tm_aces+gamma.spv");
  return {
      std::move(gpu),
      vk::PipelineShaderStageCreateInfo {
          .stage = vk::ShaderStageFlagBits::eCompute,
          .module = shader.get(),
          .pName = "main",
      },
  };
}
} // namespace

app::App::App(std::filesystem::path path, bool vkValidation)
    : _logger(::createLogger())
    , _path(::validatePath(std::move(path)))
    , _window(vkfw::createWindowUnique(constants::DEFAULT_WINDOW_WIDTH,
                                       constants::DEFAULT_WINDOW_HEIGHT, path.c_str()))
    , _controller(_window.get())
    , _gpu(pbr::core::makeGpuHandle({
          .extensions = vkfw::getRequiredInstanceExtensions(),
          .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
          .enableValidation = vkValidation,
      }))
    , _allocator(std::make_shared<pbr::MemoryAllocator>(_gpu))
    , _surface(_gpu, vkfw::createWindowSurfaceUnique(_gpu->getInstance(), _window.get()),
               pbr::utils::toExtent(_window->getFramebufferSize()))
    , _commandPool(_gpu->getDevice().createCommandPoolUnique({
          .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
          .queueFamilyIndex =
              _gpu->getPhysicalDeviceProperties().graphicsTransferPresentQueue,
      }))
    , _descPool([this] {
      std::array const sizes {
          vk::DescriptorPoolSize {
              .type = vk::DescriptorType::eUniformBuffer,
              .descriptorCount = 6,
          },
          vk::DescriptorPoolSize {
              .type = vk::DescriptorType::eSampledImage,
              .descriptorCount = 5,
          },
      };
      return _gpu->getDevice().createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo {
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
          .maxSets = 6,
      }
                                                              .setPoolSizes(sizes));
    }())
    , _imguiRenderer(::createImguiRenderer(_gpu, _allocator, _window.get(),
                                           _commandPool.get(),
                                           _surface.getFormat().format))
    , _pbrPipeline(::createPbrPipeline(*_gpu, _surface.getFormat().format))
    , _pbrSystem(::createPbrRenderSystem(_gpu))
    , _tonemapper(::createTonemapper(_gpu))
    , _sceneMemory()
    , _scene(::loadScene(
          _path,
          {
              .gpu = _gpu,
              .allocator = _allocator,
              .cameraAllocator {_gpu, _descPool.get(), _pbrPipeline.getCameraSetLayout()},
              .materialAllocator {_gpu, _descPool.get(),
                                  _pbrPipeline.getMaterialSetLayout()},
          },
          _commandPool.get(), &_sceneMemory))
    , _gBuffer(_pbrSystem.allocateGBuffer(
          *_allocator, pbr::utils::toExtent(_window->getFramebufferSize())))
    , _hdrImage(_tonemapper.allocateHdrImage(
          *_allocator, pbr::utils::toExtent(_window->getFramebufferSize())))
    , _submitter(_gpu) {
  setupWindowCallbacks();
  setupUi();

  _logger->info("Initialized app to view {}", _path.c_str());
  if (vkValidation) {
    _logger->info("Vulkan validation is enabled");
  }
}

auto app::App::run() -> void {
  auto lastFrame = std::chrono::high_resolution_clock::now();
  while (!_window->shouldClose()) {
    auto const thisFrame = std::chrono::high_resolution_clock::now();
    auto const frameDuration = thisFrame - lastFrame;
    auto const deltaTime =
        std::chrono::duration_cast<std::chrono::duration<double>>(frameDuration).count();
    lastFrame = thisFrame;

    vkfw::pollEvents();

    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    _ui.render(frameDuration);

    _controller.update(deltaTime);
    if (auto const camera = _scene.findCamera(); camera) {
      camera.value()->set(_controller.getCameraData());
    }
    for (auto& node : _scene.iterateAllNodes() | std::views::filter([](auto const& node) {
                        return node.getMesh() != nullptr;
                      }) | std::views::take(9)) {
      auto transform = node.getTransform();
      transform.rotation = glm::rotate(transform.rotation, static_cast<float>(deltaTime),
                                       {0.0f, 1.0f, 0.0f});
      node.setTransform(transform);
    }

    ImGui::Render();
    renderAndPresent();
  }
}

app::App::~App() noexcept {
  _gpu->getQueue().waitIdle();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

auto app::App::setupWindowCallbacks() -> void {
  _window->callbacks()->on_window_resize = [this](vkfw::Window const&, std::size_t width,
                                                  std::size_t height) {
    _surface.recreateSwapchain(pbr::utils::toExtent(width, height));
    _controller.onWindowResize(width, height);
  };
  _window->callbacks()->on_window_focus = [](vkfw::Window const& window, bool focus) {
    ImGui_ImplGlfw_WindowFocusCallback(window, static_cast<int>(focus));
  };
  _window->callbacks()->on_mouse_button = [](vkfw::Window const& window,
                                             vkfw::MouseButton button,
                                             vkfw::MouseButtonAction action,
                                             vkfw::ModifierKeyFlags const& mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, static_cast<int>(button),
                                       static_cast<int>(action), static_cast<int>(mods));
  };
  _window->callbacks()->on_cursor_move = [this](vkfw::Window const& window, double newX,
                                                double newY) {
    _controller.onCursorMove(newX, newY);
    ImGui_ImplGlfw_CursorPosCallback(window, newX, newY);
  };
  _window->callbacks()->on_scroll = [this](vkfw::Window const& window, double xOffset,
                                           double yOffset) {
    _controller.onScroll(xOffset, yOffset);
    ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
  };
  _window->callbacks()->on_key = [](vkfw::Window const& window, vkfw::Key key,
                                    std::int32_t scancode, vkfw::KeyAction action,
                                    vkfw::ModifierKeyFlags const& mods) {
    ImGui_ImplGlfw_KeyCallback(window, static_cast<int>(key), static_cast<int>(scancode),
                               static_cast<int>(action), static_cast<int>(mods));
  };
  _window->callbacks()->on_character = [](vkfw::Window const& window,
                                          std::uint32_t character) {
    ImGui_ImplGlfw_CharCallback(window, static_cast<unsigned int>(character));
  };
}

auto app::App::setupUi() -> void {
  _ui.sceneTree.setScene(&_scene);
}

auto app::App::makeAsyncSubmitInfo() -> pbr::AsyncSubmitInfo {
  return {
      .waitSemaphore =
          pbr::AsyncSubmitInfo::WaitSemaphore {
              .semaphore = _gpu->getDevice().createSemaphoreUnique({}),
              .waitDstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
          },
      .signalSemaphore = _gpu->getDevice().createSemaphoreUnique({}),
      .cmdBuffer =
          std::move(_gpu->getDevice()
                        .allocateCommandBuffersUnique(
                            {.commandPool = _commandPool.get(), .commandBufferCount = 1})
                        .front()),
  };
}

auto app::App::recordCommands(vk::CommandBuffer cmdBuffer,
                              pbr::SwapchainImageView imageView) -> void {
  // Render the scene
  _pbrSystem.render(cmdBuffer, _scene, _gBuffer, _hdrImage.getImage(),
                    _hdrImage.getExtent());

  // Run tonemapper
  _hdrImage.updateOutputTexture(imageView.getImage(), imageView.getImageView());
  _tonemapper.run(cmdBuffer, _hdrImage);

  { // Render imgui
    {
      vk::ImageMemoryBarrier2 const barrier {
          .srcStageMask = vk::PipelineStageFlagBits2::eComputeShader,
          .srcAccessMask = vk::AccessFlagBits2::eShaderStorageWrite,
          .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
          .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
          .oldLayout = vk::ImageLayout::eGeneral,
          .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
          .image = imageView.getImage(),
          .subresourceRange {
              .aspectMask = vk::ImageAspectFlagBits::eColor,
              .levelCount = 1,
              .layerCount = 1,
          },
      };
      cmdBuffer.pipelineBarrier2(vk::DependencyInfo {}.setImageMemoryBarriers(barrier));
    }
    vk::RenderingAttachmentInfo const attachmentInfo {
        .imageView = imageView.getImageView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eLoad,
        .storeOp = vk::AttachmentStoreOp::eStore,
    };
    auto const renderInfo =
        vk::RenderingInfo {
            .renderArea {
                .extent = imageView.getExtent(),
            },
            .layerCount = 1,
        }
            .setColorAttachments(attachmentInfo);
    cmdBuffer.beginRendering(renderInfo);
    _imguiRenderer.render(cmdBuffer);
    cmdBuffer.endRendering();
  }
}

auto app::App::resizeBuffers() -> void {
  auto const windowExtent = pbr::utils::toExtent(_window->getFramebufferSize());

  if (_gBuffer.getExtent() != windowExtent) {
    _gBuffer = _pbrSystem.allocateGBuffer(*_allocator, windowExtent);
  }

  if (_hdrImage.getExtent() != windowExtent) {
    _hdrImage = _tonemapper.allocateHdrImage(*_allocator, windowExtent);
  }
}

auto app::App::renderAndPresent() -> void {
  auto asyncInfo = _submitter.isSubmitted() ? _submitter.wait() : makeAsyncSubmitInfo();
  assert(asyncInfo.waitSemaphore.has_value());
  assert(asyncInfo.signalSemaphore.has_value());
  auto const renderDoneSemaphore = asyncInfo.signalSemaphore->get();

  auto imageView =
      _surface.acquireSwapchainImageView(asyncInfo.waitSemaphore->semaphore.get());
  assert(imageView.has_value());

  asyncInfo.cmdBuffer->reset();
  asyncInfo.cmdBuffer->begin(vk::CommandBufferBeginInfo {
      .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

  resizeBuffers();

  recordCommands(asyncInfo.cmdBuffer.get(), *imageView);
  {
    vk::ImageMemoryBarrier2 const barrier {
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::ePresentSrcKHR,
        .image = imageView->getImage(),
        .subresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    asyncInfo.cmdBuffer->pipelineBarrier2(
        vk::DependencyInfo {}.setImageMemoryBarriers(barrier));
  }
  asyncInfo.cmdBuffer->end();

  _submitter.submit(std::move(asyncInfo));

  imageView->present(renderDoneSemaphore);
}
