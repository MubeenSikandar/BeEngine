#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
PBRMaterial::PBRMaterial() : PBRMaterial("PBR Material") {}

PBRMaterial::PBRMaterial(const std::string &name) : m_Name(name) {
  CreateShader();
  m_Material = CreateRef<Material>(m_Shader, m_Name);
  SyncProperties();

  BE_CORE_TRACE("Created PBRMaterial: {}", m_Name);
}

void PBRMaterial::CreateShader() {
  m_Shader = Shader::Create(PBRVertex, PBRFragment);
}

void PBRMaterial::Bind() { m_Material->Bind(); }

// Albedo
void PBRMaterial::SetAlbedoColor(const glm::vec4 &color) {
  m_AlbedoColor = color;
  m_Material->SetFloat4("u_AlbedoColor", m_AlbedoColor);
}

void PBRMaterial::SetAlbedoMap(const Ref<Texture2D> &texture) {
  m_AlbedoMap = texture;
  if (m_AlbedoMap) {
    m_Material->SetTexture2D("u_AlbedoMap", m_AlbedoMap);
    m_Material->SetBool("u_UseAlbedoMap", true);
  } else {
    m_Material->SetBool("u_UseAlbedoMap", false);
  }
}

// Normal
void PBRMaterial::SetNormalMap(const Ref<Texture2D> &texture) {
  m_NormalMap = texture;
  if (m_NormalMap) {
    m_Material->SetTexture2D("u_NormalMap", m_NormalMap);
    m_Material->SetBool("u_UseNormalMap", true);
  } else {
    m_Material->SetBool("u_UseNormalMap", false);
  }
}

// Metallic
void PBRMaterial::SetMetallic(float metallic) {
  m_Metallic = glm::clamp(metallic, 0.0F, 1.0F);
  m_Material->SetFloat("u_Metallic", m_Metallic);
}

void PBRMaterial::SetMetallicMap(const Ref<Texture2D> &texture) {
  m_MetallicMap = texture;
  if (m_MetallicMap) {
    m_Material->SetTexture2D("u_MetallicMap", m_MetallicMap);
    m_Material->SetBool("u_UseMetallicMap", true);
  } else {
    m_Material->SetBool("u_UseMetallicMap", false);
  }
}

// Roughness
void PBRMaterial::SetRoughness(float roughness) {
  m_Roughness = glm::clamp(roughness, 0.04F, 1.0F);
  m_Material->SetFloat("u_Roughness", m_Roughness);
}

void PBRMaterial::SetRoughnessMap(const Ref<Texture2D> &texture) {
  m_RoughnessMap = texture;
  if (m_RoughnessMap) {
    m_Material->SetTexture2D("u_RoughnessMap", m_RoughnessMap);
    m_Material->SetBool("u_UseRoughnessMap", true);
  } else {
    m_Material->SetBool("u_UseRoughnessMap", false);
  }
}

// Ambient Occlusion
void PBRMaterial::SetAO(float ao) {
  m_AO = glm::clamp(ao, 0.0F, 1.0F);
  m_Material->SetFloat("u_AO", m_AO);
}

void PBRMaterial::SetAOMap(const Ref<Texture2D> &texture) {
  m_AOMap = texture;
  if (m_AOMap) {
    m_Material->SetTexture2D("u_AOMap", m_AOMap);
    m_Material->SetBool("u_UseAOMap", true);
  } else {
    m_Material->SetBool("u_UseAOMap", false);
  }
}

// Emissive
void PBRMaterial::SetEmissiveColor(const glm::vec3 &color) {
  m_EmissiveColor = color;
  m_Material->SetFloat3("u_EmissiveColor", m_EmissiveColor);
}

void PBRMaterial::SetEmissiveIntensity(float intensity) {
  m_EmissiveIntensity = glm::max(intensity, 0.0F);
  m_Material->SetFloat("u_EmissiveIntensity", m_EmissiveIntensity);
}

void PBRMaterial::SetEmissiveMap(const Ref<Texture2D> &texture) {
  m_EmissiveMap = texture;
  if (m_EmissiveMap) {
    m_Material->SetTexture2D("u_EmissiveMap", m_EmissiveMap);
    m_Material->SetBool("u_UseEmissiveMap", true);
  } else {
    m_Material->SetBool("u_UseEmissiveMap", false);
  }
}

// Render State
void PBRMaterial::SetTransparent(bool transparent) {
  if (transparent) {
    m_Material->SetBlendMode(Material::BlendMode::Transparent);
    m_Material->SetDepthWrite(false);
  } else {
    m_Material->SetBlendMode(Material::BlendMode::Opaque);
    m_Material->SetDepthWrite(true);
  }
}

void PBRMaterial::SetDoubleSided(bool doubleSided) {
  m_Material->SetCullMode(doubleSided ? Material::CullMode::None
                                      : Material::CullMode::Back);
}

void PBRMaterial::SyncProperties() {
  m_Material->SetFloat4("u_AlbedoColor", m_AlbedoColor);
  m_Material->SetFloat("u_Metallic", m_Metallic);
  m_Material->SetFloat("u_Roughness", m_Roughness);
  m_Material->SetFloat("u_AO", m_AO);
  m_Material->SetFloat3("u_EmissiveColor", m_EmissiveColor);
  m_Material->SetFloat("u_EmissiveIntensity", m_EmissiveIntensity);

  m_Material->SetBool("u_UseAlbedoMap", false);
  m_Material->SetBool("u_UseNormalMap", false);
  m_Material->SetBool("u_UseMetallicMap", false);
  m_Material->SetBool("u_UseRoughnessMap", false);
  m_Material->SetBool("u_UseAOMap", false);
  m_Material->SetBool("u_UseEmissiveMap", false);
}

} // namespace BeEngine
