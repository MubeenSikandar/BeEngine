// Camera/Camera.cpp
#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

void Camera::RecalculateViewMatrix() {

  // For 2D: simple translation and rotation around Z-axis
  // View = inverse(Transform) = inverse(T * R)
  glm::mat4 transform =
      glm::translate(glm::mat4(1.0F), m_Position) *
      glm::rotate(glm::mat4(1.0F), m_Rotation, glm::vec3(0.0F, 0.0F, 1.0F));

  m_ViewMatrix = glm::inverse(transform);
  UpdateViewProjectionMatrix();
}

glm::vec3 Camera::ScreenToWorld(const glm::vec2 &screenPos,
                                const glm::vec2 &screenSize) const {
  // Normalize screen coordinates to NDC (-1 to 1)
  float x = ((2.0F * screenPos.x) / screenSize.x) - 1.0f;
  float y = 1.0F - ((2.0F * screenPos.y) / screenSize.y); // Flip Y

  // Inverse view-projection to get world coordinates
  glm::mat4 inverseVP = glm::inverse(m_ViewProjectionMatrix);
  glm::vec4 worldPos = inverseVP * glm::vec4(x, y, 0.0f, 1.0f);

  return glm::vec3(worldPos) / worldPos.w;
}

glm::vec2 Camera::WorldToScreen(const glm::vec3 &worldPos,
                                const glm::vec2 &screenSize) const {
  // Transform to clip space
  glm::vec4 clipPos = m_ViewProjectionMatrix * glm::vec4(worldPos, 1.0f);

  // Perspective divide
  glm::vec3 ndc = glm::vec3(clipPos) / clipPos.w;

  // Convert NDC to screen coordinates
  float x = (ndc.x + 1.0F) * 0.5F * screenSize.x;
  float y = (1.0F - ndc.y) * 0.5F * screenSize.y; // Flip Y

  return {x, y};
}

} // namespace BeEngine
