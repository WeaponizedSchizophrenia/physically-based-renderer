#include "ui/PerformanceOverlay.hpp"
#include "imgui.h"

#include <chrono>

auto app::ui::PerformanceOverlay::render(std::chrono::nanoseconds deltaTime) -> void {
  ImGui::SetNextWindowPos(calculateOverlayPosition(), ImGuiCond_Always);
  ImGui::SetNextWindowBgAlpha(_alpha);

  if (ImGui::Begin("Performance overlay", &_open, createWindowFlags())) {
    ImGui::Text(
        "Frame time %.3f ms",
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(deltaTime)
            .count());
    ImGui::End();
  }
}

auto app::ui::PerformanceOverlay::calculateOverlayPosition() const -> ImVec2 {
  auto const* const viewport = ImGui::GetMainViewport();
  return {viewport->WorkPos.x + _padding, viewport->WorkPos.y + _padding};
}

auto app::ui::PerformanceOverlay::createWindowFlags() const noexcept -> ImGuiWindowFlags {
  return ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
         | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
         | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
         | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
}
