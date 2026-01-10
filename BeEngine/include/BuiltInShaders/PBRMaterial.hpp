#pragma once

#include <Core.hpp>
#include <MaterialSystem/Material.hpp>
#include <Renderer/Shader.hpp>

namespace BeEngine {
/**
 * @brief PBR Material - Physically-Based Rendering (Metallic/Roughness
 * workflow)
 *
 * Perfect for:
 * - Realistic rendering
 * - Modern games
 * - Product visualization
 * - Architectural visualization
 *
 * Supports:
 * - Albedo (base color)
 * - Normal mapping
 * - Metallic/Roughness
 * - Ambient Occlusion
 * - Emissive
 */
class BE_API PBRMaterial {
public:
  PBRMaterial();
  explicit PBRMaterial(const std::string &name);
  ~PBRMaterial() = default;

  void Bind();

  NODISCARD Ref<Shader> GetShader() const { return m_Shader; }
  NODISCARD Ref<Material> GetMaterial() const { return m_Material; }

  // ===== Albedo (Base Color) =====
  void SetAlbedoColor(const glm::vec4 &color);
  NODISCARD glm::vec4 GetAlbedoColor() const { return m_AlbedoColor; }

  void SetAlbedoMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetAlbedoMap() const { return m_AlbedoMap; }

  // ===== Normal Map =====
  void SetNormalMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetNormalMap() const { return m_NormalMap; }

  // ===== Metallic =====
  void SetMetallic(float metallic);
  NODISCARD float GetMetallic() const { return m_Metallic; }

  void SetMetallicMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetMetallicMap() const { return m_MetallicMap; }

  // ===== Roughness =====
  void SetRoughness(float roughness);
  NODISCARD float GetRoughness() const { return m_Roughness; }

  void SetRoughnessMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetRoughnessMap() const { return m_RoughnessMap; }

  // ===== Ambient Occlusion =====
  void SetAO(float ao);
  NODISCARD float GetAO() const { return m_AO; }

  void SetAOMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetAOMap() const { return m_AOMap; }

  // ===== Emissive =====
  void SetEmissiveColor(const glm::vec3 &color);
  NODISCARD glm::vec3 GetEmissiveColor() const { return m_EmissiveColor; }

  void SetEmissiveIntensity(float intensity);
  NODISCARD float GetEmissiveIntensity() const { return m_EmissiveIntensity; }

  void SetEmissiveMap(const Ref<Texture2D> &texture);
  NODISCARD Ref<Texture2D> GetEmissiveMap() const { return m_EmissiveMap; }

  // ===== Render State =====
  void SetTransparent(bool transparent);
  void SetDoubleSided(bool doubleSided);

  NODISCARD const std::string &GetName() const { return m_Name; }

private:
  void CreateShader();
  void SyncProperties();

  std::string m_Name = "PBR Material";
  Ref<Shader> m_Shader;
  Ref<Material> m_Material;

  // Properties
  glm::vec4 m_AlbedoColor = glm::vec4(1.0F);
  float m_Metallic{0.0F};
  float m_Roughness{0.5F};
  float m_AO{1.0F};
  glm::vec3 m_EmissiveColor = glm::vec3(0.0F);
  float m_EmissiveIntensity{0.0F};

  // Textures
  Ref<Texture2D> m_AlbedoMap;
  Ref<Texture2D> m_NormalMap;
  Ref<Texture2D> m_MetallicMap;
  Ref<Texture2D> m_RoughnessMap;
  Ref<Texture2D> m_AOMap;
  Ref<Texture2D> m_EmissiveMap;
};
} // namespace BeEngine
