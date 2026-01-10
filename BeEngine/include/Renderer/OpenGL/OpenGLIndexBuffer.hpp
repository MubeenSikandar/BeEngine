#pragma once

#include <Core.hpp>
#include <Renderer/IndexBuffer.hpp>
#include <cstdint>
namespace BeEngine {
/**
 * @brief OpenGL implementation of index buffer
 *
 * Stores indices for indexed rendering using GL_ELEMENT_ARRAY_BUFFER
 * Used with glDrawElements() for efficient triangle rendering
 */

class OpenGLIndexBuffer : public IndexBuffer {
public:
  /**
   * @brief Construct OpenGL index buffer
   * @param indices Pointer to index data (array of uint32_t)
   * @param count Number of indices in the array
   */
  OpenGLIndexBuffer(const uint32_t *indices, uint32_t count);

  /**
   * @brief Destructor - automatically deletes OpenGL buffer
   */
  ~OpenGLIndexBuffer() override;

  /**
   * @brief Bind this index buffer for rendering
   * Makes this buffer the active GL_ELEMENT_ARRAY_BUFFER
   */
  void Bind() const override;

  /**
   * @brief Unbind index buffer
   * Sets GL_ELEMENT_ARRAY_BUFFER to 0
   */
  void Unbind() const override;

  /**
   * @brief Get the number of indices
   * @return Number of indices in this buffer
   */
  NODISCARD uint32_t GetCount() const override { return m_Count; }

private:
  uint32_t m_RendererID{0}; ///< OpenGL buffer object ID
  uint32_t m_Count{0};      ///< Number of indices
};
} // namespace BeEngine
