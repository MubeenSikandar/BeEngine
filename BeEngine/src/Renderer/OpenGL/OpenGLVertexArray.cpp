// Renderer/OpenGL/OpenGLVertexArray.cpp
#include <PCH/BeEnginePCH.hpp>
#include <glad/glad.h>

namespace BeEngine {

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
  switch (type) {
  case ShaderDataType::Float:
    return GL_FLOAT;
  case ShaderDataType::Float2:
    return GL_FLOAT;
  case ShaderDataType::Float3:
    return GL_FLOAT;
  case ShaderDataType::Float4:
    return GL_FLOAT;
  case ShaderDataType::Int:
    return GL_INT;
  case ShaderDataType::Int2:
    return GL_INT;
  case ShaderDataType::Int3:
    return GL_INT;
  case ShaderDataType::Int4:
    return GL_INT;
  case ShaderDataType::Mat3:
    return GL_FLOAT;
  case ShaderDataType::Mat4:
    return GL_FLOAT;
  case ShaderDataType::Bool:
    return GL_BOOL;
  case ShaderDataType::None:
    return 0;
  }
  return 0;
}

OpenGLVertexArray::OpenGLVertexArray() {
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

void OpenGLVertexArray::Bind() const { glBindVertexArray(m_RendererID); }

void OpenGLVertexArray::Unbind() const { glBindVertexArray(0); }

void OpenGLVertexArray::AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) {
  const auto &layout = vertexBuffer->GetLayout();

  if (layout.GetElements().empty()) {
    BE_CORE_WARN(
        "VertexBuffer has no layout! Call SetLayout before AddVertexBuffer.");
  }

  glBindVertexArray(m_RendererID);
  vertexBuffer->Bind();

  uint32_t index = 0;
  for (const auto &element : layout) {
    switch (element.Type) {
    case ShaderDataType::Float:
    case ShaderDataType::Float2:
    case ShaderDataType::Float3:
    case ShaderDataType::Float4: {
      glEnableVertexAttribArray(index);
      glVertexAttribPointer(index,
                            static_cast<GLint>(element.GetComponentCount()),
                            ShaderDataTypeToOpenGLBaseType(element.Type),
                            element.Normalized ? GL_TRUE : GL_FALSE,
                            static_cast<GLsizei>(layout.GetStride()),
                            reinterpret_cast<const void *>(
                                static_cast<uintptr_t>(element.Offset)));
      index++;
      break;
    }
    case ShaderDataType::Int:
    case ShaderDataType::Int2:
    case ShaderDataType::Int3:
    case ShaderDataType::Int4:
    case ShaderDataType::Bool: {
      glEnableVertexAttribArray(index);
      glVertexAttribIPointer(index,
                             static_cast<GLint>(element.GetComponentCount()),
                             ShaderDataTypeToOpenGLBaseType(element.Type),
                             static_cast<GLsizei>(layout.GetStride()),
                             reinterpret_cast<const void *>(
                                 static_cast<uintptr_t>(element.Offset)));
      index++;
      break;
    }
    case ShaderDataType::Mat3:
    case ShaderDataType::Mat4: {
      uint32_t count = element.GetComponentCount();
      for (uint32_t i = 0; i < count; i++) {
        glEnableVertexAttribArray(index);
        glVertexAttribPointer(
            index, static_cast<GLint>(count),
            ShaderDataTypeToOpenGLBaseType(element.Type),
            element.Normalized ? GL_TRUE : GL_FALSE,
            static_cast<GLsizei>(layout.GetStride()),
            reinterpret_cast<const void *>(static_cast<uintptr_t>(
                element.Offset + sizeof(float) * count * i)));
        glVertexAttribDivisor(index, 1);
        index++;
      }
      break;
    }
    case ShaderDataType::None:
      break;
    }
  }

  m_VertexBuffers.push_back(vertexBuffer);
}

void OpenGLVertexArray::SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer) {
  BE_CORE_ASSERT(indexBuffer, "IndexBuffer is null!");

  glBindVertexArray(m_RendererID);
  indexBuffer->Bind();
  m_IndexBuffer = indexBuffer;

  glBindVertexArray(0);

  BE_CORE_TRACE("IndexBuffer set for VAO (Count: {})", indexBuffer->GetCount());
}

} // namespace BeEngine
