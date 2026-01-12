// SandboxLayer3D.cpp
#include "SandboxLayer3D.hpp"

// ============================================================================
// Lifecycle
// ============================================================================

void SandboxLayer3D::OnAttach() {
  BE_INFO("SandboxLayer3D attached - initializing Scene System");

  // Editor camera for navigating the scene
  float aspectRatio = m_ViewportSize.x / m_ViewportSize.y;
  m_EditorCamera = BeEngine::CreateScope<BeEngine::PerspectiveCameraController>(
      aspectRatio, 45.0F, 0.1F, 1000.0F);
  m_EditorCamera->SetPosition({5.0F, 4.0F, 8.0F});
  m_EditorCamera->SetYaw(-145.0F);
  m_EditorCamera->SetPitch(-20.0F);

  // Framebuffer for viewport rendering
  BeEngine::FramebufferSpecification fbSpec;
  fbSpec.Width = static_cast<uint32_t>(m_ViewportSize.x);
  fbSpec.Height = static_cast<uint32_t>(m_ViewportSize.y);
  fbSpec.Attachments = {BeEngine::FramebufferTextureFormat::RGBA8,
                        BeEngine::FramebufferTextureFormat::Depth};
  m_Framebuffer = BeEngine::Framebuffer::Create(fbSpec);

  // Setup editor grid
  SetupGrid();

  // Setup the demo scene
  SetupScene();

  BE_INFO("Scene System initialized with {} entities",
          m_ActiveScene ? m_ActiveScene->GetEntityCount() : 0);
}

void SandboxLayer3D::OnDetach() { BE_INFO("SandboxLayer3D detached"); }

// ============================================================================
// Scene Setup
// ============================================================================

void SandboxLayer3D::SetupScene() {
  // Create a new scene
  m_ActiveScene = &m_SceneManager.CreateScene("Demo Scene");

  // Configure scene renderer's ambient light
  auto &lightManager = m_SceneRenderer.GetLightManager();
  lightManager.GetAmbientLight().Color = glm::vec3(0.15F, 0.15F, 0.18F);

  // ===== Camera Entity =====
  {
    BeEngine::Entity camera = m_ActiveScene->CreateEntity("Main Camera");
    auto &transform = camera.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({0.0F, 3.0F, 10.0F});
    transform.SetRotation({-10.0F, 0.0F, 0.0F});

    auto &cam = camera.AddComponent<BeEngine::CameraComponent>();
    cam.Primary = true;
    cam.FOV = 60.0F;
    cam.NearClip = 0.1F;
    cam.FarClip = 1000.0F;
  }

  // ===== Sun (Directional Light) =====
  {
    BeEngine::Entity sun = m_ActiveScene->CreateEntity("Sun");
    auto &transform = sun.GetComponent<BeEngine::TransformComponent>();
    transform.SetRotation({-50.0F, 30.0F, 0.0F});

    auto &light = sun.AddComponent<BeEngine::DirectionalLightComponent>();
    light.Color = glm::vec3(1.0F, 0.98F, 0.95F);
    light.Intensity = 1.2F;
    light.CastShadows = true;
  }

  // ===== Point Light =====
  {
    BeEngine::Entity pointLight = m_ActiveScene->CreateEntity("Point Light");
    auto &transform = pointLight.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({3.0F, 2.0F, 2.0F});

    auto &light = pointLight.AddComponent<BeEngine::PointLightComponent>();
    light.Color = glm::vec3(1.0F, 0.6F, 0.3F);
    light.Intensity = 4.0F;
    light.Range = 15.0F;
  }

  // ===== Ground Plane =====
  {
    BeEngine::Entity ground = m_ActiveScene->CreateEntity("Ground");
    auto &transform = ground.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({0.0F, 0.0F, 0.0F});
    transform.SetScale({20.0F, 1.0F, 20.0F});

    auto &renderer = ground.AddComponent<BeEngine::MeshRendererComponent>();
    renderer.MeshData = BeEngine::MeshFactory::CreatePlane(1.0F, 1.0F, 1);

    // Create and configure material
    auto unlitMat =
        BeEngine::CreateRef<BeEngine::UnlitMaterial>("Ground Material");
    unlitMat->SetColor(glm::vec4(0.18F, 0.18F, 0.2F, 1.0F));
    renderer.MaterialData = unlitMat->GetMaterial();
    m_Materials.push_back(unlitMat); // Keep alive!
  }

  // ===== Red Cube =====
  {
    BeEngine::Entity cube = m_ActiveScene->CreateEntity("Red Cube");
    auto &transform = cube.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({-2.0F, 0.5F, 0.0F});

    auto &renderer = cube.AddComponent<BeEngine::MeshRendererComponent>();
    renderer.MeshData = BeEngine::MeshFactory::CreateCube();

    auto pbrMat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Red PBR");
    pbrMat->SetAlbedoColor(glm::vec4(0.9F, 0.2F, 0.2F, 1.0F));
    pbrMat->SetMetallic(0.1F);
    pbrMat->SetRoughness(0.6F);
    renderer.MaterialData = pbrMat->GetMaterial();
    m_Materials.push_back(pbrMat);
  }

  // ===== Green Sphere =====
  {
    BeEngine::Entity sphere = m_ActiveScene->CreateEntity("Green Sphere");
    auto &transform = sphere.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({0.0F, 0.5F, 0.0F});

    auto &renderer = sphere.AddComponent<BeEngine::MeshRendererComponent>();
    renderer.MeshData = BeEngine::MeshFactory::CreateSphere(0.5F, 32, 16);

    auto pbrMat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Green PBR");
    pbrMat->SetAlbedoColor(glm::vec4(0.2F, 0.9F, 0.3F, 1.0F));
    pbrMat->SetMetallic(0.8F);
    pbrMat->SetRoughness(0.2F);
    renderer.MaterialData = pbrMat->GetMaterial();
    m_Materials.push_back(pbrMat);
  }

  // ===== Blue Cube =====
  {
    BeEngine::Entity cube = m_ActiveScene->CreateEntity("Blue Cube");
    auto &transform = cube.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({2.0F, 0.5F, 0.0F});

    auto &renderer = cube.AddComponent<BeEngine::MeshRendererComponent>();
    renderer.MeshData = BeEngine::MeshFactory::CreateCube();

    auto pbrMat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Blue PBR");
    pbrMat->SetAlbedoColor(glm::vec4(0.2F, 0.3F, 0.9F, 1.0F));
    pbrMat->SetMetallic(0.5F);
    pbrMat->SetRoughness(0.4F);
    renderer.MaterialData = pbrMat->GetMaterial();
    m_Materials.push_back(pbrMat);
  }

  // ===== Damaged Helmet (glTF Model) =====
  {
    BeEngine::Entity helmet = m_ActiveScene->CreateEntity("Damaged Helmet");
    auto &transform = helmet.GetComponent<BeEngine::TransformComponent>();
    transform.SetPosition({0.0F, 1.5F, -3.0F});
    transform.SetScale({0.8F, 0.8F, 0.8F});

    auto &renderer = helmet.AddComponent<BeEngine::ModelRendererComponent>();
    renderer.ModelData =
        BeEngine::Model::Load("BeEngine/Assets/DamagedHelmet.glb");

    if (!renderer.ModelData || !renderer.ModelData->IsValid()) {
      BE_WARN("Failed to load DamagedHelmet.glb");
    }
  }

  // ===== Entity Hierarchy Example =====
  {
    // Parent
    BeEngine::Entity parent = m_ActiveScene->CreateEntity("Rotating Platform");
    auto &parentTransform = parent.GetComponent<BeEngine::TransformComponent>();
    parentTransform.SetPosition({-5.0F, 0.5F, -3.0F});

    auto &parentRenderer =
        parent.AddComponent<BeEngine::MeshRendererComponent>();
    parentRenderer.MeshData = BeEngine::MeshFactory::CreateCube();

    auto platformMat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Platform");
    platformMat->SetAlbedoColor(glm::vec4(0.5F, 0.5F, 0.5F, 1.0F));
    platformMat->SetMetallic(0.3F);
    platformMat->SetRoughness(0.7F);
    parentRenderer.MaterialData = platformMat->GetMaterial();
    m_Materials.push_back(platformMat);

    // Child 1 - Left sphere
    BeEngine::Entity child1 = m_ActiveScene->CreateEntity("Orbiting Sphere 1");
    auto &child1Transform = child1.GetComponent<BeEngine::TransformComponent>();
    child1Transform.SetPosition({-1.5F, 1.0F, 0.0F});
    child1Transform.SetScale({0.3F, 0.3F, 0.3F});
    m_ActiveScene->SetParent(child1, parent);

    auto &child1Renderer =
        child1.AddComponent<BeEngine::MeshRendererComponent>();
    child1Renderer.MeshData = BeEngine::MeshFactory::CreateSphere(1.0F, 16, 8);

    auto orbit1Mat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Orbit1");
    orbit1Mat->SetAlbedoColor(glm::vec4(1.0F, 0.8F, 0.2F, 1.0F));
    orbit1Mat->SetMetallic(0.9F);
    orbit1Mat->SetRoughness(0.1F);
    child1Renderer.MaterialData = orbit1Mat->GetMaterial();
    m_Materials.push_back(orbit1Mat);

    // Child 2 - Right sphere
    BeEngine::Entity child2 = m_ActiveScene->CreateEntity("Orbiting Sphere 2");
    auto &child2Transform = child2.GetComponent<BeEngine::TransformComponent>();
    child2Transform.SetPosition({1.5F, 1.0F, 0.0F});
    child2Transform.SetScale({0.3F, 0.3F, 0.3F});
    m_ActiveScene->SetParent(child2, parent);

    auto &child2Renderer =
        child2.AddComponent<BeEngine::MeshRendererComponent>();
    child2Renderer.MeshData = BeEngine::MeshFactory::CreateSphere(1.0F, 16, 8);

    auto orbit2Mat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Orbit2");
    orbit2Mat->SetAlbedoColor(glm::vec4(0.2F, 0.8F, 1.0F, 1.0F));
    orbit2Mat->SetMetallic(0.9F);
    orbit2Mat->SetRoughness(0.1F);
    child2Renderer.MaterialData = orbit2Mat->GetMaterial();
    m_Materials.push_back(orbit2Mat);
  }
}

// ============================================================================
// Update
// ============================================================================

void SandboxLayer3D::OnUpdate(BeEngine::Timestep ts) {
  // Update editor camera
  if (m_ViewportFocused && m_UseEditorCamera) {
    m_EditorCamera->OnUpdate(ts);
  }

  // Update scene (transforms, scripts, etc.)
  if (m_ActiveScene) {
    m_ActiveScene->OnUpdate(ts.GetSeconds());

    // Example: Rotate the platform entity
    BeEngine::Entity platform =
        m_ActiveScene->FindEntityByName("Rotating Platform");
    if (platform.IsValid()) {
      auto &transform = platform.GetComponent<BeEngine::TransformComponent>();
      glm::vec3 rotation = glm::eulerAngles(transform.GetRotation());
      rotation.y += glm::radians(30.0F) * ts.GetSeconds();
      transform.SetRotation(glm::quat(rotation));
    }
  }

  // Handle viewport resize
  const auto &spec = m_Framebuffer->GetSpecification();
  if (m_ViewportSize.x > 0.0F && m_ViewportSize.y > 0.0F &&
      (static_cast<uint32_t>(m_ViewportSize.x) != spec.Width ||
       static_cast<uint32_t>(m_ViewportSize.y) != spec.Height)) {
    m_Framebuffer->Resize(static_cast<uint32_t>(m_ViewportSize.x),
                          static_cast<uint32_t>(m_ViewportSize.y));
    m_EditorCamera->OnViewportResize(m_ViewportSize.x, m_ViewportSize.y);
    m_SceneRenderer.SetViewport(static_cast<uint32_t>(m_ViewportSize.x),
                                static_cast<uint32_t>(m_ViewportSize.y));
  }
}

void SandboxLayer3D::OnEvent(BeEngine::Event &event) {
  if (m_UseEditorCamera) {
    m_EditorCamera->OnEvent(event);
  }
}

// ============================================================================
// Rendering
// ============================================================================

void SandboxLayer3D::OnRender() {
  if (!m_Framebuffer || !m_ActiveScene) {
    return;
  }

  m_Framebuffer->Bind();

  glClearColor(0.08F, 0.08F, 0.1F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  // Get camera matrices
  glm::mat4 viewProj;
  glm::vec3 camPos;

  if (m_UseEditorCamera) {
    viewProj = m_EditorCamera->GetCamera().GetViewProjectionMatrix();
    camPos = m_EditorCamera->GetPosition();
  } else {
    BeEngine::Entity primaryCam = m_ActiveScene->GetPrimaryCamera();
    if (primaryCam.IsValid()) {
      auto &cam = primaryCam.GetComponent<BeEngine::CameraComponent>();
      viewProj = cam.ViewProjectionMatrix;
      auto &transform = primaryCam.GetComponent<BeEngine::TransformComponent>();
      camPos = glm::vec3(transform.WorldMatrix[3]);
    } else {
      m_Framebuffer->Unbind();
      return;
    }
  }

  // Draw editor grid
  if (m_GridShader && m_GridVAO) {
    glDisable(GL_CULL_FACE);
    m_GridShader->Bind();
    m_GridShader->SetMat4("u_ViewProjection", viewProj);
    m_GridShader->SetMat4("u_Model", glm::mat4(1.0F));
    m_GridVAO->Bind();
    glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(m_GridVertexCount));
  }

  // Render scene
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  m_SceneRenderer.RenderScene(*m_ActiveScene, viewProj, camPos);

  m_Framebuffer->Unbind();
}

// ============================================================================
// ImGui
// ============================================================================

void SandboxLayer3D::OnImGuiRender() {
  DrawViewportPanel();
  DrawSceneHierarchyPanel();
  DrawInspectorPanel();
  DrawSceneSettingsPanel();
}

void SandboxLayer3D::DrawViewportPanel() {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("3D Viewport");

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

void SandboxLayer3D::DrawSceneHierarchyPanel() {
  ImGui::Begin("Scene Hierarchy");

  if (m_ActiveScene) {
    ImGui::Text("Scene: %s", m_ActiveScene->GetName().c_str());
    ImGui::Text("Entities: %zu", m_ActiveScene->GetEntityCount());
    ImGui::Separator();

    // Draw root entities
    for (BeEngine::Entity entity : m_ActiveScene->GetRootEntities()) {
      DrawEntityNode(entity);
    }

    // Right-click context menu for creating entities
    if (ImGui::BeginPopupContextWindow(nullptr,
                                       ImGuiPopupFlags_NoOpenOverItems |
                                           ImGuiPopupFlags_MouseButtonRight)) {
      if (ImGui::MenuItem("Create Empty Entity")) {
        m_ActiveScene->CreateEntity("New Entity");
      }
      if (ImGui::MenuItem("Create Cube")) {
        auto entity = m_ActiveScene->CreateEntity("Cube");
        auto &renderer = entity.AddComponent<BeEngine::MeshRendererComponent>();
        renderer.MeshData = BeEngine::MeshFactory::CreateCube();

        auto pbrMat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Default");
        renderer.MaterialData = pbrMat->GetMaterial();
        m_Materials.push_back(pbrMat);
      }
      if (ImGui::MenuItem("Create Sphere")) {
        auto entity = m_ActiveScene->CreateEntity("Sphere");
        auto &renderer = entity.AddComponent<BeEngine::MeshRendererComponent>();
        renderer.MeshData = BeEngine::MeshFactory::CreateSphere(0.5F, 32, 16);

        auto pbrMat = BeEngine::CreateRef<BeEngine::PBRMaterial>("Default");
        renderer.MaterialData = pbrMat->GetMaterial();
        m_Materials.push_back(pbrMat);
      }
      ImGui::EndPopup();
    }
  }

  ImGui::End();
}

void SandboxLayer3D::DrawEntityNode(BeEngine::Entity entity) {
  auto &tag = entity.GetComponent<BeEngine::TagComponent>();
  auto children = m_ActiveScene->GetChildren(entity);

  ImGuiTreeNodeFlags flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
  if (children.empty()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }
  if (m_SelectedEntity == entity) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool opened = ImGui::TreeNodeEx(
      reinterpret_cast<void *>(static_cast<uint64_t>(entity.GetUUID())), flags,
      "%s", tag.Tag.c_str());

  if (ImGui::IsItemClicked()) {
    m_SelectedEntity = entity;
  }

  // Context menu
  if (ImGui::BeginPopupContextItem()) {
    if (ImGui::MenuItem("Delete Entity")) {
      if (m_SelectedEntity == entity) {
        m_SelectedEntity = BeEngine::Entity();
      }
      m_ActiveScene->DestroyEntity(entity);
    }
    ImGui::EndPopup();
  }

  if (opened) {
    for (BeEngine::Entity child : children) {
      DrawEntityNode(child);
    }
    ImGui::TreePop();
  }
}

void SandboxLayer3D::DrawInspectorPanel() {
  ImGui::Begin("Inspector");

  if (m_SelectedEntity.IsValid()) {
    auto &tag = m_SelectedEntity.GetComponent<BeEngine::TagComponent>();

    // Tag/Name
    char buffer[256];
    strncpy(buffer, tag.Tag.c_str(), sizeof(buffer));
    if (ImGui::InputText("Name", buffer, sizeof(buffer))) {
      tag.Tag = std::string(buffer);
    }

    ImGui::Separator();

    // Transform
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
      auto &transform =
          m_SelectedEntity.GetComponent<BeEngine::TransformComponent>();

      glm::vec3 position = transform.GetPosition();
      if (ImGui::DragFloat3("Position", &position.x, 0.1F)) {
        transform.SetPosition(position);
      }

      glm::vec3 rotation =
          glm::degrees(glm::eulerAngles(transform.GetRotation()));
      if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0F)) {
        transform.SetRotation(glm::radians(rotation));
      }

      glm::vec3 scale = transform.GetScale();
      if (ImGui::DragFloat3("Scale", &scale.x, 0.1F)) {
        transform.SetScale(scale);
      }
    }

    // Camera Component
    if (m_SelectedEntity.HasComponent<BeEngine::CameraComponent>()) {
      if (ImGui::CollapsingHeader("Camera")) {
        auto &camera =
            m_SelectedEntity.GetComponent<BeEngine::CameraComponent>();
        ImGui::Checkbox("Primary", &camera.Primary);
        ImGui::SliderFloat("FOV", &camera.FOV, 1.0F, 120.0F);
        ImGui::DragFloat("Near Clip", &camera.NearClip, 0.01F, 0.001F, 10.0F);
        ImGui::DragFloat("Far Clip", &camera.FarClip, 1.0F, 10.0F, 10000.0F);
      }
    }

    // Mesh Renderer Component
    if (m_SelectedEntity.HasComponent<BeEngine::MeshRendererComponent>()) {
      if (ImGui::CollapsingHeader("Mesh Renderer")) {
        auto &renderer =
            m_SelectedEntity.GetComponent<BeEngine::MeshRendererComponent>();
        ImGui::Checkbox("Visible", &renderer.Visible);
        ImGui::Checkbox("Cast Shadows", &renderer.CastShadows);
        ImGui::Checkbox("Receive Shadows", &renderer.ReceiveShadows);
      }
    }

    // Point Light Component
    if (m_SelectedEntity.HasComponent<BeEngine::PointLightComponent>()) {
      if (ImGui::CollapsingHeader("Point Light")) {
        auto &light =
            m_SelectedEntity.GetComponent<BeEngine::PointLightComponent>();
        ImGui::ColorEdit3("Color", &light.Color.x);
        ImGui::SliderFloat("Intensity", &light.Intensity, 0.0F, 20.0F);
        ImGui::SliderFloat("Range", &light.Range, 0.1F, 100.0F);
      }
    }

    // Directional Light Component
    if (m_SelectedEntity.HasComponent<BeEngine::DirectionalLightComponent>()) {
      if (ImGui::CollapsingHeader("Directional Light")) {
        auto &light = m_SelectedEntity
                          .GetComponent<BeEngine::DirectionalLightComponent>();
        ImGui::ColorEdit3("Color", &light.Color.x);
        ImGui::SliderFloat("Intensity", &light.Intensity, 0.0F, 10.0F);
        ImGui::Checkbox("Cast Shadows", &light.CastShadows);
      }
    }

  } else {
    ImGui::TextDisabled("Select an entity to inspect");
  }

  ImGui::End();
}

void SandboxLayer3D::DrawSceneSettingsPanel() {
  ImGui::Begin("Scene Settings");

  // Camera mode
  ImGui::Checkbox("Use Editor Camera", &m_UseEditorCamera);

  if (m_UseEditorCamera) {
    auto camPos = m_EditorCamera->GetPosition();
    ImGui::Text("Camera: (%.1f, %.1f, %.1f)", camPos.x, camPos.y, camPos.z);
  }

  ImGui::Separator();

  // Lighting
  auto &lightManager = m_SceneRenderer.GetLightManager();
  auto &ambient = lightManager.GetAmbientLight();
  ImGui::ColorEdit3("Ambient Light", &ambient.Color.x);

  ImGui::Separator();

  // Save/Load
  if (ImGui::Button("Save Scene")) {
    m_SceneManager.SaveScene("Assets/Scenes/DemoScene.scene");
  }
  ImGui::SameLine();
  if (ImGui::Button("Load Scene")) {
    m_SceneManager.LoadScene("Assets/Scenes/DemoScene.scene");
    m_ActiveScene = m_SceneManager.GetActiveScene();
    m_SelectedEntity = BeEngine::Entity();
  }

  ImGui::End();
}

// ============================================================================
// Grid Setup
// ============================================================================

void SandboxLayer3D::SetupGrid() {
  std::vector<float> gridVertices;
  const int gridSize = 20;
  const float gridStep = 1.0F;

  for (int i = -gridSize; i <= gridSize; ++i) {
    float pos = static_cast<float>(i) * gridStep;
    float alpha = (i == 0) ? 0.6F : 0.25F;

    // Z-parallel lines
    gridVertices.insert(gridVertices.end(),
                        {pos, 0.0F, static_cast<float>(-gridSize) * gridStep,
                         alpha, alpha, alpha, pos, 0.0F,
                         static_cast<float>(gridSize) * gridStep, alpha, alpha,
                         alpha});

    // X-parallel lines
    gridVertices.insert(gridVertices.end(),
                        {static_cast<float>(-gridSize) * gridStep, 0.0F, pos,
                         alpha, alpha, alpha,
                         static_cast<float>(gridSize) * gridStep, 0.0F, pos,
                         alpha, alpha, alpha});
  }

  m_GridVertexCount = static_cast<uint32_t>(gridVertices.size() / 6);

  m_GridVAO = BeEngine::VertexArray::Create();
  auto vb = BeEngine::VertexBuffer::Create(
      gridVertices.data(),
      static_cast<uint32_t>(gridVertices.size() * sizeof(float)));

  vb->SetLayout({{BeEngine::ShaderDataType::Float3, "a_Position"},
                 {BeEngine::ShaderDataType::Float3, "a_Color"}});
  m_GridVAO->AddVertexBuffer(vb);

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
}
