#pragma once

#include "ui/PerformanceOverlay.hpp"
#include "ui/SceneTree.hpp"

#include <chrono>

namespace app {
class AppUi {
public:
  static constexpr auto ROUNDING = 4.0f;

  ui::PerformanceOverlay performanceOverlay {};
  ui::SceneTree sceneTree {};

  AppUi();

  auto render(std::chrono::nanoseconds deltaTime) -> void;
};
} // namespace app
