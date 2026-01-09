// SandboxLayer3D.cpp
#include "SandboxLayer3D.hpp"
#include "MaterialSystem/Texture.hpp"
#include <vector>

void SandboxLayer3D::OnAttach() {
  BE_INFO("SandboxLayer3D attached - setting up 3D scene");

  float aspectRatio = 1280.0F / 720.0F;

  BE_INFO("Creating camera controller...");
  m_CameraController = std::make_unique<BeEngine::PerspectiveCameraController>(
      aspectRatio, 45.0F, 0.1F, 1000.0F);

  m_CameraController->SetPosition({3.0F, 2.0F, 5.0F});
  m_CameraController->SetYaw(-120.0F);
  m_CameraController->SetPitch(-15.0F);
  BE_INFO("Camera controller created!");

  m_CubeTransform.SetPosition(0.0F, 0.5F, 0.0F);
  m_CubeEulerAngles = {0.0F, 0.0F, 0.0F};

  BE_INFO("Creating framebuffer...");
  BeEngine::FramebufferSpecification fbSpec;
  fbSpec.Width = 1280;
  fbSpec.Height = 720;
  fbSpec.Attachments = {BeEngine::FramebufferTextureFormat::RGBA8,
                        BeEngine::FramebufferTextureFormat::Depth};
  m_Framebuffer = BeEngine::Framebuffer::Create(fbSpec);
  BE_INFO("Framebuffer created!");

  BE_INFO("Loading textures...");
  m_CheckerTexture = BeEngine::Texture2D::CreateCheckerboard(256, 256, 32);
  BE_INFO("Checker texture created!");

  m_CubeTexture = BeEngine::Texture2D::Create("BeEngine/Assets/Brick.jpeg");
  if (!m_CubeTexture->IsLoaded()) {
    BE_WARN("Failed to load brick texture, using checkerboard");
    m_CubeTexture = BeEngine::Texture2D::CreateCheckerboard(256, 256, 32);
  }
  BE_INFO("Textures loaded!");

  BE_INFO("Creating cube mesh...");
  m_CubeMesh = BeEngine::MeshFactory::CreateCube();
  if (!m_CubeMesh) {
    BE_CRITICAL("Failed to create cube mesh!");
    return;
  }
  BE_INFO("Cube mesh created: {} vertices, {} indices",
          m_CubeMesh->GetVertexCount(), m_CubeMesh->GetIndexCount());

  BE_INFO("Creating sphere mesh...");
  m_SphereMesh = BeEngine::MeshFactory::CreateSphere(0.5F, 32, 16);
  if (!m_SphereMesh) {
    BE_CRITICAL("Failed to create sphere mesh!");
    return;
  }
  BE_INFO("Sphere mesh created!");

  BE_INFO("Creating plane mesh...");
  m_PlaneMesh = BeEngine::MeshFactory::CreatePlane(10.0F, 10.0F, 1);
  if (!m_PlaneMesh) {
    BE_CRITICAL("Failed to create plane mesh!");
    return;
  }
  BE_INFO("Plane mesh created!");

  BE_INFO("Setting up grid...");
  SetupGrid();
  BE_INFO("Grid setup complete!");

  // =====================================================
  // MATERIAL SYSTEM - COULD ALSO CRASH HERE
  // =====================================================
  BE_INFO("Creating shader...");

  std::string vertexSrc = R"(
      #version 410 core
      layout(location = 0) in vec3 a_Position;
      layout(location = 1) in vec3 a_Normal;
      layout(location = 2) in vec4 a_Tangent;
      layout(location = 3) in vec2 a_TexCoord;
      layout(location = 4) in vec4 a_Color;

      uniform mat4 u_ViewProjection;
      uniform mat4 u_Model;

      out vec3 v_Position;
      out vec3 v_Normal;
      out vec2 v_TexCoord;
      out vec4 v_Color;

      void main() {
          v_Position = vec3(u_Model * vec4(a_Position, 1.0));
          v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;
          v_TexCoord = a_TexCoord;
          v_Color = a_Color;
          gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
      }
  )";

  std::string fragmentSrc = R"(
      #version 410 core
      in vec3 v_Position;
      in vec3 v_Normal;
      in vec2 v_TexCoord;
      in vec4 v_Color;

      uniform sampler2D u_Texture;
      uniform bool u_UseTexture;
      uniform vec4 u_TintColor;

      layout(location = 0) out vec4 o_Color;

      void main() {
          vec4 texColor = u_UseTexture ? texture(u_Texture, v_TexCoord) : v_Color;
          o_Color = texColor * u_TintColor;
      }
  )";

  auto cubeShader =
      m_ShaderLibrary.Load("TexturedShader", vertexSrc, fragmentSrc);
  if (!cubeShader) {
    BE_CRITICAL("Failed to create shader!");
    return;
  }
  BE_INFO("Shader created!");

  BE_INFO("Creating material...");
  m_CubeMaterial = m_MaterialLibrary.Create("BrickMaterial", cubeShader);
  if (!m_CubeMaterial) {
    BE_CRITICAL("Failed to create material!");
    return;
  }

  m_CubeMaterial->SetTexture2D("u_Texture", m_CubeTexture);
  m_CubeMaterial->SetBool("u_UseTexture", true);
  m_CubeMaterial->SetFloat4("u_TintColor", glm::vec4(1.0F, 1.0F, 1.0F, 1.0F));
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
  // Check for material and mesh instead of old shader/VAO
  if (!m_CubeMaterial || !m_CubeMesh || !m_Framebuffer) {
    return;
  }

  m_Framebuffer->Bind();

  glClearColor(0.15F, 0.15F, 0.2F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  auto viewProj = m_CameraController->GetCamera().GetViewProjectionMatrix();

  // Draw Grid
  if (m_GridShader && m_GridVAO) {
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    m_GridShader->Bind();
    m_GridShader->SetMat4("u_ViewProjection", viewProj);
    m_GridShader->SetMat4("u_Model", glm::mat4(1.0F));
    m_GridVAO->Bind();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_GridVertexCount));
  }

  // =====================================================
  // DRAW CUBE USING MESH + MATERIAL SYSTEM
  // =====================================================
  {
    glm::mat4 model = m_CubeTransform.GetWorldMatrix();

    // Material::Bind() handles:
    // - Shader binding
    // - Uniform uploads
    // - Render state (cull, blend, depth)
    m_CubeMaterial->Bind();

    // Set per-object uniforms
    m_CubeMaterial->GetShader()->SetMat4("u_ViewProjection", viewProj);
    m_CubeMaterial->GetShader()->SetMat4("u_Model", model);

    // Bind mesh and draw
    m_CubeMesh->Bind();
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(m_CubeMesh->GetIndexCount()),
                   GL_UNSIGNED_INT, nullptr);
  }

  // =====================================================
  // DRAW SPHERE (to the right of the cube)
  // =====================================================
  {
    glm::mat4 model =
        glm::translate(glm::mat4(1.0F), glm::vec3(2.0F, 0.5F, 0.0F));
    m_CubeMaterial->GetShader()->SetMat4("u_Model", model);

    m_SphereMesh->Bind();
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(m_SphereMesh->GetIndexCount()),
                   GL_UNSIGNED_INT, nullptr);
  }

  // =====================================================
  // DRAW PLANE (as ground - already have grid, but this shows it works)
  // =====================================================
  {
    glm::mat4 model =
        glm::translate(glm::mat4(1.0F), glm::vec3(0.0F, -0.01F, 0.0F));
    m_CubeMaterial->GetShader()->SetMat4("u_Model", model);

    // Temporarily disable texture for plane to see it differently
    m_CubeMaterial->GetShader()->SetBool("u_UseTexture", false);
    m_CubeMaterial->GetShader()->SetFloat4("u_TintColor",
                                           glm::vec4(0.3F, 0.3F, 0.35F, 1.0F));

    m_PlaneMesh->Bind();
    glDrawElements(GL_TRIANGLES,
                   static_cast<GLsizei>(m_PlaneMesh->GetIndexCount()),
                   GL_UNSIGNED_INT, nullptr);

    // Restore for next frame
    m_CubeMaterial->GetShader()->SetBool("u_UseTexture", true);
    m_CubeMaterial->GetShader()->SetFloat4("u_TintColor", glm::vec4(1.0F));
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
  auto camPos = m_CameraController->GetPosition();
  ImGui::Text("Position: (%.2f, %.2f, %.2f)", camPos.x, camPos.y, camPos.z);
  ImGui::Text("Yaw: %.1f°  Pitch: %.1f°", m_CameraController->GetYaw(),
              m_CameraController->GetPitch());

  float speed = m_CameraController->GetMoveSpeed();
  bool sprinting = m_CameraController->IsSprinting();
  if (sprinting) {
    ImGui::TextColored(ImVec4(1.0F, 0.5F, 0.0F, 1.0F),
                       "Speed: %.1f (SPRINTING)",
                       speed * m_CameraController->GetSprintMultiplier());
  } else {
    ImGui::Text("Speed: %.1f", speed);
  }

  ImGui::Text("FOV: %.1f°", m_CameraController->GetCamera().GetFOV());
  ImGui::End();

  // Material Properties Window
  ImGui::Begin("Material");

  ImGui::Text("Material: %s", m_CubeMaterial->GetName().c_str());
  ImGui::Separator();

  // Tint color picker
  static auto tintColor = glm::vec4(1.0F);
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
  std::array<const char *, 3> cullModes = {"Back", "Front", "None"};
  if (ImGui::Combo("Cull Mode", &cullMode, cullModes.data(), 3)) {
    m_CubeMaterial->SetCullMode(
        static_cast<BeEngine::Material::CullMode>(cullMode));
  }

  // Blend mode selector
  static int blendMode = 0;
  std::array<const char *, 4> blendModes = {"Opaque", "Transparent", "Additive",
                                            "Multiply"};
  if (ImGui::Combo("Blend Mode", &blendMode, blendModes.data(), 4)) {
    m_CubeMaterial->SetBlendMode(
        static_cast<BeEngine::Material::BlendMode>(blendMode));
  }

  ImGui::End();

  // Scene window
  ImGui::Begin("Scene");

  ImGui::Text("Cube Transform");

  glm::vec3 cubePos = m_CubeTransform.GetPosition();
  if (ImGui::DragFloat3("Position", &cubePos.x, 0.1F)) {
    m_CubeTransform.SetPosition(cubePos);
  }

  if (ImGui::DragFloat3("Rotation", &m_CubeEulerAngles.x, 1.0F)) {
    m_CubeTransform.SetRotation(m_CubeEulerAngles);
  }

  glm::vec3 scale = m_CubeTransform.GetScale();
  if (ImGui::DragFloat3("Scale", &scale.x, 0.1F, 0.1F, 10.0F)) {
    m_CubeTransform.SetScale(scale);
  }

  ImGui::Checkbox("Auto Rotate", &m_AutoRotate);

  if (ImGui::Button("Reset Cube")) {
    m_CubeTransform.Reset();
    m_CubeTransform.SetPosition(0.0F, 0.5F, 0.0F);
    m_CubeEulerAngles = {0.0F, 0.0F, 0.0F};
  }

  ImGui::Separator();

  // Mesh info
  ImGui::Text("Mesh Info");
  ImGui::Text("  Vertices: %u", m_CubeMesh->GetVertexCount());
  ImGui::Text("  Triangles: %u", m_CubeMesh->GetTriangleCount());

  auto bounds = m_CubeMesh->GetBoundingBox();
  ImGui::Text("  Bounds: (%.2f, %.2f, %.2f) - (%.2f, %.2f, %.2f)", bounds.Min.x,
              bounds.Min.y, bounds.Min.z, bounds.Max.x, bounds.Max.y,
              bounds.Max.z);

  ImGui::Separator();

  if (ImGui::Button("Reset Camera")) {
    m_CameraController->SetPosition({3.0F, 2.0F, 5.0F});
    m_CameraController->SetYaw(-120.0F);
    m_CameraController->SetPitch(-15.0F);
  }

  ImGui::End();
}

void SandboxLayer3D::SetupGrid() {
  BE_INFO("Setting up grid...");

  std::vector<float> gridVertices;
  const int gridSize = 10;
  const float gridStep = 1.0F;
  const float gridY = 0.0F;

  // Generate grid lines
  for (int i = -gridSize; i <= gridSize; ++i) {
    float pos = static_cast<float>(i) * gridStep;

    // Lines parallel to Z axis
    gridVertices.push_back(pos);
    gridVertices.push_back(gridY);
    gridVertices.push_back(static_cast<float>(-gridSize) * gridStep);
    gridVertices.push_back(0.4F); // Color
    gridVertices.push_back(0.4F);
    gridVertices.push_back(0.4F);

    gridVertices.push_back(pos);
    gridVertices.push_back(gridY);
    gridVertices.push_back(static_cast<float>(gridSize) * gridStep);
    gridVertices.push_back(0.4F);
    gridVertices.push_back(0.4F);
    gridVertices.push_back(0.4F);

    // Lines parallel to X axis
    gridVertices.push_back(static_cast<float>(-gridSize) * gridStep);
    gridVertices.push_back(gridY);
    gridVertices.push_back(pos);
    gridVertices.push_back(0.4F);
    gridVertices.push_back(0.4F);
    gridVertices.push_back(0.4F);

    gridVertices.push_back(static_cast<float>(gridSize) * gridStep);
    gridVertices.push_back(gridY);
    gridVertices.push_back(pos);
    gridVertices.push_back(0.4F);
    gridVertices.push_back(0.4F);
    gridVertices.push_back(0.4F);
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
