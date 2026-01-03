// SandboxLayer.hpp
#pragma once
#include "Application.hpp"
#include "PCH/BeEnginePCH.hpp"

class SandboxLayer : public BeEngine::Layer {
public:
  SandboxLayer() : Layer("SandboxLayer") {}

  void OnAttach() override {
    BE_INFO("SandboxLayer attached - setting up rendering");

    m_CameraController =
        std::make_unique<BeEngine::OrthographicCameraController>(
            1280.0F / 720.0F, // Aspect ratio
            1.0F,             // Size
            true              // Enable rotation
        );

    BeEngine::FramebufferSpecification fbSpec;
    fbSpec.Width = 1280;
    fbSpec.Height = 720;
    fbSpec.Attachments = {
        BeEngine::FramebufferTextureFormat::RGBA8, // Color only
        BeEngine::FramebufferTextureFormat::Depth  // Depth
    };
    m_Framebuffer = BeEngine::Framebuffer::Create(fbSpec);

    SetupTriangle();
  }

  void OnDetach() override { BE_INFO("SandboxLayer detached"); }

  void OnUpdate(BeEngine::Timestep ts) override {
    if (m_ViewportFocused) {
      m_CameraController->OnUpdate(ts);
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

  void OnEvent(BeEngine::Event &event) override {
    m_CameraController->OnEvent(event);
  }

  void OnRender() override {
    if (!m_Shader || !m_VertexArray || !m_Framebuffer) {
      BE_CORE_ERROR("SandboxLayer: Resources not ready!");
      return;
    }

    m_Framebuffer->Bind();

    glClearColor(0.1F, 0.1F, 0.1F, 1.0F); // Back to dark gray
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    m_Shader->Bind();

    m_Shader->SetMat4(
        "u_ViewProjection",
        m_CameraController->GetCamera().GetViewProjectionMatrix());

    m_VertexArray->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    m_Framebuffer->Unbind();
  }

  void OnImGuiRender() override {
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
    auto &camera = m_CameraController->GetCamera();
    auto pos = camera.GetPosition();
    ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
    ImGui::Text("Zoom: %.2f", camera.GetZoom());
    ImGui::Text("Rotation: %.1f°", glm::degrees(camera.GetRotation()));
    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::Text("  WASD - Move");
    ImGui::Text("  Q/E  - Rotate");
    ImGui::Text("  Scroll - Zoom");
    ImGui::End();
  }

private:
  void SetupTriangle();
  std::unique_ptr<BeEngine::OrthographicCameraController> m_CameraController;
  std::shared_ptr<BeEngine::VertexArray> m_VertexArray;
  std::shared_ptr<BeEngine::Shader> m_Shader;
  std::shared_ptr<BeEngine::Framebuffer> m_Framebuffer;
  glm::vec2 m_ViewportSize = {1280, 720};
  bool m_ViewportFocused = false;
  bool m_ViewportHovered = false;
};
