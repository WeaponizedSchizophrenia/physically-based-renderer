#pragma once

#include "vkfw/vkfw.hpp"

#include "pbr/CameraData.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

#include <glm/common.hpp>
#include <glm/ext/matrix_float3x3.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

namespace app {
class CameraController {
public:
  /// Starting position
  static constexpr auto DEFAULT_POSITION = glm::vec3(0.0);
  /// Starting pitch
  static constexpr auto DEFAULT_PITCH = 0.0f;
  /// Starting yaw
  static constexpr auto DEFAULT_YAW = -glm::half_pi<float>();
  /// Mouse look sensitivity
  static constexpr auto DEFAULT_SENSITIVITY = 0.01f;
  /// Controller move speed
  static constexpr auto DEFAULT_SPEED = 0.02f;
  /// Starting vertical fov in radians
  static constexpr auto DEFAULT_FOV = glm::half_pi<float>();

  static constexpr auto MIN_PITCH = -glm::half_pi<float>() + 0.01f;
  static constexpr auto MAX_PITCH = -MIN_PITCH;
  static constexpr auto MIN_FOV = 0.1f;
  static constexpr auto MAX_FOV = glm::half_pi<float>();

  /// The acceleration factor (gets multiplied by deltaTime and used to interpolate).
  static constexpr auto ACCELERATION_FACTOR = 10.0f;
  static constexpr auto FOV_INCREMENT = 0.1f;

private:
  vkfw::Window _window;
  glm::vec2 _lastMousePos;

  glm::vec3 _position = DEFAULT_POSITION;
  glm::vec3 _velocity {};

  float _pitch = DEFAULT_PITCH;
  float _yaw = DEFAULT_YAW;

  float _sensitivity = DEFAULT_SENSITIVITY;
  float _speed = DEFAULT_SPEED;

  float _aspectRatio;
  float _fov = DEFAULT_FOV;

public:
  constexpr explicit CameraController(vkfw::Window window);

  constexpr auto onWindowResize(std::size_t width, std::size_t height) -> void;
  constexpr auto onCursorMove(double newX, double newY) -> void;
  constexpr auto onScroll(double xOffset, double yOffset) -> void;
  constexpr auto update(double deltaTime) -> void;

  [[nodiscard]]
  constexpr auto getCameraData() const noexcept -> pbr::CameraData;

private:
  [[nodiscard]]
  constexpr auto getDirection() const noexcept -> glm::vec3;

  constexpr auto accelerate(glm::vec3 acceleration, float lerp) noexcept -> void;

  [[nodiscard]]
  constexpr auto getInputVector() const noexcept -> glm::vec3;

  [[nodiscard]]
  constexpr auto getMatrix() const noexcept -> glm::mat3x3;
};
} // namespace app

/* IMPLEMENTATIONS */

constexpr app::CameraController::CameraController(vkfw::Window window)
    : _window(std::move(window))
    , _lastMousePos(static_cast<float>(_window.getCursorPosX()),
                    static_cast<float>(_window.getCursorPosY()))
    , _aspectRatio(static_cast<float>(_window.getFramebufferWidth())
                   / static_cast<float>(_window.getFramebufferHeight())) {}

constexpr auto app::CameraController::onWindowResize(std::size_t width,
                                                     std::size_t height) -> void {
  _aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}
constexpr auto app::CameraController::onCursorMove(double const newX,
                                                   double const newY) -> void {
  glm::vec2 const newMousePos {newX, newY};
  glm::vec2 const mouseDelta = newMousePos - _lastMousePos;
  _lastMousePos = newMousePos;

  if (_window.getMouseButton(vkfw::MouseButton::eRight)) {
    _yaw += mouseDelta.x * _sensitivity;
    _pitch += mouseDelta.y * _sensitivity;
    _pitch = std::clamp(_pitch, MIN_PITCH, MAX_PITCH);
  }
}
constexpr auto app::CameraController::onScroll(double /*xOffset*/,
                                               double yOffset) -> void {
  _fov = std::clamp(_fov - (static_cast<float>(yOffset) * FOV_INCREMENT), MIN_FOV, MAX_FOV);
}
constexpr auto app::CameraController::update(double deltaTime) -> void {
  auto const inputVector = getInputVector();
  if (glm::length(inputVector) != 0.0f) {
    accelerate(glm::normalize(inputVector) * _speed,
               static_cast<float>(deltaTime) * ACCELERATION_FACTOR);
  }

  _position += getMatrix() * _velocity;
  accelerate({}, static_cast<float>(deltaTime) * ACCELERATION_FACTOR);
}

constexpr auto app::CameraController::getCameraData() const noexcept -> pbr::CameraData {
  return pbr::makeCameraData(_position, _position + getDirection(), _fov, _aspectRatio);
}

constexpr auto app::CameraController::getDirection() const noexcept -> glm::vec3 {
  auto const cosPitch = std::cos(_pitch);
  return glm::normalize(
      glm::vec3(cosPitch * std::cos(_yaw), std::sin(_pitch), cosPitch * std::sin(_yaw)));
}

constexpr auto app::CameraController::accelerate(glm::vec3 acceleration,
                                                 float lerp) noexcept -> void {
  _velocity = glm::mix(_velocity, acceleration, lerp);
}

constexpr auto app::CameraController::getInputVector() const noexcept -> glm::vec3 {
  glm::vec3 vec {};
  if (_window.getKey(vkfw::Key::eA)) {
    vec.x = -1.0f;
  } else if (_window.getKey(vkfw::Key::eD)) {
    vec.x = 1.0f;
  }
  if (_window.getKey(vkfw::Key::eW)) {
    vec.z = 1.0f;
  } else if (_window.getKey(vkfw::Key::eS)) {
    vec.z = -1.0f;
  }
  return vec;
}

constexpr auto app::CameraController::getMatrix() const noexcept -> glm::mat3x3 {
  auto const forward = getDirection();
  glm::vec3 const upVec {0.0f, 1.0f, 0.0f};
  auto const right = glm::cross(forward, upVec);
  return {
      right,
      upVec,
      forward,
  };
}
