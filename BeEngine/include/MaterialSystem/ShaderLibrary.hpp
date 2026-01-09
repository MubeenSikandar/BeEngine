#pragma once

#include "Core.hpp"
#include "Renderer/Shader.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace BeEngine {
/**
 * @brief Manages shader assets with caching and hot-reloading support
 */
class BE_API ShaderLibrary {
public:
  /**
   * @brief Add a shader to the library
   * @param name Unique identifier for the shader
   * @param shader The shader to add
   */
  void Add(const std::string &name, const Ref<Shader> &shader);

  /**
   * @brief Add a shader using its file name as the identifier
   */
  void Add(const Ref<Shader> &shader);

  /**
   * @brief Load shader from file and add to library
   * @param filepath Path to shader file (.glsl)
   * @return The loaded shader
   */
  Ref<Shader> Load(const std::filesystem::path &filepath);

  /**
   * @brief Load shader from file with custom name
   * @param name Custom identifier
   * @param filepath Path to shader file
   * @return The loaded shader
   */
  Ref<Shader> Load(const std::string &name,
                   const std::filesystem::path &filepath);

  /**
   * @brief Load shader from source strings
   * @param name Shader identifier
   * @param vertexSrc Vertex shader source
   * @param fragmentSrc Fragment shader source
   * @return The created shader
   */
  Ref<Shader> Load(const std::string &name, const std::string &vertexSrc,
                   const std::string &fragmentSrc);

  /**
   * @brief Get a shader by name
   * @param name Shader identifier
   * @return The shader, or nullptr if not found
   */
  NODISCARD Ref<Shader> Get(const std::string &name) const;

  /**
   * @brief Check if shader exists in library
   */
  NODISCARD bool Exists(const std::string &name) const;

  /**
   * @brief Remove a shader from the library
   */
  void Remove(const std::string &name);

  /**
   * @brief Clear all shaders from the library
   */
  void Clear();

  /**
   * @brief Get all shader names
   */
  NODISCARD std::vector<std::string> GetAllNames() const;

  /**
   * @brief Get the number of shaders in the library
   */
  NODISCARD size_t Size() const { return m_Shaders.size(); }

private:
  std::unordered_map<std::string, Ref<Shader>> m_Shaders;
};
} // namespace BeEngine
