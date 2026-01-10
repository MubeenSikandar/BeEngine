#pragma once

#include "Time/Timestep.hpp"
#include <Camera/PerspectiveCamera.hpp>
#include <Core.hpp>
#include <Events/Event.hpp>
namespace BeEngine {

/**
 * @brief Controller for PerspectiveCamera (3D)
 *
 * Provides:
 * - WASD movement (forward/back/strafe)
 * - Mouse look (pitch/yaw)
 * - Scroll to adjust move speed
 * - Space/Ctrl for up/down
 * - Shift for sprint
 *
 * Supports multiple modes:
 * - Fly mode (free camera, like Blender)
 * - FPS mode (locked to ground plane)
 */
class BE_API PerspectiveCameraController {
public:
  enum class Mode : uint8_t {
    Fly = 0, // Free movement in all directions
    FPS = 1  // Locked to XZ plane (Y is up)
  };

  /**
   * @brief Construct controller
   * @param aspectRatio Initial aspect ratio
   * @param fov         Field of view in degrees
   * @param nearClip    Near clipping plane
   * @param farClip     Far clipping plane
   */
  PerspectiveCameraController(float aspectRatio, float fov = 45.0F,
                              float nearClip = 0.1F, float farClip = 1000.0F);

  // ===== Update =====

  /// Call every frame to process input
  void OnUpdate(Timestep ts);

  /// Handle events (mouse, scroll, resize)
  void OnEvent(Event &event);

  // ===== Camera Access =====

  NODISCARD PerspectiveCamera &GetCamera() { return m_Camera; }
  NODISCARD const PerspectiveCamera &GetCamera() const { return m_Camera; }

  // ===== Movement Settings =====

  void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }
  NODISCARD float GetMoveSpeed() const { return m_MoveSpeed; }

  void SetSprintMultiplier(float mult) { m_SprintMultiplier = mult; }
  NODISCARD float GetSprintMultiplier() const { return m_SprintMultiplier; }

  void SetMouseSensitivity(float sens) { m_MouseSensitivity = sens; }
  NODISCARD float GetMouseSensitivity() const { return m_MouseSensitivity; }

  NODISCARD bool IsSprinting() const { return m_IsSprinting; }

  // ===== Mode =====

  void SetMode(Mode mode) { m_Mode = mode; }
  NODISCARD Mode GetMode() const { return m_Mode; }

  // ===== Mouse Control =====

  /// Enable/disable mouse look (right-click to look)
  void SetMouseLookEnabled(bool enabled) { m_MouseLookEnabled = enabled; }
  NODISCARD bool IsMouseLookEnabled() const { return m_MouseLookEnabled; }

  /// Lock cursor for continuous mouse look
  void SetCursorLocked(bool locked);
  NODISCARD bool IsCursorLocked() const { return m_CursorLocked; }

  // ===== Constraints =====

  /// Set pitch limits (prevent flipping)
  void SetPitchLimits(float minPitch, float maxPitch) {
    m_MinPitch = minPitch;
    m_MaxPitch = maxPitch;
  }

  // ===== Viewport =====

  void OnViewportResize(float width, float height);

  // ===== Position/Rotation =====

  void SetPosition(const glm::vec3 &position);
  NODISCARD const glm::vec3 &GetPosition() const { return m_Position; }

  void SetYaw(float yaw) {
    m_Yaw = yaw;
    UpdateCameraVectors();
  }
  void SetPitch(float pitch) {
    m_Pitch = glm::clamp(pitch, m_MinPitch, m_MaxPitch);
    UpdateCameraVectors();
  }

  NODISCARD float GetYaw() const { return m_Yaw; }
  NODISCARD float GetPitch() const { return m_Pitch; }

private:
  bool OnMouseMoved(class MouseMovedEvent &event);
  bool OnMouseScrolled(class MouseScrolledEvent &event);
  bool OnMouseButtonPressed(class MouseButtonPressedEvent &event);
  bool OnMouseButtonReleased(class MouseButtonReleasedEvent &event);
  bool OnWindowResized(class WindowResizeEvent &event);

  void UpdateCameraVectors();
  void ProcessKeyboardInput(float dt);
  void ProcessMouseMovement(float xOffset, float yOffset);

  PerspectiveCamera m_Camera;
  Mode m_Mode{Mode::Fly};

  // Position and orientation
  glm::vec3 m_Position{0.0F, 0.0F, 3.0F};
  float m_Yaw{-90.0F}; // Degrees - start looking down -Z
  float m_Pitch{0.0F}; // Degrees

  // Direction vectors (calculated from yaw/pitch)
  glm::vec3 m_Front{0.0F, 0.0F, -1.0F};
  glm::vec3 m_Right{1.0F, 0.0F, 0.0F};
  glm::vec3 m_Up{0.0F, 1.0F, 0.0F};
  glm::vec3 m_WorldUp{0.0F, 1.0F, 0.0F};

  // Movement settings
  float m_MoveSpeed{5.0F};
  float m_SprintMultiplier{2.5F};
  float m_MouseSensitivity{0.1F};
  float m_ScrollSensitivity{0.5F};

  // Pitch constraints (prevent gimbal lock / flipping)
  float m_MinPitch{-89.0F};
  float m_MaxPitch{89.0F};

  // Mouse state
  bool m_MouseLookEnabled{true};
  bool m_CursorLocked{false};
  bool m_RightMouseDown{false};
  glm::vec2 m_LastMousePos{0.0F, 0.0F};
  bool m_FirstMouse{true};
  bool m_IsSprinting{false};

  float m_AspectRatio;
};
} // namespace BeEngine
