// Renderer/OpenGL//OpenGLVertexArray.cpp
#include "PCH/BeEnginePCH.hpp"

#include <glad/glad.h>

namespace BeEngine {

OpenGLVertexArray::OpenGLVertexArray() {
  // Create Vertex Array Object using DSA
  if (GLAD_GL_VERSION_4_5 == 0) {
    glGenVertexArrays(1, &m_RendererID);
  } else {
    glCreateVertexArrays(1, &m_RendererID);
  }
  BE_CORE_TRACE("OpenGL VertexArray created (ID: {})", m_RendererID);
}

OpenGLVertexArray::~OpenGLVertexArray() {
  glDeleteVertexArrays(1, &m_RendererID);
  BE_CORE_TRACE("OpenGL VertexArray destroyed (ID: {})", m_RendererID);
}

void OpenGLVertexArray::Bind() const {
  // Make this VAO active
  // All subsequent vertex attribute calls will affect this VAO
  glBindVertexArray(m_RendererID);
}

void OpenGLVertexArray::Unbind() const { glBindVertexArray(0); }

void OpenGLVertexArray::AddVertexBuffer(
    std::shared_ptr<VertexBuffer> vertexBuffer) {
  BE_CORE_ASSERT(vertexBuffer, "VertexBuffer is null!");

  glBindVertexArray(m_RendererID);
  vertexBuffer->Bind();

  glEnableVertexAttribArray(m_VertexBufferIndex);
  glVertexAttribPointer(m_VertexBufferIndex,
                        3, // vec3
                        GL_FLOAT, GL_FALSE,
                        3 * sizeof(float), // stride
                        (const void *)0);

  m_VertexBuffers.push_back(vertexBuffer);
  m_VertexBufferIndex++;

  // Unbind to prevent accidental modification
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  BE_CORE_TRACE("VertexBuffer added to VAO (Attribute Location: {})",
                m_VertexBufferIndex - 1);
}

void OpenGLVertexArray::SetIndexBuffer(
    std::shared_ptr<IndexBuffer> indexBuffer) {
  BE_CORE_ASSERT(indexBuffer, "IndexBuffer is null!");

  glBindVertexArray(m_RendererID);
  indexBuffer->Bind();
  m_IndexBuffer = indexBuffer;

  // Unbind VAO (this preserves the EBO binding inside the VAO)
  glBindVertexArray(0);
  // Do NOT unbind the EBO here - it would break the VAO's reference

  BE_CORE_TRACE("IndexBuffer set for VAO (Count: {})", indexBuffer->GetCount());
}

} // namespace BeEngine
