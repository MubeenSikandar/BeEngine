// BeEngine/include/Renderer/Renderer.hpp
#pragma once

#include <Renderer/RendererAPI.hpp>
#include <glm/glm.hpp>
#include <memory>

namespace BeEngine {

/**
 * @brief High-level rendering interface
 *
 * This class provides a simple, high-level API for rendering.
 * It wraps the low-level RendererAPI and adds convenience functions.
 */
class BE_API Renderer {
public:
  // ===== Lifecycle =====

  /**
   * @brief Initialize the renderer
   *
   * Must be called after window creation and OpenGL context setup.
   * Call RendererAPI::SetAPI() before this.
   */
  static void Init();

  /**
   * @brief Shutdown the renderer
   */
  static void Shutdown();

  // ===== Frame Management =====

  /**
   * @brief Begin a new frame
   */
  static void BeginFrame();

  /**
   * @brief End the current frame
   */
  static void EndFrame();

  // ===== Clear Operations =====

  /**
   * @brief Clear the screen
   */
  static void Clear(float r = 0.0f, float g = 0.0f, float b = 0.0f,
                    float a = 1.0f);

  /**
   * @brief Set clear color
   */
  static void SetClearColor(const glm::vec4 &color);

  // ===== Viewport =====

  /**
   * @brief Set rendering viewport
   */
  static void SetViewport(uint32_t x, uint32_t y, uint32_t width,
                          uint32_t height);

  /**
   * @brief Handle window resize
   */
  static void OnWindowResize(uint32_t width, uint32_t height);

  // ===== Drawing =====

  /**
   * @brief Draw indexed geometry
   */
  static void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0);

  /**
   * @brief Draw non-indexed geometry
   */
  static void Draw(uint32_t vertexCount, uint32_t startVertex = 0);

  // ===== State =====

  static void EnableDepthTest(bool enable);
  static void EnableBlending(bool enable);
  static void EnableCulling(bool enable);
  static void SetWireframeMode(bool enable);

  // ===== Getters =====

  /**
   * @brief Get the underlying RendererAPI
   */
  static RendererAPI *GetAPI() { return s_RendererAPI.get(); }

private:
  static std::unique_ptr<RendererAPI> s_RendererAPI;
};

} // namespace BeEngine
