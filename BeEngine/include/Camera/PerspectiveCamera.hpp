// Camera/PerspectiveCamera.hpp
#pragma once

#include <Camera/Camera.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace BeEngine {

/**
 * @brief 3D Perspective Camera
 *
 * Perfect for:
 * - 3D games
 * - First/Third person views
 * - 3D visualization
 *
 * Objects appear smaller with distance (realistic depth perception).
 */
class BE_API PerspectiveCamera : public Camera {
public:
  /**
   * @brief Construct perspective camera
   * @param fov         Field of view in degrees (vertical)
   * @param aspectRatio Width / Height
   * @param nearClip    Near clipping plane
   * @param farClip     Far clipping plane
   */
  PerspectiveCamera(float fov = 45.0F, float aspectRatio = 16.0F / 9.0F,
                    float nearClip = 0.1F, float farClip = 1000.0F);

  // ===== Projection Settings =====

  /// Set field of view (degrees)
  void SetFOV(float fov);
  NODISCARD float GetFOV() const { return m_FOV; }

  /// Set aspect ratio
  void SetAspectRatio(float aspectRatio);
  NODISCARD float GetAspectRatio() const { return m_AspectRatio; }

  /// Set clip planes
  void SetClipPlanes(float nearClip, float farClip);

  // ===== 3D Transform =====

  /// Set position (override for 3D)
  void SetPosition(const glm::vec3 &position) override;

  /// Set orientation using Euler angles (pitch, yaw, roll) in radians
  void SetEulerAngles(const glm::vec3 &euler);
  NODISCARD const glm::vec3 &GetEulerAngles() const { return m_EulerAngles; }

  /// Set orientation using quaternion
  void SetOrientation(const glm::quat &orientation);
  NODISCARD const glm::quat &GetOrientation() const { return m_Orientation; }

  /// Look at a target point
  void LookAt(const glm::vec3 &target,
              const glm::vec3 &up = glm::vec3(0.0F, 1.0F, 0.0F));

  // ===== Direction Vectors =====

  NODISCARD glm::vec3 GetForward() const;
  NODISCARD glm::vec3 GetRight() const;
  NODISCARD glm::vec3 GetUp() const;

  // ===== Viewport =====

  void OnViewportResize(float width, float height);

protected:
  void RecalculateViewMatrix() override;
  void RecalculateProjectionMatrix() override;

private:
  void UpdateVectorsFromOrientation();

  float m_FOV{45.0F}; // Field of view (degrees)
  float m_AspectRatio{16.0F / 9.0F};

  // 3D Orientation
  glm::vec3 m_EulerAngles{0.0F}; // Pitch, Yaw, Roll (radians)
  glm::quat m_Orientation{glm::identity<glm::quat>()};

  // Cached direction vectors
  glm::vec3 m_Forward{0.0F, 0.0F, -1.0F};
  glm::vec3 m_Right{1.0F, 0.0F, 0.0F};
  glm::vec3 m_Up{0.0F, 1.0F, 0.0F};
};

} // namespace BeEngine
