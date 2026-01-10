// SandboxLayer3D.cpp
#include "SandboxLayer3D.hpp"
#include "MaterialSystem/Texture.hpp"
#include <vector>

void SandboxLayer3D::OnAttach() {
  BE_INFO("SandboxLayer3D attached - setting up 3D scene");

  float aspectRatio = 1280.0F / 720.0F;

  // Camera setup
  m_CameraController = std::make_unique<BeEngine::PerspectiveCameraController>(
      aspectRatio, 45.0F, 0.1F, 1000.0F);
  m_CameraController->SetPosition({3.0F, 2.0F, 5.0F});
  m_CameraController->SetYaw(-120.0F);
  m_CameraController->SetPitch(-15.0F);

  // Transforms
  m_CubeTransform.SetPosition(0.0F, 0.5F, 0.0F);
  m_SphereTransform.SetPosition(-2.0F, 0.5F, 0.0F);
  m_CubeEulerAngles = {0.0F, 0.0F, 0.0F};

  // Framebuffer
  BeEngine::FramebufferSpecification fbSpec;
  fbSpec.Width = 1280;
  fbSpec.Height = 720;
  fbSpec.Attachments = {BeEngine::FramebufferTextureFormat::RGBA8,
                        BeEngine::FramebufferTextureFormat::Depth};
  m_Framebuffer = BeEngine::Framebuffer::Create(fbSpec);

  // Textures
  m_CheckerTexture = BeEngine::Texture2D::CreateCheckerboard(256, 256, 32);
  m_CubeTexture = BeEngine::Texture2D::Create("BeEngine/Assets/Brick.jpeg");
  if (!m_CubeTexture->IsLoaded()) {
    BE_WARN("Failed to load brick texture, using checkerboard");
    m_CubeTexture = m_CheckerTexture;
  }

  // Meshes
  m_CubeMesh = BeEngine::MeshFactory::CreateCube();
  m_SphereMesh = BeEngine::MeshFactory::CreateSphere(0.5F, 32, 16);
  m_PlaneMesh = BeEngine::MeshFactory::CreatePlane(10.0F, 10.0F, 1);

  // Grid (separate VAO with simple vertex format)
  SetupGrid();

  // =====================================================
  // LIGHTING SETUP
  // =====================================================
  m_LightManager.GetAmbientLight().Color = glm::vec3(0.15F, 0.15F, 0.18F);

  auto &dirLight = m_LightManager.GetDirectionalLight();
  dirLight.Direction = glm::vec3(-0.5F, -1.0F, -0.3F);
  dirLight.Color = glm::vec3(1.0F, 0.98F, 0.95F);
  dirLight.Intensity = 1.2F;

  BeEngine::PointLight pointLight;
  pointLight.Position = glm::vec3(2.0F, 2.0F, 2.0F);
  pointLight.Color = glm::vec3(1.0F, 0.7F, 0.4F);
  pointLight.Intensity = 3.0F;
  pointLight.Range = 10.0F;
  m_LightManager.AddPointLight(pointLight);

  // =====================================================
  // MATERIALS - Each object gets ONE material
  // =====================================================

  // PBR Material for the cube (metallic brick)
  m_PBRMat = BeEngine::CreateScope<BeEngine::PBRMaterial>("Brick PBR");
  m_PBRMat->SetAlbedoMap(m_CubeTexture);
  m_PBRMat->SetAlbedoColor(glm::vec4(1.0F));
  m_PBRMat->SetMetallic(0.1F);  // Brick is not metallic
  m_PBRMat->SetRoughness(0.8F); // Brick is rough

  // Phong Material for the sphere (shiny red)
  m_PhongMat = BeEngine::CreateScope<BeEngine::PhongMaterial>("Shiny Red");
  m_PhongMat->SetDiffuseColor(glm::vec4(0.8F, 0.2F, 0.2F, 1.0F));
  m_PhongMat->SetSpecularColor(glm::vec4(1.0F));
  m_PhongMat->SetShininess(64.0F);

  // Unlit Material for ground plane (flat gray)
  m_UnlitMat = BeEngine::CreateScope<BeEngine::UnlitMaterial>("Ground");
  m_UnlitMat->SetColor(glm::vec4(0.25F, 0.25F, 0.28F, 1.0F));

  BE_INFO("Scene initialized!");
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
  if (!m_Framebuffer)
    return;

  m_Framebuffer->Bind();

  glClearColor(0.1F, 0.1F, 0.12F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  auto viewProj = m_CameraController->GetCamera().GetViewProjectionMatrix();
  auto camPos = m_CameraController->GetPosition();

  // =====================================================
  // 1. DRAW GRID (uses its own simple shader)
  // =====================================================
  if (m_GridShader && m_GridVAO) {
    glDisable(GL_CULL_FACE);
    m_GridShader->Bind();
    m_GridShader->SetMat4("u_ViewProjection", viewProj);
    m_GridShader->SetMat4("u_Model", glm::mat4(1.0F));
    m_GridVAO->Bind();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_GridVertexCount));
  }

  // =====================================================
  // 2. DRAW GROUND PLANE (Unlit - no lighting)
  // =====================================================
  if (m_UnlitMat && m_PlaneMesh) {
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_UnlitMat->Bind();
    m_UnlitMat->GetShader()->SetMat4("u_ViewProjection", viewProj);

    glm::mat4 model =
        glm::translate(glm::mat4(1.0F), glm::vec3(0.0F, 0.0F, 0.0F));
    m_UnlitMat->GetShader()->SetMat4("u_Model", model);

    m_PlaneMesh->Bind();
    glDrawElements(GL_TRIANGLES, m_PlaneMesh->GetIndexCount(), GL_UNSIGNED_INT,
                   nullptr);
  }

  // =====================================================
  // 3. DRAW CUBE (PBR - realistic lighting)
  // =====================================================
  if (m_PBRMat && m_CubeMesh) {
    m_PBRMat->Bind();
    m_LightManager.UploadToShader(m_PBRMat->GetShader(), camPos);

    glm::mat4 model = m_CubeTransform.GetWorldMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    m_PBRMat->GetShader()->SetMat4("u_ViewProjection", viewProj);
    m_PBRMat->GetShader()->SetMat4("u_Model", model);
    m_PBRMat->GetShader()->SetMat3("u_NormalMatrix", normalMatrix);

    m_CubeMesh->Bind();
    glDrawElements(GL_TRIANGLES, m_CubeMesh->GetIndexCount(), GL_UNSIGNED_INT,
                   nullptr);
  }

  // =====================================================
  // 4. DRAW SPHERE (Phong - classic lighting)
  // =====================================================
  if (m_PhongMat && m_SphereMesh) {
    m_PhongMat->Bind();
    m_LightManager.UploadToShader(m_PhongMat->GetShader(), camPos);

    glm::mat4 model = m_SphereTransform.GetWorldMatrix();
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

    m_PhongMat->GetShader()->SetMat4("u_ViewProjection", viewProj);
    m_PhongMat->GetShader()->SetMat4("u_Model", model);
    m_PhongMat->GetShader()->SetMat3("u_NormalMatrix", normalMatrix);

    m_SphereMesh->Bind();
    glDrawElements(GL_TRIANGLES, m_SphereMesh->GetIndexCount(), GL_UNSIGNED_INT,
                   nullptr);
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
  ImGui::Text("Yaw: %.1f  Pitch: %.1f", m_CameraController->GetYaw(),
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

  ImGui::Text("FOV: %.1f", m_CameraController->GetCamera().GetFOV());
  ImGui::End();

  // =====================================================
  // PBR MATERIAL PROPERTIES (Cube)
  // =====================================================
  ImGui::Begin("PBR Material (Cube)");

  if (m_PBRMat) {
    ImGui::Text("Material: %s", m_PBRMat->GetName().c_str());
    ImGui::Separator();

    // Albedo color
    static glm::vec4 albedoColor = m_PBRMat->GetAlbedoColor();
    if (ImGui::ColorEdit4("Albedo Color", &albedoColor.x)) {
      m_PBRMat->SetAlbedoColor(albedoColor);
    }

    // Metallic slider
    static float metallic = m_PBRMat->GetMetallic();
    if (ImGui::SliderFloat("Metallic", &metallic, 0.0F, 1.0F)) {
      m_PBRMat->SetMetallic(metallic);
    }

    // Roughness slider
    static float roughness = m_PBRMat->GetRoughness();
    if (ImGui::SliderFloat("Roughness", &roughness, 0.04F, 1.0F)) {
      m_PBRMat->SetRoughness(roughness);
    }

    // AO slider
    static float ao = m_PBRMat->GetAO();
    if (ImGui::SliderFloat("Ambient Occlusion", &ao, 0.0F, 1.0F)) {
      m_PBRMat->SetAO(ao);
    }
  }

  ImGui::End();

  // =====================================================
  // PHONG MATERIAL PROPERTIES (Sphere)
  // =====================================================
  ImGui::Begin("Phong Material (Sphere)");

  if (m_PhongMat) {
    ImGui::Text("Material: %s", m_PhongMat->GetName().c_str());
    ImGui::Separator();

    // Diffuse color
    static glm::vec4 diffuseColor = m_PhongMat->GetDiffuseColor();
    if (ImGui::ColorEdit4("Diffuse Color", &diffuseColor.x)) {
      m_PhongMat->SetDiffuseColor(diffuseColor);
    }

    // Specular color
    static glm::vec4 specularColor = m_PhongMat->GetSpecularColor();
    if (ImGui::ColorEdit4("Specular Color", &specularColor.x)) {
      m_PhongMat->SetSpecularColor(specularColor);
    }

    // Shininess slider
    static float shininess = m_PhongMat->GetShininess();
    if (ImGui::SliderFloat("Shininess", &shininess, 1.0F, 256.0F)) {
      m_PhongMat->SetShininess(shininess);
    }
  }

  ImGui::End();

  // =====================================================
  // LIGHTING
  // =====================================================
  ImGui::Begin("Lighting");

  // Ambient
  ImGui::Text("Ambient Light");
  auto &ambient = m_LightManager.GetAmbientLight();
  ImGui::ColorEdit3("Ambient Color", &ambient.Color.x);

  ImGui::Separator();

  // Directional Light
  ImGui::Text("Directional Light");
  auto &dirLight = m_LightManager.GetDirectionalLight();
  ImGui::Checkbox("Dir Light Enabled", &dirLight.Enabled);
  ImGui::DragFloat3("Direction", &dirLight.Direction.x, 0.01F);
  ImGui::ColorEdit3("Dir Color", &dirLight.Color.x);
  ImGui::SliderFloat("Dir Intensity", &dirLight.Intensity, 0.0F, 5.0F);

  ImGui::Separator();

  // Point Lights
  ImGui::Text("Point Lights (%d)", m_LightManager.GetPointLightCount());
  auto &pointLights = m_LightManager.GetPointLights();
  for (size_t i = 0; i < pointLights.size(); i++) {
    ImGui::PushID(static_cast<int>(i));
    if (ImGui::TreeNode("Point Light", "Point Light %zu", i)) {
      ImGui::Checkbox("Enabled", &pointLights[i].Enabled);
      ImGui::DragFloat3("Position", &pointLights[i].Position.x, 0.1F);
      ImGui::ColorEdit3("Color", &pointLights[i].Color.x);
      ImGui::SliderFloat("Intensity", &pointLights[i].Intensity, 0.0F, 10.0F);
      ImGui::SliderFloat("Range", &pointLights[i].Range, 1.0F, 50.0F);
      ImGui::TreePop();
    }
    ImGui::PopID();
  }

  ImGui::End();

  // =====================================================
  // SCENE (Transforms)
  // =====================================================
  ImGui::Begin("Scene");

  ImGui::Text("Cube Transform");
  glm::vec3 cubePos = m_CubeTransform.GetPosition();
  if (ImGui::DragFloat3("Cube Position", &cubePos.x, 0.1F)) {
    m_CubeTransform.SetPosition(cubePos);
  }

  if (ImGui::DragFloat3("Cube Rotation", &m_CubeEulerAngles.x, 1.0F)) {
    m_CubeTransform.SetRotation(m_CubeEulerAngles);
  }

  glm::vec3 cubeScale = m_CubeTransform.GetScale();
  if (ImGui::DragFloat3("Cube Scale", &cubeScale.x, 0.1F, 0.1F, 10.0F)) {
    m_CubeTransform.SetScale(cubeScale);
  }

  ImGui::Checkbox("Auto Rotate", &m_AutoRotate);

  ImGui::Separator();

  ImGui::Text("Sphere Transform");
  glm::vec3 spherePos = m_SphereTransform.GetPosition();
  if (ImGui::DragFloat3("Sphere Position", &spherePos.x, 0.1F)) {
    m_SphereTransform.SetPosition(spherePos);
  }

  ImGui::Separator();

  if (ImGui::Button("Reset Scene")) {
    m_CubeTransform.Reset();
    m_CubeTransform.SetPosition(0.0F, 0.5F, 0.0F);
    m_CubeEulerAngles = {0.0F, 0.0F, 0.0F};

    m_SphereTransform.Reset();
    m_SphereTransform.SetPosition(-2.0F, 0.5F, 0.0F);

    m_CameraController->SetPosition({3.0F, 2.0F, 5.0F});
    m_CameraController->SetYaw(-120.0F);
    m_CameraController->SetPitch(-15.0F);
  }

  ImGui::Separator();

  // Mesh info
  if (m_CubeMesh) {
    ImGui::Text("Cube Mesh: %u verts, %u tris", m_CubeMesh->GetVertexCount(),
                m_CubeMesh->GetTriangleCount());
  }
  if (m_SphereMesh) {
    ImGui::Text("Sphere Mesh: %u verts, %u tris",
                m_SphereMesh->GetVertexCount(),
                m_SphereMesh->GetTriangleCount());
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
