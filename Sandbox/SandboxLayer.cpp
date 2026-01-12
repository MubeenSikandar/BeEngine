// SandboxLayer2D.cpp
#include "SandboxLayer2D.hpp"
#include <array>

// ============================================================================
// Lifecycle
// ============================================================================

void SandboxLayer2D::OnAttach() {
  BE_INFO("SandboxLayer2D attached - initializing 2D Scene");

  float aspectRatio = m_ViewportSize.x / m_ViewportSize.y;

  // Orthographic camera for 2D
  m_CameraController =
      BeEngine::CreateScope<BeEngine::OrthographicCameraController>(aspectRatio,
                                                                    1.0F, true);

  // Framebuffer
  BeEngine::FramebufferSpecification fbSpec;
  fbSpec.Width = static_cast<uint32_t>(m_ViewportSize.x);
  fbSpec.Height = static_cast<uint32_t>(m_ViewportSize.y);
  fbSpec.Attachments = {BeEngine::FramebufferTextureFormat::RGBA8,
                        BeEngine::FramebufferTextureFormat::Depth};
  m_Framebuffer = BeEngine::Framebuffer::Create(fbSpec);

  // Setup quad for 2D rendering
  std::array<float, 20> quadVertices = {
      // Position         // TexCoord
      -0.5F, -0.5F, 0.0F, 0.0F, 0.0F, 0.5F,  -0.5F, 0.0F, 1.0F, 0.0F,
      0.5F,  0.5F,  0.0F, 1.0F, 1.0F, -0.5F, 0.5F,  0.0F, 0.0F, 1.0F};

  std::array<uint32_t, 6> quadIndices = {0, 1, 2, 2, 3, 0};

  m_QuadVAO = BeEngine::VertexArray::Create();

  auto vb = BeEngine::VertexBuffer::Create(
      quadVertices.data(),
      static_cast<uint32_t>(quadVertices.size() * sizeof(float)));
  vb->SetLayout({{BeEngine::ShaderDataType::Float3, "a_Position"},
                 {BeEngine::ShaderDataType::Float2, "a_TexCoord"}});
  m_QuadVAO->AddVertexBuffer(vb);

  auto ib = BeEngine::IndexBuffer::Create(
      quadIndices.data(), static_cast<uint32_t>(quadIndices.size()));
  m_QuadVAO->SetIndexBuffer(ib);

  // Simple 2D shader
  std::string vertexSrc = R"(
    #version 410 core
    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec2 a_TexCoord;

    uniform mat4 u_ViewProjection;
    uniform mat4 u_Model;

    out vec2 v_TexCoord;

    void main() {
      v_TexCoord = a_TexCoord;
      gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
    }
  )";

  std::string fragmentSrc = R"(
    #version 410 core
    in vec2 v_TexCoord;

    uniform vec4 u_Color;

    layout(location = 0) out vec4 o_Color;

    void main() {
      o_Color = u_Color;
    }
  )";

  m_SpriteShader = BeEngine::Shader::Create(vertexSrc, fragmentSrc);

  // Setup scene
  SetupScene();

  BE_INFO("2D Scene initialized!");
}

void SandboxLayer2D::OnDetach() { BE_INFO("SandboxLayer2D detached"); }

// ============================================================================
// Scene Setup
// ============================================================================

void SandboxLayer2D::SetupScene() {
  m_ActiveScene = &m_SceneManager.CreateScene("2D Demo Scene");

  // Note: For a full 2D implementation, you'd want:
  // - SpriteRendererComponent
  // - Sprite2DSystem
  // - 2D Physics components (Box2D integration)
  // - Tilemap components

  // For now, we'll just create some basic entities
  // that we'll render manually with quads

  // Player entity
  {
    BeEngine::Entity player = m_ActiveScene->CreateEntity("Player");
    auto &transform = player.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({0.0F, 0.0F, 0.0F});
    transform.SetScale({1.0F, 1.0F, 1.0F});
  }

  // Some "sprite" entities (positioned in 2D space)
  for (int i = 0; i < 5; i++) {
    BeEngine::Entity sprite =
        m_ActiveScene->CreateEntity("Sprite " + std::to_string(i));
    auto &transform = sprite.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({static_cast<float>(i - 2) * 1.5F,
                           static_cast<float>(rand() % 100) / 100.0F - 0.5F,
                           0.0F});
    transform.SetScale({0.8F, 0.8F, 1.0F});
  }
}

// ============================================================================
// Update
// ============================================================================

void SandboxLayer2D::OnUpdate(BeEngine::Timestep ts) {
  // Update camera
  if (m_ViewportFocused) {
    m_CameraController->OnUpdate(ts);
  }

  // Update scene
  if (m_ActiveScene) {
    m_ActiveScene->OnUpdate(ts.GetSeconds());
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

void SandboxLayer2D::OnEvent(BeEngine::Event &event) {
  m_CameraController->OnEvent(event);
}

// ============================================================================
// Rendering
// ============================================================================

void SandboxLayer2D::OnRender() {
  if (!m_Framebuffer || !m_ActiveScene) {
    return;
  }

  m_Framebuffer->Bind();

  glClearColor(0.15F, 0.15F, 0.18F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  auto viewProj = m_CameraController->GetCamera().GetViewProjectionMatrix();

  // Render all entities as colored quads
  m_SpriteShader->Bind();
  m_SpriteShader->SetMat4("u_ViewProjection", viewProj);
  m_QuadVAO->Bind();

  // Colors for sprites
  std::array<glm::vec4, 6> colors = {
      glm::vec4(0.8F, 0.2F, 0.2F, 1.0F), // Red
      glm::vec4(0.2F, 0.8F, 0.2F, 1.0F), // Green
      glm::vec4(0.2F, 0.2F, 0.8F, 1.0F), // Blue
      glm::vec4(0.8F, 0.8F, 0.2F, 1.0F), // Yellow
      glm::vec4(0.8F, 0.2F, 0.8F, 1.0F), // Magenta
      glm::vec4(0.2F, 0.8F, 0.8F, 1.0F)  // Cyan
  };

  int colorIndex = 0;
  for (BeEngine::Entity entity : m_ActiveScene->GetAllEntities()) {
    auto &transform = entity.GetComponent<BeEngine::TransformComponent>();

    // Create model matrix from transform
    glm::mat4 model = glm::mat4(1.0F);
    model = glm::translate(model, transform.GetPosition());
    model = model * glm::mat4_cast(transform.GetRotation());
    model = glm::scale(model, transform.GetScale());

    m_SpriteShader->SetMat4("u_Model", model);
    m_SpriteShader->SetFloat4("u_Color", colors[colorIndex % colors.size()]);
    colorIndex++;

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
  }

  m_Framebuffer->Unbind();
}

// ============================================================================
// ImGui
// ============================================================================

void SandboxLayer2D::OnImGuiRender() {
  DrawViewportPanel();
  DrawSceneHierarchyPanel();

  // Camera info
  ImGui::Begin("2D Camera");
  auto pos = m_CameraController->GetPosition();
  ImGui::Text("Position: (%.2f, %.2f)", pos.x, pos.y);
  ImGui::Text("Rotation: %.1f°", m_CameraController->GetRotation());
  ImGui::Text("Zoom: %.2fx", m_CameraController->GetZoom());

  ImGui::Separator();
  ImGui::Text("Controls:");
  ImGui::Text("  WASD/Arrows - Move");
  ImGui::Text("  Q/E         - Rotate");
  ImGui::Text("  Scroll      - Zoom");
  ImGui::End();
}

void SandboxLayer2D::DrawViewportPanel() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("2D Viewport");

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
}

void SandboxLayer2D::DrawSceneHierarchyPanel() {
  ImGui::Begin("2D Scene");

  if (m_ActiveScene) {
    ImGui::Text("Scene: %s", m_ActiveScene->GetName().c_str());
    ImGui::Text("Entities: %zu", m_ActiveScene->GetEntityCount());
    ImGui::Separator();

    for (BeEngine::Entity entity : m_ActiveScene->GetAllEntities()) {
      auto &tag = entity.GetComponent<BeEngine::TagComponent>();
      auto &transform = entity.GetComponent<BeEngine::TransformComponent>();

      if (ImGui::TreeNode(tag.Tag.c_str())) {
        glm::vec3 pos = transform.GetPosition();
        if (ImGui::DragFloat2("Position", &pos.x, 0.1F)) {
          transform.SetPosition(pos);
        }

        glm::vec3 scale = transform.GetScale();
        if (ImGui::DragFloat2("Scale", &scale.x, 0.1F)) {
          transform.SetScale(scale);
        }

        float rotation =
            glm::degrees(glm::eulerAngles(transform.GetRotation()).z);
        if (ImGui::SliderFloat("Rotation", &rotation, -180.0F, 180.0F)) {
          transform.SetRotation({0.0F, 0.0F, glm::radians(rotation)});
        }

        ImGui::TreePop();
      }
    }
  }

  ImGui::End();
}
