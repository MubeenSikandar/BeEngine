#include "PCH/BeEnginePCH.hpp"

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

glm::vec3 Transform::GetWorldPosition() const {
  if (m_Parent) {
    return glm::vec3(m_Parent->GetWorldMatrix() * glm::vec4(m_Position, 1.0F));
  }
  return m_Position;
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

glm::quat Transform::GetWorldRotation() const {
  if (m_Parent) {
    return m_Parent->GetWorldRotation() * m_Rotation;
  }
  return m_Rotation;
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

glm::vec3 Transform::GetWorldScale() const {
  if (m_Parent) {
    return m_Parent->GetWorldScale() * m_Scale;
  }
  return m_Scale;
}

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

const glm::mat4 &Transform::GetLocalMatrix() {
  if (m_LocalDirty) {
    RecalculateLocalMatrix();
  }
  return m_LocalMatrix;
}

const glm::mat4 &Transform::GetWorldMatrix() {
  if (m_WorldDirty || m_LocalDirty) {
    RecalculateWorldMatrix();
  }
  return m_WorldMatrix;
}

glm::mat4 Transform::GetInverseWorldMatrix() {
  return glm::inverse(GetWorldMatrix());
}

void Transform::RecalculateLocalMatrix() {
  // Order: Scale -> Rotate -> Translate
  // M = T * R * S
  m_LocalMatrix = glm::translate(glm::mat4(1.0F), m_Position) *
                  glm::mat4_cast(m_Rotation) *
                  glm::scale(glm::mat4(1.0F), m_Scale);
  m_LocalDirty = false;
}

void Transform::RecalculateWorldMatrix() {
  if (m_LocalDirty) {
    RecalculateLocalMatrix();
  }

  if (m_Parent) {
    m_WorldMatrix = m_Parent->GetWorldMatrix() * m_LocalMatrix;
  } else {
    m_WorldMatrix = m_LocalMatrix;
  }
  m_WorldDirty = false;
}

// ===== Hierarchy =====

void Transform::SetParent(Transform *parent) {
  // Remove from old parent
  if (m_Parent) {
    m_Parent->RemoveChild(this);
  }

  m_Parent = parent;

  // Add to new parent
  if (m_Parent) {
    m_Parent->AddChild(this);
  }

  MarkDirty();
}

void Transform::AddChild(Transform *child) {
  if (child && std::ranges::find(m_Children.begin(), m_Children.end(), child) ==
                   m_Children.end()) {
    m_Children.push_back(child);
  }
}

void Transform::RemoveChild(Transform *child) {
  auto it = std::ranges::find(m_Children.begin(), m_Children.end(), child);
  if (it != m_Children.end()) {
    m_Children.erase(it);
  }
}

// ===== Utility =====

void Transform::LookAt(const glm::vec3 &target, const glm::vec3 &up) {
  glm::vec3 direction = glm::normalize(target - m_Position);
  m_Rotation = glm::quatLookAt(direction, up);
  MarkDirty();
}

void Transform::Reset() {
  m_Position = glm::vec3(0.0F);
  m_Rotation = glm::identity<glm::quat>();
  m_Scale = glm::vec3(1.0F);
  MarkDirty();
}

void Transform::MarkDirty() {
  m_LocalDirty = true;
  m_WorldDirty = true;

  // Propagate to children
  for (auto *child : m_Children) {
    if (child) {
      child->m_WorldDirty = true;
      child->MarkDirty(); // Recursively mark children dirty
    }
  }
}

} // namespace BeEngine
