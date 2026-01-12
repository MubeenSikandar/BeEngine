#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
void LightManager::AddPointLight(const PointLight &light) {
  if (m_PointLights.size() < MAX_POINT_LIGHTS) {
    m_PointLights.push_back(light);
  } else {
    BE_CORE_WARN("Maximum point lights ({}) reached!", MAX_POINT_LIGHTS);
  }
}

void LightManager::RemovePointLight(uint32_t index) {
  if (index < m_PointLights.size()) {
    m_PointLights.erase(m_PointLights.begin() + index);
  }
}

void LightManager::AddSpotLight(const SpotLight &light) {
  if (m_SpotLights.size() >= MAX_SPOT_LIGHTS) {
    BE_CORE_WARN("Maximum spot lights ({}) reached!", MAX_SPOT_LIGHTS);
    return;
  }
  m_SpotLights.push_back(light);
}

void LightManager::RemoveSpotLight(uint32_t index) {
  if (index < m_SpotLights.size()) {
    m_SpotLights.erase(m_SpotLights.begin() + index);
  }
}

void LightManager::UploadToShader(const Ref<Shader> &shader,
                                  const glm::vec3 &cameraPos) const {
  if (!shader) {
    return;
  }

  shader->Bind();

  // Camera position
  shader->SetFloat3("u_CameraPos", cameraPos);

  // Ambient light
  shader->SetFloat3("u_AmbientLight", m_AmbientLight.Color);

  // Directional light
  if (m_DirectionalLight.Enabled) {
    shader->SetFloat3("u_DirLightDirection",
                      glm::normalize(m_DirectionalLight.Direction));
    shader->SetFloat3("u_DirLightColor", m_DirectionalLight.Color);
    shader->SetFloat("u_DirLightIntensity", m_DirectionalLight.Intensity);
  } else {
    shader->SetFloat("u_DirLightIntensity", 0.0F);
  }

  // Point lights
  int activeCount = 0;
  for (size_t i = 0; i < m_PointLights.size() && i < MAX_POINT_LIGHTS; i++) {
    const auto &light = m_PointLights[i];
    if (!light.Enabled) {
      continue;
    }

    std::string prefix =
        "u_PointLightPositions[" + std::to_string(activeCount) + "]";
    shader->SetFloat3(prefix, light.Position);

    prefix = "u_PointLightColors[" + std::to_string(activeCount) + "]";
    shader->SetFloat3(prefix, light.Color);

    prefix = "u_PointLightIntensities[" + std::to_string(activeCount) + "]";
    shader->SetFloat(prefix, light.Intensity);

    prefix = "u_PointLightRanges[" + std::to_string(activeCount) + "]";
    shader->SetFloat(prefix, light.Range);

    activeCount++;
  }

  shader->SetInt("u_PointLightCount", activeCount);

  // Spot lights
  shader->SetInt("u_SpotLightCount", static_cast<int>(m_SpotLights.size()));
  for (size_t i = 0; i < m_SpotLights.size() && i < MAX_SPOT_LIGHTS; i++) {
    std::string prefix = "u_SpotLights[" + std::to_string(i) + "].";
    const auto &light = m_SpotLights[i];

    shader->SetBool(prefix + "enabled", light.Enabled);
    shader->SetFloat3(prefix + "position", light.Position);
    shader->SetFloat3(prefix + "direction", glm::normalize(light.Direction));
    shader->SetFloat3(prefix + "color", light.Color);
    shader->SetFloat(prefix + "intensity", light.Intensity);
    shader->SetFloat(prefix + "range", light.Range);
    // Convert degrees to cosine for shader
    shader->SetFloat(prefix + "innerCutoff",
                     glm::cos(glm::radians(light.InnerCone)));
    shader->SetFloat(prefix + "outerCutoff",
                     glm::cos(glm::radians(light.OuterCone)));
  }
}
} // namespace BeEngine
