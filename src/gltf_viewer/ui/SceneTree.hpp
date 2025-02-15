#pragma once

#include "pbr/Scene.hpp"

#include <chrono>
#include <cstdint>
#include <utility>

namespace app::ui {
enum struct RotationFormat : std::uint8_t {
  Quaternion,
  EulerAnglesRad,
  EulerAnglesDeg,
};
[[nodiscard]]
constexpr auto rotationFormatToString(RotationFormat format) noexcept -> char const*;

class SceneTree {
  static constexpr auto DEFAULT_OPEN = true;
  static constexpr auto DEFAULT_ROTATION_FORMAT = RotationFormat::EulerAnglesDeg;

  bool _open = DEFAULT_OPEN;
  pbr::Scene* _scene = nullptr;
  // This is very bad, dont use a pointer in the future (maybe index).
  pbr::Node* _selectedNode = nullptr;

  RotationFormat _rotationFormat = DEFAULT_ROTATION_FORMAT;

public:
  [[nodiscard]]
  auto getScene() const noexcept -> pbr::Scene*;

  auto setScene(pbr::Scene* scene) noexcept -> void;

  auto render(std::chrono::nanoseconds deltaTime) -> void;

private:
  auto renderNode(pbr::Node& node) -> void;
  auto renderTree() -> void;
  auto renderNodeView() -> void;
  auto renderTransform(pbr::Transform transform) -> pbr::Transform;
  auto renderRotationFormatCombo() -> void;
};
} // namespace app::ui

/* IMPLEMENTATIONS */

constexpr auto app::ui::rotationFormatToString(RotationFormat format) noexcept -> char const* {
  switch(format) {
    using enum RotationFormat;
    case Quaternion: return "Quaternion";
    case EulerAnglesRad: return "Euler angles in radians";
    case EulerAnglesDeg: return "Euler angles in degrees";
    default: std::unreachable();
  }
}
