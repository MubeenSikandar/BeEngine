// SandboxLayer.hpp
#pragma once
#include "PCH/BeEnginePCH.hpp"

class SandboxLayer : public BeEngine::Layer {
public:
  SandboxLayer() : Layer("SandboxLayer") {}

  void OnAttach() override {
    BE_INFO("SandboxLayer attached - setting up triangle");
    SetupTriangle();
  }

  void OnDetach() override { BE_INFO("SandboxLayer detached"); }

  void OnRender() override {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if (!m_Shader || !m_VertexArray) {
      BE_CORE_ERROR("SandboxLayer: Shader or VertexArray is null!");
      return;
    }

    m_Shader->Bind();
    m_VertexArray->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 3);
  }

  void OnImGuiRender() override {
    ImGui::Begin("Triangle Demo");
    ImGui::Text("Hello from Sandbox!");
    ImGui::Text("Triangle Status: %s",
                (m_VertexArray && m_Shader) ? "Ready" : "Not Ready");
    ImGui::End();
  }

private:
  void SetupTriangle();
  std::shared_ptr<BeEngine::VertexArray> m_VertexArray;
  std::shared_ptr<BeEngine::Shader> m_Shader;
  // std::shared_ptr<BeEngine::Framebuffer> m_Framebuffer;
  // glm::vec2 m_ViewportSize = {1280, 720};
};
