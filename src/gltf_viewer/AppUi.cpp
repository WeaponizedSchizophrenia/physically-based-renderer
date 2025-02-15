#include "AppUi.hpp"

#include "imgui.h"

#include <chrono>

app::AppUi::AppUi() {
  auto& style = ImGui::GetStyle();
  style.WindowBorderSize = 1.0f;
  style.ChildBorderSize = 1.0f;
  style.PopupBorderSize = 1.0f;
  style.FrameBorderSize = 1.0f;
  style.TabBorderSize = 1.0f;
  style.TabBorderSize = 1.0f;
  style.WindowRounding = ROUNDING;
  style.ChildRounding = ROUNDING;
  style.FrameRounding = ROUNDING;
  style.PopupRounding = ROUNDING;
  style.ScrollbarRounding = ROUNDING;
  style.GrabRounding = ROUNDING;
  style.TabRounding = ROUNDING;
  // NOLINTNEXTLINE 0.5 is not a magic number
  style.WindowTitleAlign.x = 0.5f;
  style.WindowMenuButtonPosition = ImGuiDir_Right;
}

auto app::AppUi::render(std::chrono::nanoseconds deltaTime) -> void {
  performanceOverlay.render(deltaTime);
  sceneTree.render(deltaTime);
}
