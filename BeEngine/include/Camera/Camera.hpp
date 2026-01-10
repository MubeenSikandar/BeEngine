#pragma once

#include <Core.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace BeEngine {

/**
 * @brief Abstract base class for all camera types
 *
 * Cameras transform world coordinates to screen coordinates through:
 * 1. View Matrix - Transforms world space to camera/view space
 * 2. Projection Matrix - Transforms view space to clip space (NDC)
 *
 * ViewProjection = Projection * View
 * ClipPosition = ViewProjection * WorldPosition
 */

class BE_API Camera {
public:
  enum class ProjectionType : uint8_t { Orthographic = 0, Perspective = 1 };

  virtual ~Camera() = default;

  // ===== Core Matrices =====

  /// View matrix: transforms world space -> camera space
  NODISCARD const glm::mat4 &GetViewMatrix() const { return m_ViewMatrix; }

  /// Projection matrix: transforms camera space -> clip space
  NODISCARD const glm::mat4 &GetProjectionMatrix() const {
    return m_ProjectionMatrix;
  }

  /// Combined view-projection matrix (cached for performance)
  NODISCARD const glm::mat4 &GetViewProjectionMatrix() const {
    return m_ViewProjectionMatrix;
  }

  // ===== Transform =====

  NODISCARD const glm::vec3 &GetPosition() const { return m_Position; }
  virtual void SetPosition(const glm::vec3 &position) {
    m_Position = position;
    RecalculateViewMatrix();
  }

  NODISCARD float GetRotation() const { return m_Rotation; }
  virtual void SetRotation(float rotation) {
    m_Rotation = rotation;
    RecalculateViewMatrix();
  }

  // ===== Camera Info =====

  NODISCARD ProjectionType GetProjectionType() const {
    return m_ProjectionType;
  }
  NODISCARD float GetNearClip() const { return m_NearClip; }
  NODISCARD float GetFarClip() const { return m_FarClip; }

  // ===== Utility =====

  /// Convert screen coordinates to world coordinates
  NODISCARD virtual glm::vec3 ScreenToWorld(const glm::vec2 &screenPos,
                                            const glm::vec2 &screenSize) const;

  /// Convert world coordinates to screen coordinates
  NODISCARD virtual glm::vec2 WorldToScreen(const glm::vec3 &worldPos,
                                            const glm::vec2 &screenSize) const;

protected:
  Camera(ProjectionType type) : m_ProjectionType(type) {}

  virtual void RecalculateViewMatrix();
  virtual void RecalculateProjectionMatrix() = 0;

  void UpdateViewProjectionMatrix() {
    m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
  }

  // Matrices
  glm::mat4 m_ViewMatrix{1.0F};
  glm::mat4 m_ProjectionMatrix{1.0F};
  glm::mat4 m_ViewProjectionMatrix{1.0F};

  // Transform
  glm::vec3 m_Position{0.0F, 0.0F, 0.0F};
  float m_Rotation{0.0F}; // Z-axis rotation in radians

  // Clip planes
  float m_NearClip{-1.0F};
  float m_FarClip{1.0F};

  ProjectionType m_ProjectionType;
};

} // namespace BeEngine
