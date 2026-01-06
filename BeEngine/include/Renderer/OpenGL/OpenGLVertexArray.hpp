// Renderer/OpenGL/OpenGLVertexArray.hpp
#pragma once

#include "Core.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/VertexArray.hpp"
#include "Renderer/VertexBuffer.hpp"
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
  OpenGLVertexArray();
  ~OpenGLVertexArray() override;

  void Bind() const override;
  void Unbind() const override;

  void
  AddVertexBuffer(const std::shared_ptr<VertexBuffer> &vertexBuffer) override;
  void SetIndexBuffer(const std::shared_ptr<IndexBuffer> &indexBuffer) override;

  NODISCARD const std::shared_ptr<IndexBuffer> &
  GetIndexBuffer() const override {
    return m_IndexBuffer;
  }

private:
  uint32_t m_RendererID{0};
  std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
  std::shared_ptr<IndexBuffer> m_IndexBuffer;
};

} // namespace BeEngine
