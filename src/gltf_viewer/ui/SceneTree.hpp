#pragma once

#include "pbr/Scene.hpp"

#include <chrono>

namespace app::ui {
class SceneTree {
public:
  static constexpr auto DEFAULT_OPEN = true;

private:
  bool _open = DEFAULT_OPEN;
  pbr::Scene* _scene = nullptr;

public:
  [[nodiscard]]
  auto getScene() const noexcept -> pbr::Scene*;

  auto setScene(pbr::Scene* scene) noexcept -> void;

  auto render(std::chrono::nanoseconds deltaTime) -> void;
};
} // namespace app::ui
