#pragma once

#include <Core.hpp>
#include <MaterialSystem/Material.hpp>
#include <Renderer/Shader.hpp>

namespace BeEngine {
/**
 * @brief Unlit material - no lighting calculations
 *
 * Perfect for:
 * - UI elements
 * - Particles
 * - Stylized/toon rendering
 * - Debug visualization
 * - Skyboxes
 */
class BE_API UnlitMaterial {
public:
  UnlitMaterial();
  explicit UnlitMaterial(const std::string &name);
  ~UnlitMaterial() = default;

  /**
   * @brief Bind material for rendering
   */
  void Bind();

  /**
   * @brief Get the underlying shader
   */
  NODISCARD Ref<Shader> GetShader() const { return m_Shader; }

  /**
   * @brief Get the underlying material
   */
  NODISCARD Ref<Material> GetMaterial() const { return m_Material; }

  // ===== Properties =====

  void SetColor(const glm::vec4 &color);
  NODISCARD glm::vec4 GetColor() const { return m_Color; }

  void SetAlbedoMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetAlbedoMap() const { return m_AlbedoMap; }

  void SetAlphaCutoff(float cutoff);
  NODISCARD float GetAlphaCutoff() const { return m_AlphaCutoff; }

  // Render state shortcuts
  void SetTransparent(bool transparent);
  void SetDoubleSided(bool doubleSided);

  NODISCARD const std::string &GetName() const { return m_Name; }

private:
  void CreateShader();
  void SyncProperties();

  std::string m_Name = "Unlit Material";
  Ref<Shader> m_Shader;
  Ref<Material> m_Material;

  // Properties
  glm::vec4 m_Color = glm::vec4(1.0F);
  Ref<Texture2D> m_AlbedoMap;
  float m_AlphaCutoff{0.0F};
};

} // namespace BeEngine
