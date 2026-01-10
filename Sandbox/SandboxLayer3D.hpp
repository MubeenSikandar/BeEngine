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
  void SetupGrid();

  BeEngine::Scope<BeEngine::PerspectiveCameraController> m_CameraController;

  // Meshes (using Ref<Mesh>, NOT MeshFactory)
  BeEngine::Ref<BeEngine::Mesh> m_CubeMesh;
  BeEngine::Ref<BeEngine::Mesh> m_SphereMesh;
  BeEngine::Ref<BeEngine::Mesh> m_PlaneMesh;

  // Grid (floor) - still using raw VAO since it's simple lines
  BeEngine::Ref<BeEngine::VertexArray> m_GridVAO;
  BeEngine::Ref<BeEngine::Shader> m_GridShader;
  uint32_t m_GridVertexCount = 0;

  // Framebuffer
  BeEngine::Ref<BeEngine::Framebuffer> m_Framebuffer;
  glm::vec2 m_ViewportSize = {1280, 720};
  bool m_ViewportFocused = false;
  bool m_ViewportHovered = false;

  // Transform
  bool m_AutoRotate = true;
  glm::vec3 m_CubeEulerAngles = {0.0F, 0.0F, 0.0F};
  BeEngine::Transform m_CubeTransform;
  BeEngine::Transform m_SphereTransform;

  // Textures
  BeEngine::Ref<BeEngine::Texture2D> m_CheckerTexture;
  BeEngine::Ref<BeEngine::Texture2D> m_CubeTexture;

  // Lighting
  BeEngine::LightManager m_LightManager;

  // Materials (using built-in material types)
  BeEngine::Scope<BeEngine::UnlitMaterial> m_UnlitMat;
  BeEngine::Scope<BeEngine::PhongMaterial> m_PhongMat;
  BeEngine::Scope<BeEngine::PBRMaterial> m_PBRMat;
};
