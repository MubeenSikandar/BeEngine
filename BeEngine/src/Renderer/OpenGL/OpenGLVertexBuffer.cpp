// Renderer/OpenGL//OpenGLVertexBuffer.cpp
#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

OpenGLVertexBuffer::OpenGLVertexBuffer(const void *vertices, uint32_t size) {
  // Create buffer using Direct State Access (DSA) - OpenGL 4.5+
  // This is more modern than glGenBuffers + glBindBuffer
  if (GLAD_GL_VERSION_4_5 == 0) {
    glGenBuffers(1, &m_RendererID);
  } else {
    glCreateBuffers(1, &m_RendererID);
  }

  // Bind the buffer to GL_ARRAY_BUFFER target
  glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);

  // Upload vertex data to GPU
  // GL_STATIC_DRAW: Data won't change often (perfect for static geometry)
  glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

  BE_CORE_TRACE("OpenGL VertexBuffer created (ID: {}, Size: {} bytes)",
                m_RendererID, size);
}

OpenGLVertexBuffer::~OpenGLVertexBuffer() {
  // Delete buffer from GPU memory
  glDeleteBuffers(1, &m_RendererID);
  BE_CORE_TRACE("OpenGL VertexBuffer destroyed (ID: {})", m_RendererID);
}

void OpenGLVertexBuffer::Bind() const {
  // Make this buffer the active GL_ARRAY_BUFFER
  glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void OpenGLVertexBuffer::Unbind() const {
  // Unbind by setting GL_ARRAY_BUFFER to 0
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace BeEngine
