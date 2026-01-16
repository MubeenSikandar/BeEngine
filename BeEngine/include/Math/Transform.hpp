#pragma once

#include <Core.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace BeEngine {

/**
 * @brief Transform component for positioning objects in 2D/3D space
 *
 * Supports:
 * - Position, Rotation (quaternion or euler), Scale
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

  // ===== Rotation =====
  void SetRotation(const glm::vec3 &eulerDegrees);
  void SetRotation(float pitch, float yaw, float roll);
  void SetRotation(const glm::quat &quaternion);
  void Rotate(const glm::vec3 &eulerDegrees);
  void Rotate(float pitch, float yaw, float roll);
  void RotateAround(const glm::vec3 &axis, float angleDegrees);

  NODISCARD const glm::quat &GetRotation() const { return m_Rotation; }
  NODISCARD glm::vec3 GetEulerAngles() const;

  // ===== Scale =====
  void SetScale(const glm::vec3 &scale);
  void SetScale(float uniformScale);
  void SetScale(float x, float y, float z);

  NODISCARD const glm::vec3 &GetScale() const { return m_Scale; }

  // ===== Direction Vectors =====
  NODISCARD glm::vec3 GetForward() const;
  NODISCARD glm::vec3 GetRight() const;
  NODISCARD glm::vec3 GetUp() const;

  // ===== Matrix =====
  NODISCARD const glm::mat4 &GetLocalMatrix() const;
  NODISCARD const glm::mat4 &GetWorldMatrix() const { return GetLocalMatrix(); }

  // ===== Utility =====
  void LookAt(const glm::vec3 &target,
              const glm::vec3 &up = glm::vec3(0.0f, 1.0f, 0.0f));
  void Reset();

  // ===== 2D Convenience =====
  void SetPosition2D(float x, float y) { SetPosition(x, y, m_Position.z); }
  void SetPosition2D(const glm::vec2 &pos) {
    SetPosition(pos.x, pos.y, m_Position.z);
  }
  NODISCARD glm::vec2 GetPosition2D() const {
    return {m_Position.x, m_Position.y};
  }

  void SetRotation2D(float angleDegrees) {
    SetRotation(0.0f, 0.0f, angleDegrees);
  }
  NODISCARD float GetRotation2D() const { return GetEulerAngles().z; }

  void SetScale2D(float x, float y) { SetScale(x, y, m_Scale.z); }
  void SetScale2D(float uniform) { SetScale(uniform, uniform, m_Scale.z); }
  NODISCARD glm::vec2 GetScale2D() const { return {m_Scale.x, m_Scale.y}; }

private:
  void MarkDirty();
  void RecalculateMatrix() const;

  glm::vec3 m_Position{0.0F, 0.0F, 0.0F};
  glm::quat m_Rotation{glm::identity<glm::quat>()};
  glm::vec3 m_Scale{1.0F, 1.0F, 1.0F};

  mutable glm::mat4 m_LocalMatrix{1.0F};
  mutable bool m_Dirty{true};
};

} // namespace BeEngine
