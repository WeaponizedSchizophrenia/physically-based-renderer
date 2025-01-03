#pragma once

#include "imgui.h"

#include <chrono>

namespace app::ui {
class PerformanceOverlay {
public:
  static constexpr auto PADDING = 10.0f;
  static constexpr auto ALPHA = 0.25f;
  static constexpr auto MIN_WIDTH = 200.0f;
  static constexpr auto DEFAULT_OPEN = true;

private:
  bool _open = DEFAULT_OPEN;

public:
  PerformanceOverlay() = default;

  auto render(std::chrono::nanoseconds deltaTime) -> void;

private:
  [[nodiscard]]
  static auto calculateOverlayPosition() -> ImVec2;
  [[nodiscard]]
  static auto createWindowFlags() noexcept -> ImGuiWindowFlags;
};
} // namespace app::ui
