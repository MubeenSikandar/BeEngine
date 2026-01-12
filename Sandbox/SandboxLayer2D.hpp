// SandboxLayer2D.hpp
#pragma once

#include <Application.hpp>
#include <PCH/BeEnginePCH.hpp>

/**
 * @brief 2D Sandbox Layer demonstrating 2D rendering with Scene System
 *
 * Features:
 * - 2D orthographic camera
 * - Sprite rendering (future)
 * - 2D physics components (future)
 * - Tilemaps (future)
 */
class SandboxLayer2D : public BeEngine::Layer {
public:
  SandboxLayer2D() : Layer("SandboxLayer2D") {}

  void OnAttach() override;
  void OnDetach() override;
  void OnUpdate(BeEngine::Timestep ts) override;
  void OnEvent(BeEngine::Event &event) override;
  void OnRender() override;
  void OnImGuiRender() override;

private:
  void SetupScene();
  void DrawViewportPanel();
  void DrawSceneHierarchyPanel();

  // ===== Scene System =====
  BeEngine::SceneManager m_SceneManager;
  BeEngine::Scene *m_ActiveScene = nullptr;

  // ===== 2D Camera =====
  BeEngine::Scope<BeEngine::OrthographicCameraController> m_CameraController;

  // ===== Rendering =====
  BeEngine::Ref<BeEngine::Framebuffer> m_Framebuffer;
  BeEngine::Ref<BeEngine::VertexArray> m_QuadVAO;
  BeEngine::Ref<BeEngine::Shader> m_SpriteShader;

  // ===== Viewport State =====
  glm::vec2 m_ViewportSize = {1280.0F, 720.0F};
  bool m_ViewportFocused = false;
  bool m_ViewportHovered = false;
};
