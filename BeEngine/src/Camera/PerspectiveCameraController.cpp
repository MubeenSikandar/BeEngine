#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
PerspectiveCameraController::PerspectiveCameraController(float aspectRatio,
                                                         float fov,
                                                         float nearClip,
                                                         float farClip)
    : m_Camera(fov, aspectRatio, nearClip, farClip),
      m_AspectRatio(aspectRatio) {

  UpdateCameraVectors();

  BE_CORE_TRACE("PerspectiveCameraController created: aspect={}, fov={}",
                aspectRatio, fov);
}

void PerspectiveCameraController::OnUpdate(Timestep ts) {
  float dt = ts.GetSeconds();

  // Process keyboard movement
  ProcessKeyboardInput(dt);

  // Update camera position and orientation
  m_Camera.SetPosition(m_Position);
  m_Camera.LookAt(m_Position + m_Front, m_WorldUp);
}

void PerspectiveCameraController::OnEvent(Event &event) {
  EventDispatcher dispatcher(event);

  dispatcher.Dispatch<MouseMovedEvent>(
      [this](MouseMovedEvent &e) { return OnMouseMoved(e); });
  dispatcher.Dispatch<MouseScrolledEvent>(
      [this](MouseScrolledEvent &e) { return OnMouseScrolled(e); });
  dispatcher.Dispatch<MouseButtonPressedEvent>(
      [this](MouseButtonPressedEvent &e) { return OnMouseButtonPressed(e); });
  dispatcher.Dispatch<MouseButtonReleasedEvent>(
      [this](MouseButtonReleasedEvent &e) { return OnMouseButtonReleased(e); });
  dispatcher.Dispatch<WindowResizeEvent>(
      [this](WindowResizeEvent &e) { return OnWindowResized(e); });
}

void PerspectiveCameraController::ProcessKeyboardInput(float dt) {
  float speed = m_MoveSpeed;

  if (Input::IsKeyPressed(KeyCode::LeftShift) ||
      Input::IsKeyPressed(KeyCode::RightShift)) {
    speed *= m_SprintMultiplier;
    m_IsSprinting = true;
  } else {
    m_IsSprinting = false;
  }

  float velocity = speed * dt;

  // WASD movement
  if (Input::IsKeyPressed(KeyCode::W)) {
    if (m_Mode == Mode::Fly) {
      m_Position += m_Front * velocity;
    } else {
      // FPS mode: move along XZ plane only
      glm::vec3 forward = glm::normalize(glm::vec3(m_Front.x, 0.0F, m_Front.z));
      m_Position += forward * velocity;
    }
  }
  if (Input::IsKeyPressed(KeyCode::S)) {
    if (m_Mode == Mode::Fly) {
      m_Position -= m_Front * velocity;
    } else {
      glm::vec3 forward = glm::normalize(glm::vec3(m_Front.x, 0.0F, m_Front.z));
      m_Position -= forward * velocity;
    }
  }
  if (Input::IsKeyPressed(KeyCode::A)) {
    m_Position -= m_Right * velocity;
  }
  if (Input::IsKeyPressed(KeyCode::D)) {
    m_Position += m_Right * velocity;
  }

  // Up/Down movement
  if (Input::IsKeyPressed(KeyCode::Space)) {
    if (m_Mode == Mode::Fly) {
      m_Position += m_WorldUp * velocity;
    }
    // In FPS mode, Space could trigger jump (not implemented here)
  }
  if (Input::IsKeyPressed(KeyCode::LeftControl) ||
      Input::IsKeyPressed(KeyCode::RightControl)) {
    if (m_Mode == Mode::Fly) {
      m_Position -= m_WorldUp * velocity;
    }
  }

  // Use C or F for down (more reliable on Mac than Ctrl/Cmd)
  if (Input::IsKeyPressed(KeyCode::C) || Input::IsKeyPressed(KeyCode::F) ||
      Input::IsKeyPressed(KeyCode::LeftControl) ||
      Input::IsKeyPressed(KeyCode::RightControl)) {
    if (m_Mode == Mode::Fly) {
      m_Position -= m_WorldUp * velocity;
    }
  }

  // Q/E for roll-like horizontal rotation (optional)
  if (Input::IsKeyPressed(KeyCode::Q)) {
    m_Yaw -= 90.0F * dt;
    UpdateCameraVectors();
  }
  if (Input::IsKeyPressed(KeyCode::E)) {
    m_Yaw += 90.0F * dt;
    UpdateCameraVectors();
  }

  if (Input::IsKeyPressed(KeyCode::R)) {
    m_Pitch += 45.0F * dt;
    m_Pitch = glm::clamp(m_Pitch, m_MinPitch, m_MaxPitch);
    UpdateCameraVectors();
  }
}

void PerspectiveCameraController::ProcessMouseMovement(float xOffset,
                                                       float yOffset) {

  xOffset *= m_MouseSensitivity;
  yOffset *= m_MouseSensitivity;

  m_Yaw += xOffset;
  m_Pitch -= yOffset; // Inverted: moving mouse up looks up

  // Constrain pitch
  m_Pitch = glm::clamp(m_Pitch, m_MinPitch, m_MaxPitch);

  // Normalize yaw
  if (m_Yaw > 360.0F) {
    m_Yaw -= 360.0F;
  }
  if (m_Yaw < 0.0F) {
    m_Yaw += 360.0F;
  }

  UpdateCameraVectors();
}

void PerspectiveCameraController::UpdateCameraVectors() {
  // Calculate front vector from Euler angles
  glm::vec3 front;
  front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
  front.y = sin(glm::radians(m_Pitch));
  front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

  m_Front = glm::normalize(front);
  m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
  m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

bool PerspectiveCameraController::OnMouseMoved(MouseMovedEvent &event) {
  if (!m_MouseLookEnabled) {
    return false;
  }

  float xPos = event.GetX();
  float yPos = event.GetY();

  if (m_FirstMouse) {
    m_LastMousePos = {xPos, yPos};
    m_FirstMouse = false;
    return false;
  }

  // Only process mouse look when right mouse button is held (or cursor locked)
  if (m_RightMouseDown || m_CursorLocked) {
    float xOffset = xPos - m_LastMousePos.x;
    float yOffset = yPos - m_LastMousePos.y;

    ProcessMouseMovement(xOffset, yOffset);
  }

  m_LastMousePos = {xPos, yPos};
  return false;
}

bool PerspectiveCameraController::OnMouseScrolled(MouseScrolledEvent &event) {
  // Scroll adjusts movement speed
  m_MoveSpeed += event.GetYOffset() * m_ScrollSensitivity;
  m_MoveSpeed = glm::clamp(m_MoveSpeed, 0.5F, 50.0F);

  BE_CORE_TRACE("Camera move speed: {:.1f}", m_MoveSpeed);

  return false;
}

bool PerspectiveCameraController::OnMouseButtonPressed(
    MouseButtonPressedEvent &event) {
  if (event.GetMouseButton() == MouseButton::Right) {
    m_RightMouseDown = true;
    m_FirstMouse = true; // Reset to avoid jump
  }
  return false;
}

bool PerspectiveCameraController::OnMouseButtonReleased(
    MouseButtonReleasedEvent &event) {
  if (event.GetMouseButton() == MouseButton::Right) {
    m_RightMouseDown = false;
  }
  return false;
}

bool PerspectiveCameraController::OnWindowResized(WindowResizeEvent &event) {
  OnViewportResize(static_cast<float>(event.GetWidth()),
                   static_cast<float>(event.GetHeight()));
  return false;
}

void PerspectiveCameraController::OnViewportResize(float width, float height) {
  if (height == 0.0F) {
    return;
  }

  m_AspectRatio = width / height;
  m_Camera.OnViewportResize(width, height);
}

void PerspectiveCameraController::SetPosition(const glm::vec3 &position) {
  m_Position = position;
  m_Camera.SetPosition(position);
}

void PerspectiveCameraController::SetCursorLocked(bool locked) {
  m_CursorLocked = locked;
  m_FirstMouse = true; // Avoid jump when locking

  // TODO: Actually lock/unlock cursor via Window class
  // Application::Get().GetWindow().SetCursorMode(locked ? CursorMode::Locked :
  // CursorMode::Normal);
}

} // namespace BeEngine
