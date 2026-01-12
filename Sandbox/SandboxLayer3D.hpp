// SandboxLayer3D.hpp
#pragma once
#include <Application.hpp>
#include <PCH/BeEnginePCH.hpp>

class SandboxLayer3D : public BeEngine::Layer {
public:
  SandboxLayer3D() : Layer("SandboxLayer3D") {}
  void OnAttach() override;
  void OnDetach() override;
  void OnUpdate(BeEngine::Timestep ts) override;
  void OnEvent(BeEngine::Event &event) override;
  void OnRender() override;
  void OnImGuiRender() override;

private:
  // Scene setup
  void SetupScene();
  void SetupGrid();

  // ImGui panels
  void DrawViewportPanel();
  void DrawSceneHierarchyPanel();
  void DrawInspectorPanel();
  void DrawSceneSettingsPanel();

  // Recursive hierarchy drawing
  void DrawEntityNode(BeEngine::Entity entity);

private:
  // ===== Scene System =====
  BeEngine::SceneManager m_SceneManager;
  BeEngine::SceneRenderer m_SceneRenderer;
  BeEngine::Scene *m_ActiveScene = nullptr;

  // ===== Materials (keep specific material types alive) =====
  std::vector<std::shared_ptr<void>> m_Materials; // Type-erased storage

  // ===== Editor State =====
  BeEngine::Scope<BeEngine::PerspectiveCameraController> m_EditorCamera;
  BeEngine::Entity m_SelectedEntity;
  bool m_UseEditorCamera = true;

  // ===== Rendering =====
  BeEngine::Ref<BeEngine::Framebuffer> m_Framebuffer;
  BeEngine::Ref<BeEngine::VertexArray> m_GridVAO;
  BeEngine::Ref<BeEngine::Shader> m_GridShader;
  uint32_t m_GridVertexCount = 0;

  // ===== Viewport State =====
  glm::vec2 m_ViewportSize = {1280.0F, 720.0F};
  bool m_ViewportFocused = false;
  bool m_ViewportHovered = false;
};
