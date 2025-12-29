//OpenGLRendererAPI.hpp
#pragma once

#include "Renderer/RendererAPI.hpp"
#include <cstdint>
#include <glm/glm.hpp>
namespace BeEngine {
/**
 * @brief OpenGL implementation of RendererAPI
 *
 * Modern OpenGL (4.5+) renderer using:
 * - Core profile (no legacy functions)
 * - Vertex Array Objects (VAOs)
 * - Vertex Buffer Objects (VBOs)
 * - Element Buffer Objects (EBOs)
 * - GLSL Shaders
 */
class OpenGLRendererAPI : public RendererAPI {
public:
  OpenGLRendererAPI() = default;
  ~OpenGLRendererAPI() override = default;

  void Init() override;
  void Shutdown() override;

  void BeginFrame() override;
  void EndFrame() override;

  void Clear(float r, float g, float b, float a) override;
  void SetClearColor(const glm::vec4 &color) override;

  void SetViewport(uint32_t x, uint32_t y, uint32_t width,
                   uint32_t height) override;

  void DrawIndexed(uint32_t indexCount, uint32_t startIndex) override;
  void Draw(uint32_t vertexCount, uint32_t startVertex) override;

  void EnableDepthTest(bool enable) override;
  void EnableBlending(bool enable) override;
  void EnableCulling(bool enable) override;
  void SetWireframeMode(bool enable) override;

private:
  const float CLEARCOLOR_RGB{0.1F};
  const float CLEARCOLOR_A{0.1F};
  glm::vec4 m_ClearColor{CLEARCOLOR_RGB, CLEARCOLOR_RGB, CLEARCOLOR_RGB,
                         CLEARCOLOR_A};
};

} // namespace BeEngine
