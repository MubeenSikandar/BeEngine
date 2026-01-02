// SandboxLayer.hpp
#pragma once
#include "PCH/BeEnginePCH.hpp"

class SandboxLayer : public BeEngine::Layer {
public:
  SandboxLayer() : Layer("SandboxLayer") {}

  void OnAttach() override {
    BE_INFO("SandboxLayer attached - setting up rendering");

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
    // Handle viewport resize
    const auto &spec = m_Framebuffer->GetSpecification();
    if (m_ViewportSize.x > 0.0F && m_ViewportSize.y > 0.0F &&
        (static_cast<uint32_t>(m_ViewportSize.x) != spec.Width ||
         static_cast<uint32_t>(m_ViewportSize.y) != spec.Height)) {
      m_Framebuffer->Resize(static_cast<uint32_t>(m_ViewportSize.x),
                            static_cast<uint32_t>(m_ViewportSize.y));
    }
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
    m_VertexArray->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    m_Framebuffer->Unbind();
  }

  void OnImGuiRender() override {
    // ===== VIEWPORT WINDOW - DISPLAYS THE FRAMEBUFFER =====
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Viewport");

    // Track focus/hover state
    m_ViewportFocused = ImGui::IsWindowFocused();
    m_ViewportHovered = ImGui::IsWindowHovered();

    // Get available size for the viewport
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    m_ViewportSize = {viewportSize.x, viewportSize.y};

    // Display the framebuffer texture
    uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID(0);
    ImGui::Image(reinterpret_cast<void *>(textureID),
                 ImVec2(m_ViewportSize.x, m_ViewportSize.y),
                 ImVec2(0, 1),  // UV top-left (flipped for OpenGL)
                 ImVec2(1, 0)); // UV bottom-right (flipped for OpenGL)

    ImGui::End();
    ImGui::PopStyleVar();

    // ===== DEBUG INFO WINDOW =====
    // Debug window with time info
    ImGui::Begin("Triangle Demo");
    ImGui::Text("Triangle Status: %s",
                (m_VertexArray && m_Shader) ? "Ready" : "Not Ready");
    ImGui::Separator();
    ImGui::Text("Viewport: %.0f x %.0f", m_ViewportSize.x, m_ViewportSize.y);
    ImGui::Text("Focused: %s", m_ViewportFocused ? "Yes" : "No");

    // Time info
    ImGui::Separator();
    ImGui::Text("FPS: %.1f", BeEngine::Time::GetFPS());
    ImGui::Text("Delta Time: %.3f ms",
                BeEngine::Time::GetDeltaTime() * 1000.0f);
    ImGui::Text("Time: %.2f s", BeEngine::Time::GetTime());
    ImGui::Text("Frame: %llu", static_cast<long long unsigned int>(
                                   BeEngine::Time::GetFrameCount()));

    // Time controls
    ImGui::Separator();
    float timeScale = BeEngine::Time::GetTimeScale();
    if (ImGui::SliderFloat("Time Scale", &timeScale, 0.0f, 2.0f)) {
      BeEngine::Time::SetTimeScale(timeScale);
    }

    bool paused = BeEngine::Time::IsPaused();
    if (ImGui::Checkbox("Paused", &paused)) {
      BeEngine::Time::SetPaused(paused);
    }

    ImGui::End();
  }

private:
  void SetupTriangle();
  std::shared_ptr<BeEngine::VertexArray> m_VertexArray;
  std::shared_ptr<BeEngine::Shader> m_Shader;
  std::shared_ptr<BeEngine::Framebuffer> m_Framebuffer;
  glm::vec2 m_ViewportSize = {1280, 720};
  bool m_ViewportFocused = false;
  bool m_ViewportHovered = false;
};
