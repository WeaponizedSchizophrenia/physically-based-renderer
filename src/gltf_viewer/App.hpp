#pragma once

#include "vkfw/vkfw.hpp"

#include <filesystem>

namespace app {
class App {
  std::filesystem::path _path;
  vkfw::Window _window;

public:
  explicit App(std::filesystem::path path);

  App(const App&) = delete;
  auto operator=(const App&) -> App& = delete;
  App(App&&) = default;
  auto operator=(App&&) -> App& = default;

  ~App() noexcept;

  auto run() -> void;
};
} // namespace app
