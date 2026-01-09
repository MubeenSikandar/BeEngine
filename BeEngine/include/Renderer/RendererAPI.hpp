#pragma once

#include "Core.hpp"
#include "glm/fwd.hpp"
#include <cstdint>
#include <memory>
namespace BeEngine {

/**
 * @brief Supported rendering APIs
 */
enum class RenderAPI : uint8_t {
  None = 0,
  OpenGL = 1,
  Vulkan = 2,
  DirectX11 = 3,
  DirectX12 = 4,
  Metal = 5
};

/**
 * @brief Abstract rendering API interface
 *
 * This interface abstracts away the underlying graphics API (OpenGL, Vulkan,
 * etc.) allowing you to swap implementations without changing higher-level
 * code.
 *
 * Design Pattern: Strategy Pattern
 * - RendererAPI = Strategy Interface
 * - OpenGLRendererAPI = Concrete Strategy (OpenGL)
 * - VulkanRendererAPI = Concrete Strategy (Vulkan)
 */

class BE_API RendererAPI {
public:
  virtual ~RendererAPI();

  // ===== Initialization =====

  /**
   * @brief Initialize the rendering API
   *
   * Called once during application startup.
   * Sets up graphics context, debug layers, etc.
   */

  virtual void Init() = 0;

  /**
   * @brief Shutdown the rendering API
   *
   * Called once during application shutdown.
   * Cleans up all graphics resources.
   */
  virtual void Shutdown() = 0;

  // ===== Frame Management =====

  /**
   * @brief Begin a new frame
   *
   * Called at the start of each frame.
   * Prepares the renderer for drawing.
   */
  virtual void BeginFrame() = 0;

  /**
   * @brief End the current frame
   *
   * Called at the end of each frame.
   * Presents the final image to the screen.
   */
  virtual void EndFrame() = 0;

  // ===== Clear Operations =====

  /**
   * @brief Clear the screen with a color
   * @param r Red component (0.0 - 1.0)
   * @param g Green component (0.0 - 1.0)
   * @param b Blue component (0.0 - 1.0)
   * @param a Alpha component (0.0 - 1.0)
   */
  virtual void Clear(float r = 0.0F, float g = 0.0F, float b = 0.0F,
                     float a = 0.0F) = 0;

  /**
   * @brief Set the clear color
   */
  virtual void SetClearColor(const glm::vec4 &color) = 0;

  // ===== Viewport =====

  /**
   * @brief Set the rendering viewport
   * @param x X position (bottom-left corner)
   * @param y Y position (bottom-left corner)
   * @param width Viewport width
   * @param height Viewport height
   */
  virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width,
                           uint32_t height) = 0;

  // ===== Drawing =====

  /**
   * @brief Draw indexed geometry
   * @param indexCount Number of indices to draw
   *
   * Draws using the currently bound vertex array and shader.
   */
  virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndex = 0) = 0;

  /**
   * @brief Draw non-indexed geometry
   * @param vertexCount Number of vertices to draw
   */
  virtual void Draw(uint32_t vertexCount, uint32_t startVertex = 0) = 0;

  // ===== State Management =====

  /**
   * @brief Enable depth testing
   */
  virtual void EnableDepthTest(bool enabled) = 0;

  /**
   * @brief Enable blending (transparency)
   */
  virtual void EnableBlending(bool enabled) = 0;

  /**
   * @brief Enable face culling
   */
  virtual void EnableCulling(bool enabled) = 0;

  /**
   * @brief Enable wireframe mode
   */
  virtual void SetWireframeMode(bool enabled) = 0;

  // ===== Static API Selection =====

  /**
   * @brief Get the current rendering API
   */
  static RenderAPI GetAPI() { return s_CurrentAPI; }

  /**
   * @brief Set the rendering API to use
   *
   * Must be called before Renderer::Init()
   */
  static void SetAPI(RenderAPI api) { s_CurrentAPI = api; }

  /**
   * @brief Create a RendererAPI instance for the current API
   */
  static Scope<RendererAPI> Create();

private:
  static RenderAPI s_CurrentAPI;
};

} // namespace BeEngine
