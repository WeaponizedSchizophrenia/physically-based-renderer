#pragma once

#include "ui/PerformanceOverlay.hpp"

#include <chrono>

namespace app {
class AppUi {
public:
  static constexpr auto ROUNDING = 4.0f;

private:
  ui::PerformanceOverlay _performanceOverlay {};

public:
  AppUi();

  auto render(std::chrono::nanoseconds deltaTime) -> void;
};
} // namespace app
