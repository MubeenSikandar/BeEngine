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

  void AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) override;
  void SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer) override;

  NODISCARD const Ref<IndexBuffer> &GetIndexBuffer() const override {
    return m_IndexBuffer;
  }

private:
  uint32_t m_RendererID{0};
  std::vector<Ref<VertexBuffer>> m_VertexBuffers;
  Ref<IndexBuffer> m_IndexBuffer;
};

} // namespace BeEngine
