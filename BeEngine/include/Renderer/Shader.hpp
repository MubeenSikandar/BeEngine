// Renderer/Shader.hpp
#pragma once
#include "Core.hpp"
#include "glm/fwd.hpp"
#include <memory>
#include <string>

namespace BeEngine {

/**
 * @brief Shader program - GPU program for rendering
 */
class BE_API Shader {
public:
  virtual ~Shader() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  /**
   * @brief Create a shader from source code
   * @param vertexSrc Vertex shader source code
   * @param fragmentSrc Fragment shader source code
   */
  static Ref<Shader> Create(const std::string &vertexSrc,
                            const std::string &fragmentSrc);

  // Uniform Setters
  virtual void SetInt(const std::string &name, int value) = 0;
  virtual void SetIntArray(const std::string &name, int *values,
                           uint32_t count) = 0;
  virtual void SetFloat(const std::string &name, float value) = 0;
  virtual void SetFloat2(const std::string &name, const glm::vec2 &value) = 0;
  virtual void SetFloat3(const std::string &name, const glm::vec3 &value) = 0;
  virtual void SetFloat4(const std::string &name, const glm::vec4 &value) = 0;
  virtual void SetMat3(const std::string &name, const glm::mat3 &value) = 0;
  virtual void SetMat4(const std::string &name, const glm::mat4 &value) = 0;
  virtual void SetBool(const std::string &name, bool value) = 0;
};
} // namespace BeEngine
