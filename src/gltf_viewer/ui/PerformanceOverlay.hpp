#pragma once

#include "imgui.h"

#include <chrono>

namespace app::ui {
class PerformanceOverlay {
public:
  static constexpr auto DEFAULT_PADDING = 10.0f;
  static constexpr auto DEFAULT_ALPHA = 0.25f;
  static constexpr auto DEFAULT_OPEN = true;

private:
  float _padding = DEFAULT_PADDING;
  float _alpha = DEFAULT_ALPHA;
  bool _open = DEFAULT_OPEN;

public:
  PerformanceOverlay() = default;

  auto render(std::chrono::nanoseconds deltaTime) -> void;

private:
  [[nodiscard]]
  auto calculateOverlayPosition() const -> ImVec2;
  [[nodiscard]]
  auto createWindowFlags() const noexcept -> ImGuiWindowFlags;
};
} // namespace app::ui
