#pragma once

#include "Renderer/VertexBuffer.hpp"
#include <cstdint>

namespace BeEngine {

/**
 * @brief OpenGL implementation of vertex buffer
 *
 * Uses modern OpenGL (4.5+) Direct State Access (DSA) functions:
 * - glCreateBuffers() instead of glGenBuffers()
 * - Stores vertex data in GPU memory (GL_ARRAY_BUFFER)
 */

class OpenGLVertexBuffer : public VertexBuffer {
public:
  /**
   * @brief Construct OpenGL vertex buffer
   * @param vertices Pointer to vertex data
   * @param size Size of data in bytes
   */
  OpenGLVertexBuffer(const void *vertices, uint32_t size);

  /**
   * @brief Destructor - automatically deletes OpenGL buffer
   */
  ~OpenGLVertexBuffer() override;

  /**
   * @brief Bind this vertex buffer for rendering
   * Makes this buffer the active GL_ARRAY_BUFFER
   */
  void Bind() const override;

  /**
   * @brief Unbind vertex buffer
   * Sets GL_ARRAY_BUFFER to 0
   */
  void Unbind() const override;

private:
  uint32_t m_RendererID{0}; ///< OpenGL buffer object ID
};
} // namespace BeEngine
