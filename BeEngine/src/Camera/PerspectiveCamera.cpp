// Camera/PerspectiveCamera.cpp
#include "PCH/BeEnginePCH.hpp"
#include <glm/gtc/quaternion.hpp>

namespace BeEngine {

PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio,
                                     float nearClip, float farClip)
    : Camera(ProjectionType::Perspective), m_FOV(fov),
      m_AspectRatio(aspectRatio) {

  m_NearClip = nearClip;
  m_FarClip = farClip;

  RecalculateProjectionMatrix();
  RecalculateViewMatrix();

  BE_CORE_TRACE("PerspectiveCamera created: fov={}, aspect={}, near={}, far={}",
                fov, aspectRatio, nearClip, farClip);
}

void PerspectiveCamera::SetFOV(float fov) {
  m_FOV = glm::clamp(fov, 1.0F, 179.0F);
  RecalculateProjectionMatrix();
}

void PerspectiveCamera::SetAspectRatio(float aspectRatio) {
  m_AspectRatio = aspectRatio;
  RecalculateProjectionMatrix();
}

void PerspectiveCamera::SetClipPlanes(float nearClip, float farClip) {
  m_NearClip = nearClip;
  m_FarClip = farClip;
  RecalculateProjectionMatrix();
}

void PerspectiveCamera::SetPosition(const glm::vec3 &position) {
  m_Position = position;
  RecalculateViewMatrix();
}

void PerspectiveCamera::SetEulerAngles(const glm::vec3 &euler) {
  m_EulerAngles = euler;
  m_Orientation = glm::quat(euler);
  UpdateVectorsFromOrientation();
  RecalculateViewMatrix();
}

void PerspectiveCamera::SetOrientation(const glm::quat &orientation) {
  m_Orientation = orientation;
  m_EulerAngles = glm::eulerAngles(orientation);
  UpdateVectorsFromOrientation();
  RecalculateViewMatrix();
}

void PerspectiveCamera::LookAt(const glm::vec3 &target, const glm::vec3 &up) {
  m_Forward = glm::normalize(target - m_Position);
  m_Right = glm::normalize(glm::cross(m_Forward, up));
  m_Up = glm::cross(m_Right, m_Forward);

  // Update orientation from direction vectors
  m_ViewMatrix = glm::lookAt(m_Position, target, up);
  UpdateViewProjectionMatrix();
}

glm::vec3 PerspectiveCamera::GetForward() const { return m_Forward; }

glm::vec3 PerspectiveCamera::GetRight() const { return m_Right; }

glm::vec3 PerspectiveCamera::GetUp() const { return m_Up; }

void PerspectiveCamera::OnViewportResize(float width, float height) {
  if (height == 0.0F) {
    return;
  }

  m_AspectRatio = width / height;
  RecalculateProjectionMatrix();

  BE_CORE_TRACE("PerspectiveCamera viewport resized: {}x{}, aspect={}", width,
                height, m_AspectRatio);
}

void PerspectiveCamera::RecalculateViewMatrix() {
  // View matrix using position and orientation
  glm::mat4 rotation = glm::mat4_cast(glm::conjugate(m_Orientation));
  glm::mat4 translation = glm::translate(glm::mat4(1.0F), -m_Position);

  m_ViewMatrix = rotation * translation;
  UpdateViewProjectionMatrix();
}

void PerspectiveCamera::RecalculateProjectionMatrix() {
  m_ProjectionMatrix = glm::perspective(glm::radians(m_FOV), m_AspectRatio,
                                        m_NearClip, m_FarClip);
  UpdateViewProjectionMatrix();
}

void PerspectiveCamera::UpdateVectorsFromOrientation() {
  m_Forward = glm::normalize(m_Orientation * glm::vec3(0.0F, 0.0F, -1.0F));
  m_Right = glm::normalize(m_Orientation * glm::vec3(1.0F, 0.0F, 0.0F));
  m_Up = glm::normalize(m_Orientation * glm::vec3(0.0F, 1.0F, 0.0F));
}

} // namespace BeEngine
