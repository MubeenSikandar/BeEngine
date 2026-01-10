// Camera/OrthographicCameraController.hpp
#pragma once

#include <Camera/OrthographicCamera.hpp>
#include <Events/Event.hpp>
#include <Time/Timestep.hpp>

namespace BeEngine {

/**
 * @brief Controller for OrthographicCamera
 *
 * Provides:
 * - WASD/Arrow key movement
 * - Q/E rotation
 * - Mouse scroll zoom
 * - Viewport resize handling
 */
class BE_API OrthographicCameraController {
public:
  /**
   * @brief Construct controller
   * @param aspectRatio Initial aspect ratio
   * @param size        Initial orthographic size
   * @param rotation    Enable rotation (default: true)
   */
  OrthographicCameraController(float aspectRatio, float size = 1.0F,
                               bool rotation = true);

  // ===== Update =====

  /// Call every frame to process input
  void OnUpdate(Timestep ts);

  /// Handle events (scroll, resize)
  void OnEvent(Event &event);

  // ===== Camera Access =====

  NODISCARD OrthographicCamera &GetCamera() { return m_Camera; }
  NODISCARD const OrthographicCamera &GetCamera() const { return m_Camera; }

  // ===== Settings =====

  void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }
  NODISCARD float GetMoveSpeed() const { return m_MoveSpeed; }

  void SetRotationSpeed(float speed) { m_RotationSpeed = speed; }
  NODISCARD float GetRotationSpeed() const { return m_RotationSpeed; }

  void SetZoomSpeed(float speed) { m_ZoomSpeed = speed; }
  NODISCARD float GetZoomSpeed() const { return m_ZoomSpeed; }

  void SetZoomLimits(float min, float max) {
    m_MinZoom = min;
    m_MaxZoom = max;
  }

  void EnableRotation(bool enable) { m_RotationEnabled = enable; }
  NODISCARD bool IsRotationEnabled() const { return m_RotationEnabled; }

  // ===== Viewport =====

  void OnViewportResize(float width, float height);

  // ===== State Access =====
  NODISCARD const glm::vec3 &GetPosition() const { return m_Position; }
  NODISCARD float GetRotation() const { return m_Rotation; }
  NODISCARD float GetZoom() const { return m_Zoom; }

private:
  bool OnMouseScrolled(class MouseScrolledEvent &event);
  bool OnWindowResized(class WindowResizeEvent &event);

  OrthographicCamera m_Camera;

  // Movement settings
  float m_MoveSpeed{5.0F};
  float m_RotationSpeed{180.0F}; // Degrees per second
  float m_ZoomSpeed{0.1F};

  // Zoom limits
  float m_MinZoom{0.1F};
  float m_MaxZoom{10.0F};

  // State
  glm::vec3 m_Position{0.0F};
  float m_Rotation{0.0F};
  float m_Zoom{1.0F};

  bool m_RotationEnabled{true};
  float m_AspectRatio;
};

} // namespace BeEngine
