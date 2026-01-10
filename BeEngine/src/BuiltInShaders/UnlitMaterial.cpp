#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

UnlitMaterial::UnlitMaterial() : UnlitMaterial("Unlit Material") {}

UnlitMaterial::UnlitMaterial(const std::string &name) : m_Name(name) {
  CreateShader();
  m_Material = CreateRef<Material>(m_Shader, m_Name);
  SyncProperties();

  BE_CORE_TRACE("Created UnlitMaterial: {}", m_Name);
}

void UnlitMaterial::CreateShader() {
  m_Shader = Shader::Create(UnlitVertex, UnlitFragment);
}

void UnlitMaterial::Bind() { m_Material->Bind(); }

void UnlitMaterial::SetColor(const glm::vec4 &color) {
  m_Color = color;
  m_Material->SetFloat4("u_Color", m_Color);
}

void UnlitMaterial::SetAlbedoMap(const Ref<Texture2D> &texture) {
  m_AlbedoMap = texture;
  if (m_AlbedoMap) {
    m_Material->SetTexture2D("u_AlbedoMap", m_AlbedoMap);
    m_Material->SetBool("u_UseAlbedoMap", true);
  } else {
    m_Material->SetBool("u_UseAlbedoMap", false);
  }
}

void UnlitMaterial::SetAlphaCutoff(float cutoff) {
  m_AlphaCutoff = cutoff;
  m_Material->SetFloat("u_AlphaCutoff", m_AlphaCutoff);
}

void UnlitMaterial::SetTransparent(bool transparent) {
  if (transparent) {
    m_Material->SetBlendMode(Material::BlendMode::Transparent);
    m_Material->SetDepthWrite(false);
  } else {
    m_Material->SetBlendMode(Material::BlendMode::Opaque);
    m_Material->SetDepthWrite(true);
  }
}

void UnlitMaterial::SetDoubleSided(bool doubleSided) {
  m_Material->SetCullMode(doubleSided ? Material::CullMode::None
                                      : Material::CullMode::Back);
}

void UnlitMaterial::SyncProperties() {
  m_Material->SetFloat4("u_Color", m_Color);
  m_Material->SetBool("u_UseAlbedoMap", m_AlbedoMap != nullptr);
  m_Material->SetFloat("u_AlphaCutoff", m_AlphaCutoff);
}

} // namespace BeEngine
