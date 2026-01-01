#pragma once

#include "Core.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/VertexArray.hpp"
#include <vector>

namespace BeEngine {

/**
 * @brief OpenGL implementation of Vertex Array Object (VAO)
 *
 * A VAO stores:
 * - Vertex buffer bindings
 * - Vertex attribute configurations (layout)
 * - Index buffer binding
 *
 * This allows switching between different vertex configurations quickly
 */
class OpenGLVertexArray : public VertexArray {
public:
  /**
   * @brief Construct OpenGL VAO
   * Creates an empty VAO - buffers must be added separately
   */
  OpenGLVertexArray();

  /**
   * @brief Destructor - automatically deletes OpenGL VAO
   */
  ~OpenGLVertexArray() override;

  /**
   * @brief Bind this VAO for rendering
   * Makes this VAO the active vertex array
   */
  void Bind() const override;

  /**
   * @brief Unbind VAO
   * Sets active VAO to 0
   */
  void Unbind() const override;

  /**
   * @brief Add a vertex buffer to this VAO
   * @param vertexBuffer The buffer containing vertex data
   *
   * This configures the vertex attribute layout:
   * - Currently assumes: layout(location = 0) in vec3 position
   * - Can be extended to support multiple attributes (color, UV, etc.)
   */
  void AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) override;

  /**
   * @brief Set the index buffer for this VAO
   * @param indexBuffer The buffer containing indices
   *
   * VAO remembers the index buffer binding
   */
  void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) override;

  /**
   * @brief Get the index buffer bound to this VAO
   * @return Pointer to index buffer
   */
  NODISCARD const std::shared_ptr<IndexBuffer> &
  GetIndexBuffer() const override {
    return m_IndexBuffer;
  }

private:
  uint32_t m_RendererID{0};        ///< OpenGL VAO ID
  uint32_t m_VertexBufferIndex{0}; ///< Current vertex attribute location

  std::vector<std::shared_ptr<VertexBuffer>>
      m_VertexBuffers;                        ///< All vertex buffers
  std::shared_ptr<IndexBuffer> m_IndexBuffer; ///< Index buffer
};

} // namespace BeEngine
