#pragma once

#include <glm/glm.hpp>

namespace BeEngine {
/**
 * @brief Directional light (sun-like, infinite distance)
 */
struct DirectionalLight {
  glm::vec3 Direction = glm::vec3(0.0F, -1.0F, -0.5F);
  glm::vec3 Color = glm::vec3(1.0F, 1.0F, 1.0F);
  float Intensity = 1.0F;
  bool Enabled = true;
};

/**
 * @brief Point light (omni-directional, position-based)
 */
struct PointLight {
  glm::vec3 Position = glm::vec3(0.0F);
  glm::vec3 Color = glm::vec3(1.0F, 1.0F, 1.0F);
  float Intensity{1.0F};
  float Range{10.0F};
  bool Enabled{true};
};

/**
 * @brief Spot light (cone-shaped)
 */
struct SpotLight {
  glm::vec3 Position = glm::vec3(0.0F);
  glm::vec3 Direction = glm::vec3(0.0F, -1.0F, 0.0F);
  glm::vec3 Color = glm::vec3(1.0F, 1.0F, 1.0F);
  float Intensity{1.0F};
  float Range{10.0F};
  float InnerCone{12.5F}; // Degrees
  float OuterCone{17.5F}; // Degrees
  bool Enabled{true};
};

/**
 * @brief Ambient light settings
 */
struct AmbientLight {
  glm::vec3 Color = glm::vec3(0.1F, 0.1F, 0.12F);
};

} // namespace BeEngine
