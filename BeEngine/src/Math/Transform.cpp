#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
Transform::Transform() { MarkDirty(); }

Transform::Transform(const glm::vec3 &position) : m_Position(position) {
  MarkDirty();
}

Transform::Transform(const glm::vec3 &position, const glm::vec3 &rotation,
                     const glm::vec3 &scale)
    : m_Position(position), m_Scale(scale) {
  SetRotation(rotation);
  MarkDirty();
}

// ===== Position =====

void Transform::SetPosition(const glm::vec3 &position) {
  m_Position = position;
  MarkDirty();
}

void Transform::SetPosition(float x, float y, float z) {
  SetPosition({x, y, z});
}

void Transform::Translate(const glm::vec3 &delta) {
  m_Position += delta;
  MarkDirty();
}

void Transform::Translate(float dx, float dy, float dz) {
  Translate({dx, dy, dz});
}

// ===== Rotation =====

void Transform::SetRotation(const glm::vec3 &eulerDegrees) {
  m_Rotation = glm::quat(glm::radians(eulerDegrees));
  MarkDirty();
}

void Transform::SetRotation(float pitch, float yaw, float roll) {
  SetRotation({pitch, yaw, roll});
}

void Transform::SetRotation(const glm::quat &quaternion) {
  m_Rotation = quaternion;
  MarkDirty();
}

void Transform::Rotate(const glm::vec3 &eulerDegrees) {
  glm::quat deltaRotation = glm::quat(glm::radians(eulerDegrees));
  m_Rotation = deltaRotation * m_Rotation;
  MarkDirty();
}

void Transform::Rotate(float pitch, float yaw, float roll) {
  Rotate({pitch, yaw, roll});
}

void Transform::RotateAround(const glm::vec3 &axis, float angleDegrees) {
  glm::quat deltaRotation =
      glm::angleAxis(glm::radians(angleDegrees), glm::normalize(axis));
  m_Rotation = deltaRotation * m_Rotation;
  MarkDirty();
}

glm::vec3 Transform::GetEulerAngles() const {
  return glm::degrees(glm::eulerAngles(m_Rotation));
}

// ===== Scale =====

void Transform::SetScale(const glm::vec3 &scale) {
  m_Scale = scale;
  MarkDirty();
}

void Transform::SetScale(float uniformScale) {
  SetScale({uniformScale, uniformScale, uniformScale});
}

void Transform::SetScale(float x, float y, float z) { SetScale({x, y, z}); }

// ===== Direction Vectors =====

glm::vec3 Transform::GetForward() const {
  return glm::normalize(m_Rotation * glm::vec3(0.0F, 0.0F, -1.0F));
}

glm::vec3 Transform::GetRight() const {
  return glm::normalize(m_Rotation * glm::vec3(1.0F, 0.0F, 0.0F));
}

glm::vec3 Transform::GetUp() const {
  return glm::normalize(m_Rotation * glm::vec3(0.0F, 1.0F, 0.0F));
}

// ===== Matrices =====

const glm::mat4 &Transform::GetLocalMatrix() const {
  if (m_Dirty) {
    RecalculateMatrix();
  }
  return m_LocalMatrix;
}

void Transform::RecalculateMatrix() const {
  // Order: Scale -> Rotate -> Translate (M = T * R * S)
  m_LocalMatrix = glm::translate(glm::mat4(1.0f), m_Position) *
                  glm::mat4_cast(m_Rotation) *
                  glm::scale(glm::mat4(1.0f), m_Scale);
  m_Dirty = false;
}

// ===== Utility =====

void Transform::LookAt(const glm::vec3 &target, const glm::vec3 &up) {
  glm::vec3 direction = glm::normalize(target - m_Position);

  // Handle edge case where direction is parallel to up
  if (glm::abs(glm::dot(direction, up)) > 0.999f) {
    m_Rotation = glm::quatLookAt(direction, glm::vec3(0.0f, 0.0f, 1.0f));
  } else {
    m_Rotation = glm::quatLookAt(direction, up);
  }
  MarkDirty();
}

void Transform::Reset() {
  m_Position = glm::vec3(0.0f);
  m_Rotation = glm::identity<glm::quat>();
  m_Scale = glm::vec3(1.0f);
  MarkDirty();
}

void Transform::MarkDirty() { m_Dirty = true; }

} // namespace BeEngine
