// SandboxLayer3D.cpp
#include "SandboxLayer3D.hpp"
#include <vector>

void SandboxLayer3D::OnAttach() {
  BE_INFO("SandboxLayer3D attached - setting up 3D scene");

  float aspectRatio = 1280.0f / 720.0f;

  m_CameraController = std::make_unique<BeEngine::PerspectiveCameraController>(
      aspectRatio, 45.0f, 0.1f, 1000.0f);

  // Position camera to see the scene nicely
  m_CameraController->SetPosition({3.0f, 2.0f, 5.0f});
  m_CameraController->SetYaw(-120.0f);
  m_CameraController->SetPitch(-15.0f);

  m_CubeTransform.SetPosition(0.0F, 0.5F, 0.0F);
  m_CubeTransform.SetPosition(0.0F, 0.5F, 0.0F);
  m_CubeEulerAngles = {0.0F, 0.0F, 0.0F};

  BeEngine::FramebufferSpecification fbSpec;
  fbSpec.Width = 1280;
  fbSpec.Height = 720;
  fbSpec.Attachments = {BeEngine::FramebufferTextureFormat::RGBA8,
                        BeEngine::FramebufferTextureFormat::Depth};
  m_Framebuffer = BeEngine::Framebuffer::Create(fbSpec);

  SetupCube();
  SetupGrid();
}

void SandboxLayer3D::OnDetach() { BE_INFO("SandboxLayer3D detached"); }

void SandboxLayer3D::OnUpdate(BeEngine::Timestep ts) {
  if (m_ViewportFocused) {
    m_CameraController->OnUpdate(ts);
  }

  // Auto-rotate cube
  if (m_AutoRotate) {
    m_CubeEulerAngles.y += 45.0F * ts.GetSeconds();
    if (m_CubeEulerAngles.y > 360.0F) {
      m_CubeEulerAngles.y -= 360.0F;
    }
    m_CubeTransform.SetRotation(m_CubeEulerAngles); // Apply euler to transform
  }

  // Handle viewport resize
  const auto &spec = m_Framebuffer->GetSpecification();
  if (m_ViewportSize.x > 0.0F && m_ViewportSize.y > 0.0F &&
      (static_cast<uint32_t>(m_ViewportSize.x) != spec.Width ||
       static_cast<uint32_t>(m_ViewportSize.y) != spec.Height)) {
    m_Framebuffer->Resize(static_cast<uint32_t>(m_ViewportSize.x),
                          static_cast<uint32_t>(m_ViewportSize.y));
    m_CameraController->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
  }
}

void SandboxLayer3D::OnEvent(BeEngine::Event &event) {
  m_CameraController->OnEvent(event);
}

void SandboxLayer3D::OnRender() {
  if (!m_CubeShader || !m_CubeVAO || !m_Framebuffer) {
    return;
  }

  m_Framebuffer->Bind();

  glClearColor(0.15F, 0.15F, 0.2F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE); // Show both sides of cube for now

  auto viewProj = m_CameraController->GetCamera().GetViewProjectionMatrix();

  // Draw Grid
  if (m_GridShader && m_GridVAO) {
    m_GridShader->Bind();
    m_GridShader->SetMat4("u_ViewProjection", viewProj);
    m_GridShader->SetMat4("u_Model", glm::mat4(1.0F));
    m_GridVAO->Bind();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_GridVertexCount));
  }

  // Draw Cube
  {
    glm::mat4 model = m_CubeTransform.GetWorldMatrix();

    m_CubeShader->Bind();
    m_CubeShader->SetMat4("u_ViewProjection", viewProj);
    m_CubeShader->SetMat4("u_Model", model);
    m_CubeVAO->Bind();                 // ADD THIS
    glDrawArrays(GL_TRIANGLES, 0, 36); // ADD THIS
  }

  m_Framebuffer->Unbind();
}

void SandboxLayer3D::OnImGuiRender() {
  // Viewport window
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Viewport");

  m_ViewportFocused = ImGui::IsWindowFocused();
  m_ViewportHovered = ImGui::IsWindowHovered();

  BeEngine::Application::Get().SetBlockEvents(!m_ViewportHovered);

  ImVec2 viewportSize = ImGui::GetContentRegionAvail();
  m_ViewportSize = {viewportSize.x, viewportSize.y};

  uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
  ImGui::Image(reinterpret_cast<void *>(textureID),
               ImVec2(m_ViewportSize.x, m_ViewportSize.y), ImVec2(0, 1),
               ImVec2(1, 0));

  ImGui::End();
  ImGui::PopStyleVar();

  // Camera info window
  ImGui::Begin("Camera");
  auto camPos = m_CameraController->GetPosition(); // Renamed to camPos
  ImGui::Text("Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
  ImGui::Text("Yaw: %.1f°  Pitch: %.1f°", m_CameraController->GetYaw(),
              m_CameraController->GetPitch());

  float speed = m_CameraController->GetMoveSpeed();
  bool sprinting = m_CameraController->IsSprinting();
  if (sprinting) {
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
                       "Speed: %.1f (SPRINTING)",
                       speed * m_CameraController->GetSprintMultiplier());
  } else {
    ImGui::Text("Speed: %.1f", speed);
  }

  ImGui::Text("FOV: %.1f°", m_CameraController->GetCamera().GetFOV());

  ImGui::Separator();
  ImGui::Text("Controls:");
  ImGui::Text("  WASD        - Move");
  ImGui::Text("  Space       - Up");
  ImGui::Text("  C/F         - Down");
  ImGui::Text("  Shift+Move  - Sprint");
  ImGui::Text("  RMB + Drag  - Look");
  ImGui::Text("  Q/E         - Turn");
  ImGui::Text("  Scroll      - Adjust Speed");
  ImGui::End();

  // Scene
  ImGui::Begin("Scene");

  ImGui::Text("Cube Transform");

  glm::vec3 cubePos = m_CubeTransform.GetPosition();
  if (ImGui::DragFloat3("Position", &cubePos.x, 0.1f)) {
    m_CubeTransform.SetPosition(cubePos);
  }

  // Use stored euler angles instead of converting from quaternion
  if (ImGui::DragFloat3("Rotation", &m_CubeEulerAngles.x, 1.0f)) {
    m_CubeTransform.SetRotation(m_CubeEulerAngles);
  }

  glm::vec3 scale = m_CubeTransform.GetScale();
  if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 10.0f)) {
    m_CubeTransform.SetScale(scale);
  }

  ImGui::Checkbox("Auto Rotate", &m_AutoRotate);

  if (ImGui::Button("Reset Cube")) {
    m_CubeTransform.Reset();
    m_CubeTransform.SetPosition(0.0f, 0.5f, 0.0f);
    m_CubeEulerAngles = {0.0f, 0.0f, 0.0f}; // Reset euler angles too
  }

  ImGui::Separator();

  if (ImGui::Button("Reset Camera")) {
    m_CameraController->SetPosition({3.0f, 2.0f, 5.0f});
    m_CameraController->SetYaw(-120.0f);
    m_CameraController->SetPitch(-15.0f);
  }

  ImGui::End();
}

void SandboxLayer3D::SetupCube() {
  BE_INFO("Setting up 3D cube...");

  // Cube vertices with colors (position + color)
  // Each face has a different color
  std::vector<float> vertices = {
      // Front face (Red)
      -0.5f,
      -0.5f,
      0.5f,
      1.0f,
      0.3f,
      0.3f,
      0.5f,
      -0.5f,
      0.5f,
      1.0f,
      0.3f,
      0.3f,
      0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.3f,
      0.3f,
      0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.3f,
      0.3f,
      -0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.3f,
      0.3f,
      -0.5f,
      -0.5f,
      0.5f,
      1.0f,
      0.3f,
      0.3f,

      // Back face (Green)
      -0.5f,
      -0.5f,
      -0.5f,
      0.3f,
      1.0f,
      0.3f,
      0.5f,
      0.5f,
      -0.5f,
      0.3f,
      1.0f,
      0.3f,
      0.5f,
      -0.5f,
      -0.5f,
      0.3f,
      1.0f,
      0.3f,
      0.5f,
      0.5f,
      -0.5f,
      0.3f,
      1.0f,
      0.3f,
      -0.5f,
      -0.5f,
      -0.5f,
      0.3f,
      1.0f,
      0.3f,
      -0.5f,
      0.5f,
      -0.5f,
      0.3f,
      1.0f,
      0.3f,

      // Top face (Blue)
      -0.5f,
      0.5f,
      -0.5f,
      0.3f,
      0.3f,
      1.0f,
      0.5f,
      0.5f,
      0.5f,
      0.3f,
      0.3f,
      1.0f,
      0.5f,
      0.5f,
      -0.5f,
      0.3f,
      0.3f,
      1.0f,
      0.5f,
      0.5f,
      0.5f,
      0.3f,
      0.3f,
      1.0f,
      -0.5f,
      0.5f,
      -0.5f,
      0.3f,
      0.3f,
      1.0f,
      -0.5f,
      0.5f,
      0.5f,
      0.3f,
      0.3f,
      1.0f,

      // Bottom face (Yellow)
      -0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      1.0f,
      0.3f,
      0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      1.0f,
      0.3f,
      0.5f,
      -0.5f,
      0.5f,
      1.0f,
      1.0f,
      0.3f,
      0.5f,
      -0.5f,
      0.5f,
      1.0f,
      1.0f,
      0.3f,
      -0.5f,
      -0.5f,
      0.5f,
      1.0f,
      1.0f,
      0.3f,
      -0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      1.0f,
      0.3f,

      // Right face (Magenta)
      0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      0.3f,
      1.0f,
      0.5f,
      0.5f,
      -0.5f,
      1.0f,
      0.3f,
      1.0f,
      0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.3f,
      1.0f,
      0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.3f,
      1.0f,
      0.5f,
      -0.5f,
      0.5f,
      1.0f,
      0.3f,
      1.0f,
      0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      0.3f,
      1.0f,

      // Left face (Cyan)
      -0.5f,
      -0.5f,
      -0.5f,
      0.3f,
      1.0f,
      1.0f,
      -0.5f,
      0.5f,
      0.5f,
      0.3f,
      1.0f,
      1.0f,
      -0.5f,
      0.5f,
      -0.5f,
      0.3f,
      1.0f,
      1.0f,
      -0.5f,
      0.5f,
      0.5f,
      0.3f,
      1.0f,
      1.0f,
      -0.5f,
      -0.5f,
      -0.5f,
      0.3f,
      1.0f,
      1.0f,
      -0.5f,
      -0.5f,
      0.5f,
      0.3f,
      1.0f,
      1.0f,
  };

  m_CubeVAO = BeEngine::VertexArray::Create();
  auto vb = BeEngine::VertexBuffer::Create(
      vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));

  vb->SetLayout({{BeEngine::ShaderDataType::Float3, "a_Position"},
                 {BeEngine::ShaderDataType::Float3, "a_Color"}});
  m_CubeVAO->AddVertexBuffer(vb);

  std::string vertexSrc = R"(
    #version 410 core
    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec3 a_Color;

    uniform mat4 u_ViewProjection;
    uniform mat4 u_Model;

    out vec3 v_Color;

    void main() {
      v_Color = a_Color;
      gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
    }
  )";

  std::string fragmentSrc = R"(
    #version 410 core
    in vec3 v_Color;
    layout(location = 0) out vec4 o_Color;

    void main() {
      o_Color = vec4(v_Color, 1.0);
    }
  )";

  m_CubeShader = BeEngine::Shader::Create(vertexSrc, fragmentSrc);

  BE_INFO("Cube setup complete!");
}

void SandboxLayer3D::SetupGrid() {
  BE_INFO("Setting up grid...");

  std::vector<float> gridVertices;
  const int gridSize = 10;
  const float gridStep = 1.0f;
  const float gridY = 0.0f;

  // Generate grid lines
  for (int i = -gridSize; i <= gridSize; ++i) {
    float pos = static_cast<float>(i) * gridStep;

    // Lines parallel to Z axis
    gridVertices.push_back(pos);
    gridVertices.push_back(gridY);
    gridVertices.push_back(static_cast<float>(-gridSize) * gridStep);
    gridVertices.push_back(0.4f); // Color
    gridVertices.push_back(0.4f);
    gridVertices.push_back(0.4f);

    gridVertices.push_back(pos);
    gridVertices.push_back(gridY);
    gridVertices.push_back(static_cast<float>(gridSize) * gridStep);
    gridVertices.push_back(0.4f);
    gridVertices.push_back(0.4f);
    gridVertices.push_back(0.4f);

    // Lines parallel to X axis
    gridVertices.push_back(static_cast<float>(-gridSize) * gridStep);
    gridVertices.push_back(gridY);
    gridVertices.push_back(pos);
    gridVertices.push_back(0.4f);
    gridVertices.push_back(0.4f);
    gridVertices.push_back(0.4f);

    gridVertices.push_back(static_cast<float>(gridSize) * gridStep);
    gridVertices.push_back(gridY);
    gridVertices.push_back(pos);
    gridVertices.push_back(0.4f);
    gridVertices.push_back(0.4f);
    gridVertices.push_back(0.4f);
  }

  m_GridVertexCount = static_cast<uint32_t>(gridVertices.size() / 6);

  m_GridVAO = BeEngine::VertexArray::Create();
  auto vb = BeEngine::VertexBuffer::Create(
      gridVertices.data(),
      static_cast<uint32_t>(gridVertices.size() * sizeof(float)));

  vb->SetLayout({{BeEngine::ShaderDataType::Float3, "a_Position"},
                 {BeEngine::ShaderDataType::Float3, "a_Color"}});
  m_GridVAO->AddVertexBuffer(vb);

  // Use same shader as cube (position + color)
  std::string vertexSrc = R"(
    #version 410 core
    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec3 a_Color;

    uniform mat4 u_ViewProjection;
    uniform mat4 u_Model;

    out vec3 v_Color;

    void main() {
      v_Color = a_Color;
      gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
    }
  )";

  std::string fragmentSrc = R"(
    #version 410 core
    in vec3 v_Color;
    layout(location = 0) out vec4 o_Color;

    void main() {
      o_Color = vec4(v_Color, 1.0);
    }
  )";

  m_GridShader = BeEngine::Shader::Create(vertexSrc, fragmentSrc);

  BE_INFO("Grid setup complete with {} vertices", m_GridVertexCount);
}
