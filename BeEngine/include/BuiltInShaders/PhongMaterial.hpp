#pragma once

#include <Core.hpp>
#include <MaterialSystem/Material.hpp>
#include <Renderer/Shader.hpp>
namespace BeEngine {
/**
 * @brief Phong material - classic ambient + diffuse + specular lighting
 *
 * Perfect for:
 * - Simple 3D games
 * - Prototyping
 * - Retro/classic look
 * - Performance-critical applications
 */

class BE_API PhongMaterial {
public:
  PhongMaterial();
  explicit PhongMaterial(const std::string &name);
  ~PhongMaterial() = default;

  void Bind();

  NODISCARD Ref<Shader> GetShader() const { return m_Shader; }
  NODISCARD Ref<Material> GetMaterial() const { return m_Material; }

  // ===== Properties =====

  // Diffuse (main color/texture)
  void SetDiffuseColor(const glm::vec4 &color);
  NODISCARD glm::vec4 GetDiffuseColor() const { return m_DiffuseColor; }

  void SetDiffuseMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetDiffuseMap() const { return m_DiffuseMap; }

  // Specular (highlight)
  void SetSpecularColor(const glm::vec4 &color);
  NODISCARD glm::vec4 GetSpecularColor() const { return m_SpecularColor; }

  void SetSpecularMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetSpecularMap() const { return m_SpecularMap; }

  // Shininess (specular exponent, higher = tighter highlight)
  void SetShininess(float shininess);
  NODISCARD float GetShininess() const { return m_Shininess; }

  // Render state
  void SetTransparent(bool transparent);
  void SetDoubleSided(bool doubleSided);

  NODISCARD const std::string &GetName() const { return m_Name; }

private:
  void CreateShader();
  void SyncProperties();

  std::string m_Name = "Phong Material";
  Ref<Shader> m_Shader;
  Ref<Material> m_Material;

  // Properties
  glm::vec4 m_DiffuseColor = glm::vec4(1.0F);
  glm::vec4 m_SpecularColor = glm::vec4(1.0F);
  float m_Shininess{32.0F};

  Ref<Texture2D> m_DiffuseMap;
  Ref<Texture2D> m_SpecularMap;
};
} // namespace BeEngine
