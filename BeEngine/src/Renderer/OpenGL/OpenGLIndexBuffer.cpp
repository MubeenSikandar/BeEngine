// Renderer/OpenGL//OpenGLIndexBuffer.cpp
#include <PCH/BeEnginePCH.hpp>

#include <glad/glad.h>

namespace BeEngine {

OpenGLIndexBuffer::OpenGLIndexBuffer(const uint32_t *indices, uint32_t count)
    : m_Count(count) {

  if (GLAD_GL_VERSION_4_5) {
    // OpenGL 4.5+ (DSA)
    glCreateBuffers(1, &m_RendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices,
                 GL_STATIC_DRAW);
  } else {
    // OpenGL 4.1 (macOS)
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(uint32_t), indices,
                 GL_STATIC_DRAW);
  }

  BE_CORE_TRACE("OpenGL IndexBuffer created (ID: {}, Count: {})", m_RendererID,
                count);
}

OpenGLIndexBuffer::~OpenGLIndexBuffer() {
  glDeleteBuffers(1, &m_RendererID);
  BE_CORE_TRACE("OpenGL IndexBuffer destroyed (ID: {})", m_RendererID);
}

void OpenGLIndexBuffer::Bind() const {
  // Make this buffer the active GL_ELEMENT_ARRAY_BUFFER
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void OpenGLIndexBuffer::Unbind() const {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

} // namespace BeEngine
