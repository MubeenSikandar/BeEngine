// Camera/OrthographicCameraController.cpp
#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

OrthographicCameraController::OrthographicCameraController(float aspectRatio,
                                                           float size,
                                                           bool rotation)
    : m_Camera(aspectRatio, size), m_RotationEnabled(rotation),
      m_AspectRatio(aspectRatio) {

  BE_CORE_TRACE("OrthographicCameraController created: aspect={}, size={}",
                aspectRatio, size);
}

void OrthographicCameraController::OnUpdate(Timestep ts) {
  float dt = ts.GetSeconds();

  // Calculate movement direction based on camera rotation
  float rad = glm::radians(m_Rotation);
  float cos_r = glm::cos(rad);
  float sin_r = glm::sin(rad);

  // Speed scales with zoom (zoomed out = move faster)
  float effectiveSpeed = m_MoveSpeed / m_Zoom;

  // WASD Movement (rotated by camera angle)
  if (Input::IsKeyPressed(KeyCode::W) || Input::IsKeyPressed(KeyCode::Up)) {
    m_Position.x += sin_r * effectiveSpeed * dt;
    m_Position.y -= cos_r * effectiveSpeed * dt;
  }
  if (Input::IsKeyPressed(KeyCode::S) || Input::IsKeyPressed(KeyCode::Down)) {
    m_Position.x -= sin_r * effectiveSpeed * dt;
    m_Position.y += cos_r * effectiveSpeed * dt;
  }
  if (Input::IsKeyPressed(KeyCode::A) || Input::IsKeyPressed(KeyCode::Left)) {
    m_Position.x += cos_r * effectiveSpeed * dt;
    m_Position.y += sin_r * effectiveSpeed * dt;
  }
  if (Input::IsKeyPressed(KeyCode::D) || Input::IsKeyPressed(KeyCode::Right)) {
    m_Position.x -= cos_r * effectiveSpeed * dt;
    m_Position.y -= sin_r * effectiveSpeed * dt;
  }

  // Q/E Rotation
  if (m_RotationEnabled) {
    if (Input::IsKeyPressed(KeyCode::Q)) {
      m_Rotation -= m_RotationSpeed * dt;
    }
    if (Input::IsKeyPressed(KeyCode::E)) {
      m_Rotation += m_RotationSpeed * dt;
    }

    // Normalize rotation to [0, 360)
    if (m_Rotation > 360.0F) {
      m_Rotation -= 360.0F;
    }
    if (m_Rotation < 0.0F) {
      m_Rotation += 360.0F;
    }

    m_Camera.SetRotation(glm::radians(m_Rotation));
  }

  // Apply position
  m_Camera.SetPosition(m_Position);
}

void OrthographicCameraController::OnEvent(Event &event) {
  EventDispatcher dispatcher(event);
  dispatcher.Dispatch<MouseScrolledEvent>(
      [this](MouseScrolledEvent &e) { return OnMouseScrolled(e); });
  dispatcher.Dispatch<WindowResizeEvent>(
      [this](WindowResizeEvent &e) { return OnWindowResized(e); });
}

bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent &event) {
  m_Zoom -= event.GetYOffset() * m_ZoomSpeed;
  m_Zoom = glm::clamp(m_Zoom, m_MinZoom, m_MaxZoom);

  m_Camera.SetZoom(m_Zoom);

  return false; // Don't consume - other things might need scroll events
}

bool OrthographicCameraController::OnWindowResized(WindowResizeEvent &event) {
  OnViewportResize(static_cast<float>(event.GetWidth()),
                   static_cast<float>(event.GetHeight()));
  return false;
}

void OrthographicCameraController::OnViewportResize(float width, float height) {
  if (height == 0.0F) {
    return;
  }

  m_AspectRatio = width / height;
  m_Camera.OnViewportResize(width, height);
}

} // namespace BeEngine
