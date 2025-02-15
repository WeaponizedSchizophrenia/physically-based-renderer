#include "ui/SceneTree.hpp"

#include "pbr/Scene.hpp"

#include "imgui.h"

#include <chrono>

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/trigonometric.hpp>

auto app::ui::SceneTree::getScene() const noexcept -> pbr::Scene* { return _scene; }

auto app::ui::SceneTree::setScene(pbr::Scene* scene) noexcept -> void {
  _scene = scene;
  _selectedNode = nullptr;
}

auto app::ui::SceneTree::render([[maybe_unused]] std::chrono::nanoseconds deltaTime)
    -> void {
  auto const [vpWidth, vpHeight] = ImGui::GetMainViewport()->Size;
  ImGui::SetNextWindowSize(ImVec2(vpWidth * 0.5f, vpHeight * 0.5f),
                           ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Scene tree", &_open)) {
    auto const [width, height] = ImGui::GetWindowSize();

    ImGui::BeginChild("scene-tree", ImVec2(width * 0.3f, 0),
                      ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders);
    renderTree();
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("scene-tree-node-view");
    renderNodeView();
    ImGui::EndChild();
  }
  ImGui::End();
}

auto app::ui::SceneTree::renderTree() -> void {
  if (_scene == nullptr) {
    ImGui::TextColored({1.0, 1.0, 0.0, 1.0}, "No scene found!");
  } else {
    for (auto& node : _scene->getTopLevelNodes()) {
      renderNode(node);
    }
  }
}

auto app::ui::SceneTree::renderNode(pbr::Node& node) -> void {
  ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow;
  if (node.getChildren().empty()) {
    nodeFlags |= ImGuiTreeNodeFlags_Leaf;
  }
  if (&node == _selectedNode) {
    nodeFlags |= ImGuiTreeNodeFlags_Selected;
  }

  auto const isNodeOpen = ImGui::TreeNodeEx(node.getName().c_str(), nodeFlags);

  if (ImGui::IsItemClicked()) {
    _selectedNode = &node;
  }

  if (isNodeOpen) {
    for (auto& node : node.getChildren()) {
      renderNode(node);
    }

    ImGui::TreePop();
  }
}

auto app::ui::SceneTree::renderNodeView() -> void {
  if (_selectedNode == nullptr) {
    ImGui::TextColored({1.0, 1.0, 0.0, 1.0}, "No node selected!");
  } else {
    auto& node = *_selectedNode;

    ImGui::Text("%s", node.getName().c_str());
    ImGui::Separator();

    node.setTransform(renderTransform(node.getTransform()));
  }
}

auto app::ui::SceneTree::renderTransform(pbr::Transform transform) -> pbr::Transform {
  if (ImGui::CollapsingHeader("Transform")) {
    ImGui::Indent(15.0f);

    renderRotationFormatCombo();

    ImGui::DragFloat3("Position", &transform.position.x);
    constexpr auto rotationSliderFlags =
        ImGuiSliderFlags_WrapAround | ImGuiSliderFlags_AlwaysClamp;
    switch (_rotationFormat) {
      using enum RotationFormat;
    case Quaternion:
      ImGui::DragFloat4("Rotation", &transform.rotation.x, 0.05f);
      break;
    case EulerAnglesRad: {
      auto eulerAngles = glm::eulerAngles(transform.rotation);
      if (ImGui::DragFloat3("Rotation", &eulerAngles.x, 0.05f, -glm::half_pi<float>(),
                            glm::half_pi<float>(), "%.3f", rotationSliderFlags)) {
        transform.rotation = glm::quat(eulerAngles);
      }
      break;
    }
    case EulerAnglesDeg: {
      auto eulerAnglesRad = glm::eulerAngles(transform.rotation);
      glm::vec3 eulerAnglesDeg {
          glm::degrees(eulerAnglesRad.x),
          glm::degrees(eulerAnglesRad.y),
          glm::degrees(eulerAnglesRad.z),
      };
      if (ImGui::DragFloat3("Rotation", &eulerAnglesDeg.x, 0.1f, -90.0f, 90.0f, "%.3f",
                            rotationSliderFlags)) {
        eulerAnglesRad = glm::vec3 {
            glm::radians(eulerAnglesDeg.x),
            glm::radians(eulerAnglesDeg.y),
            glm::radians(eulerAnglesDeg.z),
        };
        transform.rotation = glm::quat(eulerAnglesRad);
      }
      break;
    }
    }

    ImGui::DragFloat3("Scale", &transform.scale.x, 0.1f, 0.001f, 0.0f);
    ImGui::Indent(0.0f);
  }
  return transform;
}

auto app::ui::SceneTree::renderRotationFormatCombo() -> void {
  if (ImGui::BeginCombo("Rotation format", ui::rotationFormatToString(_rotationFormat))) {
    auto renderSelectable = [this](RotationFormat format) {
      if (ImGui::Selectable(ui::rotationFormatToString(format),
                            _rotationFormat == format)) {
        _rotationFormat = format;
      }
    };

    renderSelectable(RotationFormat::Quaternion);
    renderSelectable(RotationFormat::EulerAnglesRad);
    renderSelectable(RotationFormat::EulerAnglesDeg);

    ImGui::EndCombo();
  }
}
