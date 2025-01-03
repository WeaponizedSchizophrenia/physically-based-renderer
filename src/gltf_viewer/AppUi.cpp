#include "AppUi.hpp"

#include <chrono>

auto app::AppUi::render(std::chrono::nanoseconds deltaTime) -> void {
  _performanceOverlay.render(deltaTime);
}
