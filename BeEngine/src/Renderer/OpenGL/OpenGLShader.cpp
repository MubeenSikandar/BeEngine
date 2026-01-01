// Renderer/OpenGL/OpenGLShader.cpp
#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

OpenGLShader::OpenGLShader(const std::string &vertexSrc,
                           const std::string &fragmentSrc) {
  // Step 1: Compile vertex shader
  uint32_t vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);

  // Step 2: Compile fragment shader
  uint32_t fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

  // Check if compilation succeeded
  if (vertexShader == 0 || fragmentShader == 0) {
    BE_CORE_ERROR("Shader compilation failed!");
    if (vertexShader) {
      glDeleteShader(vertexShader);
    }
    if (fragmentShader) {
      glDeleteShader(fragmentShader);
    }
    return;
  }

  // Step 3: Create shader program
  m_RendererID = glCreateProgram();

  // Step 4: Attach shaders to program
  glAttachShader(m_RendererID, vertexShader);
  glAttachShader(m_RendererID, fragmentShader);

  // Step 5: Link program
  glLinkProgram(m_RendererID);

  // Step 6: Check linking status
  GLint success;
  glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
  if (!success) {
    // Get error log length
    GLint length;
    glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &length);

    // Get error log
    std::vector<char> infoLog(length);
    glGetProgramInfoLog(m_RendererID, length, &length, infoLog.data());

    BE_CORE_ERROR("Shader linking failed: {}", infoLog.data());

    // Cleanup on failure
    glDeleteProgram(m_RendererID);
    m_RendererID = 0;
  }

  // Step 7: Cleanup individual shaders
  // They're now linked into the program, so we don't need them anymore
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  if (m_RendererID) {
    BE_CORE_INFO("OpenGL Shader created successfully (ID: {})", m_RendererID);
  }
}

OpenGLShader::~OpenGLShader() {
  if (m_RendererID) {
    glDeleteProgram(m_RendererID);
    BE_CORE_TRACE("OpenGL Shader destroyed (ID: {})", m_RendererID);
  }
}

void OpenGLShader::Bind() const { glUseProgram(m_RendererID); }

void OpenGLShader::Unbind() const { glUseProgram(0); }

uint32_t OpenGLShader::CompileShader(uint32_t type, const std::string &source) {
  // Step 1: Create shader object
  uint32_t shader = glCreateShader(type);

  // Step 2: Set shader source code
  const char *sourceCStr = source.c_str();
  glShaderSource(shader, 1, &sourceCStr, nullptr);

  // Step 3: Compile shader
  glCompileShader(shader);

  // Step 4: Check compilation status
  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    // Get error log length
    GLint length;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    // Get error log
    std::vector<char> infoLog(length);
    glGetShaderInfoLog(shader, length, &length, infoLog.data());

    // Determine shader type for error message
    const char *shaderType = (type == GL_VERTEX_SHADER) ? "Vertex" : "Fragment";
    BE_CORE_ERROR("{} shader compilation failed: {}", shaderType,
                  infoLog.data());

    // Cleanup and return 0 (failure)
    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

} // namespace BeEngine
