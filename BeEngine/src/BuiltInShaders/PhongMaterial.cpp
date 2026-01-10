#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
PhongMaterial::PhongMaterial() : PhongMaterial("Phong Material") {}

PhongMaterial::PhongMaterial(const std::string &name) : m_Name(name) {
  CreateShader();
  m_Material = CreateRef<Material>(m_Shader, m_Name);
  SyncProperties();

  BE_CORE_TRACE("Created PhongMaterial: {}", m_Name);
}

void PhongMaterial::CreateShader() {
  m_Shader = Shader::Create(PhongVertex, PhongFragment);
}

void PhongMaterial::Bind() { m_Material->Bind(); }

void PhongMaterial::SetDiffuseColor(const glm::vec4 &color) {
  m_DiffuseColor = color;
  m_Material->SetFloat4("u_DiffuseColor", m_DiffuseColor);
}

void PhongMaterial::SetDiffuseMap(const Ref<Texture2D> &texture) {
  m_DiffuseMap = texture;
  if (m_DiffuseMap) {
    m_Material->SetTexture2D("u_DiffuseMap", m_DiffuseMap);
    m_Material->SetBool("u_UseDiffuseMap", true);
  } else {
    m_Material->SetBool("u_UseDiffuseMap", false);
  }
}

void PhongMaterial::SetSpecularColor(const glm::vec4 &color) {
  m_SpecularColor = color;
  m_Material->SetFloat4("u_SpecularColor", m_SpecularColor);
}

void PhongMaterial::SetSpecularMap(const Ref<Texture2D> &texture) {
  m_SpecularMap = texture;
  if (m_SpecularMap) {
    m_Material->SetTexture2D("u_SpecularMap", m_SpecularMap);
    m_Material->SetBool("u_UseSpecularMap", true);
  } else {
    m_Material->SetBool("u_UseSpecularMap", false);
  }
}

void PhongMaterial::SetShininess(float shininess) {
  m_Shininess = glm::clamp(shininess, 1.0F, 512.0F);
  m_Material->SetFloat("u_Shininess", m_Shininess);
}

void PhongMaterial::SetTransparent(bool transparent) {
  if (transparent) {
    m_Material->SetBlendMode(Material::BlendMode::Transparent);
    m_Material->SetDepthWrite(false);
  } else {
    m_Material->SetBlendMode(Material::BlendMode::Opaque);
    m_Material->SetDepthWrite(true);
  }
}

void PhongMaterial::SetDoubleSided(bool doubleSided) {
  m_Material->SetCullMode(doubleSided ? Material::CullMode::None
                                      : Material::CullMode::Back);
}

void PhongMaterial::SyncProperties() {
  m_Material->SetFloat4("u_DiffuseColor", m_DiffuseColor);
  m_Material->SetFloat4("u_SpecularColor", m_SpecularColor);
  m_Material->SetFloat("u_Shininess", m_Shininess);
  m_Material->SetBool("u_UseDiffuseMap", m_DiffuseMap != nullptr);
  m_Material->SetBool("u_UseSpecularMap", m_SpecularMap != nullptr);
}

} // namespace BeEngine
