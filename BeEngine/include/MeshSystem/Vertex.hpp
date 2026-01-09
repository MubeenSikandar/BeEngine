// MeshSystem/Vertex.hpp
#pragma once

#include "Core.hpp"
#include "Renderer/BufferLayout.hpp"
#include <glm/glm.hpp>

namespace BeEngine {

/**
 * @brief Standard vertex format for 3D rendering
 *
 * Contains all attributes needed for PBR rendering:
 * - Position: World space position
 * - Normal: Surface normal for lighting
 * - Tangent: For normal mapping (with handedness in w)
 * - TexCoord: UV coordinates
 * - Color: Vertex color (optional tinting)
 */
struct Vertex {
  glm::vec3 Position = {0.0F, 0.0F, 0.0F};
  glm::vec3 Normal = {0.0F, 1.0F, 0.0F};
  glm::vec4 Tangent = {1.0F, 0.0F, 0.0F, 1.0F};
  glm::vec2 TexCoord = {0.0F, 0.0F};
  glm::vec4 Color = {1.0F, 1.0F, 1.0F, 1.0F};

  /**
   * @brief Get the vertex buffer layout for this vertex format
   */
  NODISCARD static BufferLayout GetLayout() {
    return {{ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float4, "a_Tangent"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float4, "a_Color"}};
  }

  BE_FORCE_INLINE bool operator==(const Vertex &other) const {
    return Position == other.Position && Normal == other.Normal &&
           Tangent == other.Tangent && TexCoord == other.TexCoord &&
           Color == other.Color;
  }
};

/**
 * @brief Simplified vertex for unlit/2D rendering
 */
struct SimpleVertex {
  glm::vec3 Position = {0.0F, 0.0F, 0.0F};
  glm::vec2 TexCoord = {0.0F, 0.0F};
  glm::vec4 Color = {1.0F, 1.0F, 1.0F, 1.0F};

  NODISCARD static BufferLayout GetLayout() {
    return {{ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float4, "a_Color"}};
  }
};

} // namespace BeEngine
