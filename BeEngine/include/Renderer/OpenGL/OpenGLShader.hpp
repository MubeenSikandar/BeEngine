// Renderer/OpenGL/OpenGLShader.hpp
#pragma once

#include "Renderer/Shader.hpp"
#include <unordered_map>
namespace BeEngine {

/**
 * @brief OpenGL implementation of shader program
 *
 * Compiles GLSL shaders and links them into a shader program
 * A shader program consists of:
 * - Vertex shader: Processes each vertex
 * - Fragment shader: Processes each pixel
 */
class OpenGLShader : public Shader {
public:
  /**
   * @brief Construct and compile OpenGL shader
   * @param vertexSrc GLSL vertex shader source code
   * @param fragmentSrc GLSL fragment shader source code
   *
   * This will:
   * 1. Compile vertex shader
   * 2. Compile fragment shader
   * 3. Link both into a program
   * 4. Report any compilation/linking errors
   */
  OpenGLShader(const std::string &vertexSrc, const std::string &fragmentSrc);

  /**
   * @brief Destructor - automatically deletes shader program
   */
  ~OpenGLShader() override;

  /**
   * @brief Bind this shader for rendering
   * Makes this shader program active (glUseProgram)
   */
  void Bind() const override;

  /**
   * @brief Unbind shader
   * Sets active program to 0
   */
  void Unbind() const override;

  void SetInt(const std::string &name, int value) override;
  void SetIntArray(const std::string &name, int *values,
                   uint32_t count) override;
  void SetFloat(const std::string &name, float value) override;
  void SetFloat2(const std::string &name, const glm::vec2 &value) override;
  void SetFloat3(const std::string &name, const glm::vec3 &value) override;
  void SetFloat4(const std::string &name, const glm::vec4 &value) override;
  void SetMat3(const std::string &name, const glm::mat3 &value) override;
  void SetMat4(const std::string &name, const glm::mat4 &value) override;
  void SetBool(const std::string &name, bool value) override;

private:
  uint32_t m_RendererID{0}; ///< OpenGL shader program ID

  /**
   * @brief Compile a single shader
   * @param type GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
   * @param source GLSL source code
   * @return Compiled shader ID, or 0 on failure
   */
  uint32_t CompileShader(uint32_t type, const std::string &source);

  // Uniform location cache
  mutable std::unordered_map<std::string, int> m_UniformLocationCache;
  int GetUniformLocation(const std::string &name) const;
};
} // namespace BeEngine
