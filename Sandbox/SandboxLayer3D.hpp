// SandboxLayer3D.hpp
#pragma once

#include "Application.hpp"
#include "PCH/BeEnginePCH.hpp"

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
  void SetupCube();
  void SetupGrid();

  std::unique_ptr<BeEngine::PerspectiveCameraController> m_CameraController;

  // Cube
  std::shared_ptr<BeEngine::VertexArray> m_CubeVAO;
  std::shared_ptr<BeEngine::Shader> m_CubeShader;

  // Grid (floor)
  std::shared_ptr<BeEngine::VertexArray> m_GridVAO;
  std::shared_ptr<BeEngine::Shader> m_GridShader;
  uint32_t m_GridVertexCount = 0;

  std::shared_ptr<BeEngine::Framebuffer> m_Framebuffer;

  glm::vec2 m_ViewportSize = {1280, 720};
  bool m_ViewportFocused = false;
  bool m_ViewportHovered = false;

  // Cube transform
  // glm::vec3 m_CubePosition = {0.0f, 0.5f, 0.0f};
  // glm::vec3 m_CubeRotation = {0.0f, 0.0f, 0.0f};
  // float m_CubeScale = 1.0f;
  bool m_AutoRotate = true;
  glm::vec3 m_CubeEulerAngles = {0.0F, 0.0F, 0.0F};

  BeEngine::Transform m_CubeTransform;
};
