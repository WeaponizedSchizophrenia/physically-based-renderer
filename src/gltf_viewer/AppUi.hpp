#pragma once

#include "ui/PerformanceOverlay.hpp"

#include <chrono>

namespace app {
class AppUi {
  ui::PerformanceOverlay _performanceOverlay;

public:
  AppUi() = default;

  auto render(std::chrono::nanoseconds deltaTime) -> void;
};
}
