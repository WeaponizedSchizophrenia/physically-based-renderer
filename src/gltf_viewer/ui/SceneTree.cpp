#include "ui/SceneTree.hpp"

#include "pbr/Scene.hpp"

#include "imgui.h"

#include <chrono>

auto app::ui::SceneTree::getScene() const noexcept -> pbr::Scene* { return _scene; }

auto app::ui::SceneTree::setScene(pbr::Scene* scene) noexcept -> void { _scene = scene; }

auto app::ui::SceneTree::render([[maybe_unused]] std::chrono::nanoseconds deltaTime)
    -> void {
  if (ImGui::Begin("Scene tree", &_open)) {
    if (_scene == nullptr) {
      ImGui::TextColored({1.0, 1.0, 0.0, 1.0}, "No scene found!");
    } else {
      if (ImGui::TreeNode("Node")) {
        ImGui::LabelText("Text", "TextFmt");
        ImGui::TreePop();
      }
    }
  }
  ImGui::End();
}
