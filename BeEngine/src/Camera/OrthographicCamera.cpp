// Camera/OrthographicCamera.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

OrthographicCamera::OrthographicCamera(float left, float right, float bottom,
                                       float top)
    : Camera(ProjectionType::Orthographic), m_Left(left), m_Right(right),
      m_Bottom(bottom), m_Top(top) {

  m_AspectRatio = (right - left) / (top - bottom);
  m_Size = (top - bottom) / 2.0f;
  m_NearClip = -1.0f;
  m_FarClip = 1.0f;

  RecalculateProjectionMatrix();
  RecalculateViewMatrix();

  BE_CORE_TRACE(
      "OrthographicCamera created: bounds=[{}, {}, {}, {}], aspect={}", left,
      right, bottom, top, m_AspectRatio);
}

OrthographicCamera::OrthographicCamera(float aspectRatio, float size)
    : Camera(ProjectionType::Orthographic), m_AspectRatio(aspectRatio),
      m_Size(size) {

  m_NearClip = -1.0f;
  m_FarClip = 1.0f;

  SetProjectionByAspect(aspectRatio, size);
  RecalculateViewMatrix();

  BE_CORE_TRACE("OrthographicCamera created: aspect={}, size={}", aspectRatio,
                size);
}

void OrthographicCamera::SetProjection(float left, float right, float bottom,
                                       float top) {
  m_Left = left;
  m_Right = right;
  m_Bottom = bottom;
  m_Top = top;
  m_AspectRatio = (right - left) / (top - bottom);
  m_Size = (top - bottom) / 2.0f;

  RecalculateProjectionMatrix();
}

void OrthographicCamera::SetProjectionByAspect(float aspectRatio, float size) {
  m_AspectRatio = aspectRatio;
  m_Size = size;

  float effectiveSize = size / m_Zoom;
  m_Left = -aspectRatio * effectiveSize;
  m_Right = aspectRatio * effectiveSize;
  m_Bottom = -effectiveSize;
  m_Top = effectiveSize;

  RecalculateProjectionMatrix();
}

void OrthographicCamera::SetClipPlanes(float nearClip, float farClip) {
  m_NearClip = nearClip;
  m_FarClip = farClip;
  RecalculateProjectionMatrix();
}

void OrthographicCamera::SetZoom(float zoom) {
  m_Zoom = glm::clamp(zoom, 0.01f, 100.0f); // Prevent invalid zoom
  SetProjectionByAspect(m_AspectRatio, m_Size);
}

void OrthographicCamera::SetSize(float size) {
  m_Size = glm::max(size, 0.01f); // Prevent zero/negative size
  SetProjectionByAspect(m_AspectRatio, m_Size);
}

void OrthographicCamera::OnViewportResize(float width, float height) {
  if (height == 0.0f)
    return;

  m_AspectRatio = width / height;
  SetProjectionByAspect(m_AspectRatio, m_Size);

  BE_CORE_TRACE("OrthographicCamera viewport resized: {}x{}, aspect={}", width,
                height, m_AspectRatio);
}

void OrthographicCamera::RecalculateProjectionMatrix() {
  // glm::ortho creates an orthographic projection matrix
  // Maps (left,right,bottom,top,near,far) to normalized device coordinates
  m_ProjectionMatrix =
      glm::ortho(m_Left, m_Right, m_Bottom, m_Top, m_NearClip, m_FarClip);
  UpdateViewProjectionMatrix();
}

} // namespace BeEngine
