#include "ui/PerformanceOverlay.hpp"

#include "imgui.h"

#include <chrono>
#include <limits>

auto app::ui::PerformanceOverlay::render(std::chrono::nanoseconds deltaTime) -> void {
  ImGui::SetNextWindowPos(calculateOverlayPosition(), ImGuiCond_Always);
  ImGui::SetNextWindowSizeConstraints(
      {MIN_WIDTH, 0.0f},
      {std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
  ImGui::SetNextWindowBgAlpha(ALPHA);

  if (ImGui::Begin("Performance overlay", &_open, createWindowFlags())) {
    ImGui::Text(
        "Frame time %.3f ms",
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(deltaTime)
            .count());
    ImGui::End();
  }
}

auto app::ui::PerformanceOverlay::calculateOverlayPosition() -> ImVec2 {
  auto const* const viewport = ImGui::GetMainViewport();
  return {viewport->WorkPos.x + PADDING, viewport->WorkPos.y + PADDING};
}

auto app::ui::PerformanceOverlay::createWindowFlags() noexcept -> ImGuiWindowFlags {
  return ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
         | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
         | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
         | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}
