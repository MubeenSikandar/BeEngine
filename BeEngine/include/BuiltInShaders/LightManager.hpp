#pragma once

#include <BuiltInShaders/Light.hpp>
#include <Core.hpp>
#include <Renderer/Shader.hpp>

namespace BeEngine {
/**
 * @brief Manages lights in the scene and uploads them to shaders
 */
class BE_API LightManager {
public:
  static constexpr uint32_t MAX_POINT_LIGHTS = 4;
  static constexpr uint32_t MAX_SPOT_LIGHTS = 4;

  LightManager() = default;
  ~LightManager() = default;

  // Directional Light (only one)
  void SetDirectionalLight(const DirectionalLight &light) {
    m_DirectionalLight = light;
  }
  NODISCARD DirectionalLight &GetDirectionalLight() {
    return m_DirectionalLight;
  }
  NODISCARD const DirectionalLight &GetDirectionalLight() const {
    return m_DirectionalLight;
  }

  // Ambient Light
  void SetAmbientLight(const AmbientLight &light) { m_AmbientLight = light; }
  NODISCARD AmbientLight &GetAmbientLight() { return m_AmbientLight; }
  NODISCARD const AmbientLight &GetAmbientLight() const {
    return m_AmbientLight;
  }

  // Point Lights
  void AddPointLight(const PointLight &light);
  void RemovePointLight(uint32_t index);
  void ClearPointLights() { m_PointLights.clear(); }
  NODISCARD std::vector<PointLight> &GetPointLights() { return m_PointLights; }
  NODISCARD const std::vector<PointLight> &GetPointLights() const {
    return m_PointLights;
  }
  NODISCARD uint32_t GetPointLightCount() const {
    return static_cast<uint32_t>(m_PointLights.size());
  }

  /**
   * @brief Upload all light data to a shader
   * @param shader The shader to upload to
   * @param cameraPos Camera position for specular calculations
   */
  void UploadToShader(const Ref<Shader> &shader,
                      const glm::vec3 &cameraPos) const;

private:
  DirectionalLight m_DirectionalLight;
  AmbientLight m_AmbientLight;
  std::vector<PointLight> m_PointLights;
};
} // namespace BeEngine
