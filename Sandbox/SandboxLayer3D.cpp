// SandboxLayer3D.cpp
#include "SandboxLayer3D.hpp"
#include "MaterialSystem/Texture.hpp"
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

  m_CheckerTexture = BeEngine::Texture2D::CreateCheckerboard(256, 256, 32);
  m_CubeTexture = BeEngine::Texture2D::Create("BeEngine/Assets/Brick.jpeg");

  if (!m_CubeTexture->IsLoaded()) {
    BE_WARN("Failed to load brick texture, using checkerboard");
    m_CubeTexture = BeEngine::Texture2D::CreateCheckerboard(256, 256, 32);
  }

  // BE_INFO("Created checkerboard texture!");

  SetupCube();
  SetupGrid();

  // =====================================================
  // MATERIAL SYSTEM USAGE
  // =====================================================

  // 1. Define shader source
  std::string vertexSrc = R"(
        #version 410 core
        layout(location = 0) in vec3 a_Position;
        layout(location = 1) in vec3 a_Color;
        layout(location = 2) in vec2 a_TexCoord;

        uniform mat4 u_ViewProjection;
        uniform mat4 u_Model;

        out vec3 v_Color;
        out vec2 v_TexCoord;

        void main() {
            v_Color = a_Color;
            v_TexCoord = a_TexCoord;
            gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
        }
    )";

  std::string fragmentSrc = R"(
        #version 410 core
        in vec3 v_Color;
        in vec2 v_TexCoord;

        uniform sampler2D u_Texture;
        uniform bool u_UseTexture;
        uniform vec4 u_TintColor;

        layout(location = 0) out vec4 o_Color;

        void main() {
            vec4 texColor = u_UseTexture ? texture(u_Texture, v_TexCoord) : vec4(v_Color, 1.0);
            o_Color = texColor * u_TintColor;
        }
    )";

  // 2. Load shader into library
  auto cubeShader =
      m_ShaderLibrary.Load("TexturedShader", vertexSrc, fragmentSrc);

  // 3. Create material with shader
  m_CubeMaterial = m_MaterialLibrary.Create("BrickMaterial", cubeShader);

  // 4. Set material properties
  m_CubeMaterial->SetTexture2D("u_Texture", m_CubeTexture);
  m_CubeMaterial->SetBool("u_UseTexture", true);
  m_CubeMaterial->SetFloat4("u_TintColor",
                            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // No tint

  // 5. Set render state
  m_CubeMaterial->SetCullMode(BeEngine::Material::CullMode::Back);
  m_CubeMaterial->SetDepthTest(true);
  m_CubeMaterial->SetDepthWrite(true);
  m_CubeMaterial->SetBlendMode(BeEngine::Material::BlendMode::Opaque);

  BE_INFO("Material system initialized!");
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
  glDisable(GL_CULL_FACE);

  auto viewProj = m_CameraController->GetCamera().GetViewProjectionMatrix();

  // Draw Grid
  if (m_GridShader && m_GridVAO) {
    m_GridShader->Bind();
    m_GridShader->SetMat4("u_ViewProjection", viewProj);
    m_GridShader->SetMat4("u_Model", glm::mat4(1.0F));
    m_GridVAO->Bind();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_GridVertexCount));
  }

  // =====================================================
  // DRAW CUBE USING MATERIAL SYSTEM
  // =====================================================
  {
    glm::mat4 model = m_CubeTransform.GetWorldMatrix();

    // Material::Bind() does ALL of this automatically:
    // - Binds the shader
    // - Uploads all uniforms (textures, floats, bools, etc.)
    // - Sets render state (culling, blending, depth)
    m_CubeMaterial->Bind();

    // Only set per-object uniforms (transform matrices)
    m_CubeMaterial->GetShader()->SetMat4("u_ViewProjection", viewProj);
    m_CubeMaterial->GetShader()->SetMat4("u_Model", model);

    m_CubeVAO->Bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
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
  ImGui::Text("Material Properties");

  // =====================================================
  // EDIT MATERIAL PROPERTIES IN REAL-TIME
  // =====================================================

  // Tint color picker
  static glm::vec4 tintColor = glm::vec4(1.0f);
  if (ImGui::ColorEdit4("Tint Color", &tintColor.x)) {
    m_CubeMaterial->SetFloat4("u_TintColor", tintColor);
  }

  // Toggle texture on/off
  static bool useTexture = true;
  if (ImGui::Checkbox("Use Texture", &useTexture)) {
    m_CubeMaterial->SetBool("u_UseTexture", useTexture);
  }

  // Cull mode selector
  static int cullMode = 0;
  const char *cullModes[] = {"Back", "Front", "None"};
  if (ImGui::Combo("Cull Mode", &cullMode, cullModes, 3)) {
    m_CubeMaterial->SetCullMode(
        static_cast<BeEngine::Material::CullMode>(cullMode));
  }

  // Blend mode selector
  static int blendMode = 0;
  const char *blendModes[] = {"Opaque", "Transparent", "Additive", "Multiply"};
  if (ImGui::Combo("Blend Mode", &blendMode, blendModes, 4)) {
    m_CubeMaterial->SetBlendMode(
        static_cast<BeEngine::Material::BlendMode>(blendMode));
  }

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
  if (ImGui::DragFloat3("Position", &cubePos.x, 0.1F)) {
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

  // 24 vertices (4 per face, 6 faces)
  // Format: x, y, z, r, g, b, u, v
  std::vector<float> vertices = {
      // Front face (red)
      -0.5f,
      -0.5f,
      0.5f,
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      0.0f,
      0.5f,
      -0.5f,
      0.5f,
      1.0f,
      0.0f,
      0.0f,
      1.0f,
      0.0f,
      0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.0f,
      0.0f,
      1.0f,
      1.0f,
      -0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      1.0f,

      // Back face (green)
      0.5f,
      -0.5f,
      -0.5f,
      0.0f,
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      -0.5f,
      -0.5f,
      -0.5f,
      0.0f,
      1.0f,
      0.0f,
      1.0f,
      0.0f,
      -0.5f,
      0.5f,
      -0.5f,
      0.0f,
      1.0f,
      0.0f,
      1.0f,
      1.0f,
      0.5f,
      0.5f,
      -0.5f,
      0.0f,
      1.0f,
      0.0f,
      0.0f,
      1.0f,

      // Top face (blue)
      -0.5f,
      0.5f,
      0.5f,
      0.0f,
      0.0f,
      1.0f,
      0.0f,
      0.0f,
      0.5f,
      0.5f,
      0.5f,
      0.0f,
      0.0f,
      1.0f,
      1.0f,
      0.0f,
      0.5f,
      0.5f,
      -0.5f,
      0.0f,
      0.0f,
      1.0f,
      1.0f,
      1.0f,
      -0.5f,
      0.5f,
      -0.5f,
      0.0f,
      0.0f,
      1.0f,
      0.0f,
      1.0f,

      // Bottom face (yellow)
      -0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      1.0f,
      0.0f,
      0.0f,
      0.0f,
      0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      1.0f,
      0.0f,
      1.0f,
      0.0f,
      0.5f,
      -0.5f,
      0.5f,
      1.0f,
      1.0f,
      0.0f,
      1.0f,
      1.0f,
      -0.5f,
      -0.5f,
      0.5f,
      1.0f,
      1.0f,
      0.0f,
      0.0f,
      1.0f,

      // Right face (magenta)
      0.5f,
      -0.5f,
      0.5f,
      1.0f,
      0.0f,
      1.0f,
      0.0f,
      0.0f,
      0.5f,
      -0.5f,
      -0.5f,
      1.0f,
      0.0f,
      1.0f,
      1.0f,
      0.0f,
      0.5f,
      0.5f,
      -0.5f,
      1.0f,
      0.0f,
      1.0f,
      1.0f,
      1.0f,
      0.5f,
      0.5f,
      0.5f,
      1.0f,
      0.0f,
      1.0f,
      0.0f,
      1.0f,

      // Left face (cyan)
      -0.5f,
      -0.5f,
      -0.5f,
      0.0f,
      1.0f,
      1.0f,
      0.0f,
      0.0f,
      -0.5f,
      -0.5f,
      0.5f,
      0.0f,
      1.0f,
      1.0f,
      1.0f,
      0.0f,
      -0.5f,
      0.5f,
      0.5f,
      0.0f,
      1.0f,
      1.0f,
      1.0f,
      1.0f,
      -0.5f,
      0.5f,
      -0.5f,
      0.0f,
      1.0f,
      1.0f,
      0.0f,
      1.0f,
  };

  // Index buffer: 6 faces × 2 triangles × 3 vertices = 36 indices
  std::vector<uint32_t> indices = {
      // Front
      0,
      1,
      2,
      2,
      3,
      0,
      // Back
      4,
      5,
      6,
      6,
      7,
      4,
      // Top
      8,
      9,
      10,
      10,
      11,
      8,
      // Bottom
      12,
      13,
      14,
      14,
      15,
      12,
      // Right
      16,
      17,
      18,
      18,
      19,
      16,
      // Left
      20,
      21,
      22,
      22,
      23,
      20,
  };

  m_CubeVAO = BeEngine::VertexArray::Create();

  auto vb = BeEngine::VertexBuffer::Create(
      vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));

  vb->SetLayout({{BeEngine::ShaderDataType::Float3, "a_Position"},
                 {BeEngine::ShaderDataType::Float3, "a_Color"},
                 {BeEngine::ShaderDataType::Float2, "a_TexCoord"}});
  m_CubeVAO->AddVertexBuffer(vb);

  // Add index buffer
  auto ib = BeEngine::IndexBuffer::Create(
      indices.data(), static_cast<uint32_t>(indices.size()));
  m_CubeVAO->SetIndexBuffer(ib);

  std::string vertexSrc = R"(
      #version 410 core
      layout(location = 0) in vec3 a_Position;
      layout(location = 1) in vec3 a_Color;
      layout(location = 2) in vec2 a_TexCoord;

      uniform mat4 u_ViewProjection;
      uniform mat4 u_Model;

      out vec3 v_Color;
      out vec2 v_TexCoord;

      void main() {
          v_Color = a_Color;
          v_TexCoord = a_TexCoord;
          gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
      }
  )";

  std::string fragmentSrc = R"(
      #version 410 core
      in vec3 v_Color;
      in vec2 v_TexCoord;

      uniform sampler2D u_Texture;
      uniform bool u_UseTexture;

      layout(location = 0) out vec4 o_Color;

      void main() {
          if (u_UseTexture) {
              o_Color = texture(u_Texture, v_TexCoord);
          } else {
              o_Color = vec4(v_Color, 1.0);
          }
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
