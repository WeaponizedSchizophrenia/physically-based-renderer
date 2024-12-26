#include "App.hpp"

#include "vkfw/vkfw.hpp"

#include <filesystem>
#include <format>
#include <stdexcept>
#include <utility>

namespace constants {
constexpr static auto DEFAULT_WINDOW_WIDTH = 1280uz;
constexpr static auto DEFAULT_WINDOW_HEIGHT = 720uz;
} // namespace constants

namespace {
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
} // namespace

app::App::App(std::filesystem::path path)
    : _path(::validatePath(std::move(path)))
    , _window(vkfw::createWindow(constants::DEFAULT_WINDOW_WIDTH,
                                 constants::DEFAULT_WINDOW_HEIGHT, path.c_str())) {}

auto app::App::run() -> void {
  while (!_window.shouldClose()) {
    vkfw::pollEvents();

    _window.setShouldClose(true);
  }
}

app::App::~App() noexcept { _window.destroy(); }
