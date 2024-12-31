#include "App.hpp"

#include "pbr/Vulkan.hpp"

#include "pbr/gltf/Loader.hpp"
#include "vkfw/vkfw.hpp"

#include "pbr/utils/Conversions.hpp"

#include "pbr/core/GpuHandle.hpp"

#include "pbr/AsyncSubmitInfo.hpp"
#include "pbr/Buffer.hpp"
#include "pbr/Mesh.hpp"
#include "pbr/ModelPushConstant.hpp"
#include "pbr/PbrPipeline.hpp"
#include "pbr/SwapchainImageView.hpp"
#include "pbr/TransferStager.hpp"
#include "pbr/memory/IAllocator.hpp"
#include "pbr/memory/MemoryAllocator.hpp"

#include "CameraController.hpp"

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
#include <stdexcept>
#include <utility>
#include <vector>

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <glm/ext/matrix_transform.hpp>

namespace constants {
constexpr static auto DEFAULT_WINDOW_WIDTH = 1280uz;
constexpr static auto DEFAULT_WINDOW_HEIGHT = 720uz;
} // namespace constants

namespace {
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
constexpr auto loadPbrShaders(pbr::core::GpuHandle const& gpu)
    -> std::pair<vk::UniqueShaderModule, vk::UniqueShaderModule> {
  auto loadBinary = [](std::filesystem::path const& path) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    auto const size = std::filesystem::file_size(path);
    std::vector<std::uint32_t> binary(size / 4);

    // NOLINTNEXTLINE casting to char* is not UB
    file.read(reinterpret_cast<char*>(binary.data()), static_cast<std::streamsize>(size));

    return binary;
  };
  auto const vertexSpv = loadBinary("assets/shaders/compiled/pbr_vertex.spv");
  auto const fragmentSpv = loadBinary("assets/shaders/compiled/pbr_fragment.spv");
  return std::make_pair(gpu.getDevice().createShaderModuleUnique(
                            vk::ShaderModuleCreateInfo {}.setCode(vertexSpv)),
                        gpu.getDevice().createShaderModuleUnique(
                            vk::ShaderModuleCreateInfo {}.setCode(fragmentSpv)));
}
[[nodiscard]]
constexpr auto createPbrPipeline(pbr::core::GpuHandle const& gpu) -> pbr::PbrPipeline {
  auto const [vertexModule, fragmentModule] = loadPbrShaders(gpu);
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
      },
  };
}
[[nodiscard]]
constexpr auto loadMesh(std::filesystem::path const& path, pbr::core::SharedGpuHandle gpu,
                        std::shared_ptr<pbr::IAllocator> allocator,
                        vk::CommandPool cmdPool) -> pbr::Mesh {
  pbr::gltf::Loader loader;
  auto asset = loader.loadAsset(path);
  pbr::TransferStager stager(std::move(gpu), std::move(allocator));
  auto const meshHandle = stager.addTransfer(asset.buildMesh(0));
  stager.submit(cmdPool);
  stager.wait();
  return stager.get(meshHandle);
}
} // namespace

app::App::App(std::filesystem::path path)
    : _logger(::createLogger())
    , _path(::validatePath(std::move(path)))
    , _window(vkfw::createWindowUnique(constants::DEFAULT_WINDOW_WIDTH,
                                       constants::DEFAULT_WINDOW_HEIGHT, path.c_str()))
    , _controller(_window.get())
    , _gpu(pbr::core::makeGpuHandle({
          .extensions = vkfw::getRequiredInstanceExtensions(),
          .presentPredicate = vkfw::getPhysicalDevicePresentationSupport,
          .enableValidation = true,
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
              .descriptorCount = 1,
          },
      };
      return _gpu->getDevice().createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo {
          .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
          .maxSets = 1,
      }
                                                              .setPoolSizes(sizes));
    }())
    , _pbrPipeline(::createPbrPipeline(*_gpu))
    , _cameraUniform(*_gpu, *_allocator, _pbrPipeline.getCameraSetLayout(),
                     _descPool.get(), _controller.getCameraData())
    , _mesh(::loadMesh(_path, _gpu, _allocator, _commandPool.get()))
    , _submitter(_gpu) {
  _window->callbacks()->on_window_resize = [this](vkfw::Window const&, std::size_t width,
                                                  std::size_t height) {
    _surface.recreateSwapchain(pbr::utils::toExtent(width, height));
    _controller.onWindowResize(width, height);
  };
  _window->callbacks()->on_cursor_move = [this](vkfw::Window const&, double newX,
                                                double newY) {
    _controller.onCursorMove(newX, newY);
  };
  _window->callbacks()->on_scroll = [this](vkfw::Window const&, double xOffset,
                                           double yOffset) {
    _controller.onScroll(xOffset, yOffset);
  };

  _logger->info("Initialized app to view {}", _path.c_str());
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

    _controller.update(deltaTime);
    _cameraUniform.set(_controller.getCameraData());

    renderAndPresent();
  }
}

app::App::~App() noexcept { _gpu->getQueue().waitIdle(); }

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
  { // Swith imageView to colorAttachmentOptimal
    vk::ImageMemoryBarrier2 const barrier {
        .srcStageMask = vk::PipelineStageFlagBits2::eTopOfPipe,
        .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
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
  { // render to imageView
    vk::RenderingAttachmentInfo const attachmentInfo {
        .imageView = imageView.getImageView(),
        .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .loadOp = vk::AttachmentLoadOp::eClear,
        .storeOp = vk::AttachmentStoreOp::eStore,
        .clearValue = vk::ClearColorValue {}.setFloat32({0.0f, 0.0f, 0.0f, 1.0f}),
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
    { // set scissor and viewport for imageView
      auto const extent = imageView.getExtent();
      cmdBuffer.setScissor(0, vk::Rect2D {.extent = extent});
      cmdBuffer.setViewport(0, vk::Viewport {
                                   .width = static_cast<float>(extent.width),
                                   .height = static_cast<float>(extent.height),
                                   .maxDepth = 1.0f,
                               });
    }
    { // render the triangle
      cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                             _pbrPipeline.getPipeline());
      cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                   _pbrPipeline.getPipelineLayout(), 0,
                                   _cameraUniform.getDescriptorSet(), {});
      pbr::ModelPushConstant const modelPc {
          .model = glm::translate(glm::identity<glm::mat4x4>(), {-5.0, 0.0f, 0.0f}),
      };
      cmdBuffer.pushConstants<pbr::ModelPushConstant>(
          _pbrPipeline.getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, modelPc);
      cmdBuffer.bindVertexBuffers(0, _mesh.getVertexBuffer().getBuffer(), {0});
      cmdBuffer.bindIndexBuffer(_mesh.getIndexBuffer().getBuffer(), 0,
                                vk::IndexType::eUint16);

      for (auto primitive : _mesh.getPrimitives()) {
        cmdBuffer.drawIndexed(primitive.indexCount, 1, primitive.firstIndex,
                              static_cast<std::int32_t>(primitive.firstVertex), 0);
      }
    }
    cmdBuffer.endRendering();
  }
  { // switch imageView to presentSrc
    vk::ImageMemoryBarrier2 const barrier {
        .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
        .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
        .dstStageMask = vk::PipelineStageFlagBits2::eBottomOfPipe,
        .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
        .newLayout = vk::ImageLayout::ePresentSrcKHR,
        .image = imageView.getImage(),
        .subresourceRange {
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .levelCount = 1,
            .layerCount = 1,
        },
    };
    cmdBuffer.pipelineBarrier2(vk::DependencyInfo {}.setImageMemoryBarriers(barrier));
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
  recordCommands(asyncInfo.cmdBuffer.get(), *imageView);
  asyncInfo.cmdBuffer->end();

  _submitter.submit(std::move(asyncInfo));

  imageView->present(renderDoneSemaphore);
}
