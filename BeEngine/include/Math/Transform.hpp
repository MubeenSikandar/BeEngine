#pragma once

#include "Core.hpp"
#include "glm/fwd.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace BeEngine {

/**
 * @brief Transform component for positioning objects in 2D/3D space
 *
 * Supports:
 * - Position, Rotation (quaternion or euler), Scale
 * - Parent-child hierarchy
 * - Local and World space transformations
 * - Lazy matrix recalculation (dirty flag)
 */
class BE_API Transform {
public:
  Transform();
  explicit Transform(const glm::vec3 &position);
  Transform(const glm::vec3 &position, const glm::vec3 &rotation,
            const glm::vec3 &scale);

  // ===== Position =====
  void SetPosition(const glm::vec3 &position);
  void SetPosition(float x, float y, float z);
  void Translate(const glm::vec3 &delta);
  void Translate(float dx, float dy, float dz);

  NODISCARD const glm::vec3 &GetPosition() const { return m_Position; }
  NODISCARD glm::vec3 GetWorldPosition() const;

  // ===== Rotation =====

  /// Set rotation using euler angles (degrees)
  void SetRotation(const glm::vec3 &eulerDegrees);
  void SetRotation(float pitch, float yaw, float roll);

  /// Set rotation using quaternion
  void SetRotation(const glm::quat &quaternion);

  /// Rotate by euler angles (degrees)
  void Rotate(const glm::vec3 &eulerDegrees);
  void Rotate(float pitch, float yaw, float roll);

  /// Rotate around an axis
  void RotateAround(const glm::vec3 &axis, float angleDegrees);

  NODISCARD const glm::quat &GetRotation() const { return m_Rotation; }
  NODISCARD glm::vec3 GetEulerAngles() const; // Returns degrees
  NODISCARD glm::quat GetWorldRotation() const;

  // ===== Scale =====

  void SetScale(const glm::vec3 &scale);
  void SetScale(float uniformScale);
  void SetScale(float x, float y, float z);

  NODISCARD const glm::vec3 &GetScale() const { return m_Scale; }
  NODISCARD glm::vec3 GetWorldScale() const;

  // ===== Direction Vectors =====

  NODISCARD glm::vec3 GetForward() const; // Local -Z axis
  NODISCARD glm::vec3 GetRight() const;   // Local +X axis
  NODISCARD glm::vec3 GetUp() const;      // Local +Y axis

  // ===== Matrices =====

  /// Get local transformation matrix (relative to parent)
  NODISCARD const glm::mat4 &GetLocalMatrix();

  /// Get world transformation matrix (absolute)
  NODISCARD const glm::mat4 &GetWorldMatrix();

  /// Get inverse world matrix (useful for view matrices)
  NODISCARD glm::mat4 GetInverseWorldMatrix();

  // ===== Hierarchy =====

  void SetParent(Transform *parent);
  NODISCARD Transform *GetParent() const { return m_Parent; }
  NODISCARD const std::vector<Transform *> &GetChildren() const {
    return m_Children;
  }

  void AddChild(Transform *child);
  void RemoveChild(Transform *child);

  // ===== Utility =====

  /// Look at a target position
  void LookAt(const glm::vec3 &target,
              const glm::vec3 &up = glm::vec3(0.0F, 1.0F, 0.0F));

  /// Reset to identity transform
  void Reset();

  // ===== 2D Convenience (for 2D games) =====

  void SetPosition2D(float x, float y) { SetPosition(x, y, m_Position.z); }
  void SetPosition2D(const glm::vec2 &pos) {
    SetPosition(pos.x, pos.y, m_Position.z);
  }
  NODISCARD glm::vec2 GetPosition2D() const {
    return {m_Position.x, m_Position.y};
  }

  void SetRotation2D(float angleDegrees) {
    SetRotation(0.0F, 0.0F, angleDegrees);
  }
  NODISCARD float GetRotation2D() const { return GetEulerAngles().z; }

  void SetScale2D(float x, float y) { SetScale(x, y, m_Scale.z); }
  void SetScale2D(float uniform) { SetScale(uniform, uniform, m_Scale.z); }
  NODISCARD glm::vec2 GetScale2D() const { return {m_Scale.x, m_Scale.y}; }

private:
  void MarkDirty();
  void RecalculateLocalMatrix();
  void RecalculateWorldMatrix();

  // Transform data
  glm::vec3 m_Position{0.0F, 0.0F, 0.0F};
  glm::quat m_Rotation{glm::identity<glm::quat>()};
  glm::vec3 m_Scale{1.0F, 1.0F, 1.0F};

  // Cached matrices
  glm::mat4 m_LocalMatrix{1.0F};
  glm::mat4 m_WorldMatrix{1.0F};

  // Dirty flags for lazy recalculation
  bool m_LocalDirty{true};
  bool m_WorldDirty{true};

  // Hierarchy
  Transform *m_Parent{nullptr};
  std::vector<Transform *> m_Children;
};
} // namespace BeEngine
