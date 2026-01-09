#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {
Material::Material(const Ref<Shader> &shader, std::string name)
    : m_Name(std::move(name)), m_Shader(shader) {
  BE_CORE_TRACE("Material '{}' created", m_Name);
}

Ref<Material> Material::Create(const Ref<Shader> &shader,
                               const std::string &name) {
  return CreateRef<Material>(shader, name);
}

// ============================================================================
// Property Setters
// ============================================================================

template <typename T>
void Material::SetProperty(const std::string &name, MaterialPropertyType type,
                           const T &value) {
  MaterialProperty prop;
  prop.Name = name;
  prop.Type = type;
  prop.Value = value;
  m_Properties[name] = prop;
}

void Material::SetFloat(const std::string &name, float value) {
  SetProperty(name, MaterialPropertyType::Float, value);
}

void Material::SetFloat2(const std::string &name, const glm::vec2 &value) {
  SetProperty(name, MaterialPropertyType::Float2, value);
}

void Material::SetFloat3(const std::string &name, const glm::vec3 &value) {
  SetProperty(name, MaterialPropertyType::Float3, value);
}

void Material::SetFloat4(const std::string &name, const glm::vec4 &value) {
  SetProperty(name, MaterialPropertyType::Float4, value);
}

void Material::SetInt(const std::string &name, int value) {
  SetProperty(name, MaterialPropertyType::Int, value);
}

void Material::SetBool(const std::string &name, bool value) {
  SetProperty(name, MaterialPropertyType::Bool, value);
}

void Material::SetMat3(const std::string &name, const glm::mat3 &value) {
  SetProperty(name, MaterialPropertyType::Mat3, value);
}

void Material::SetMat4(const std::string &name, const glm::mat4 &value) {
  SetProperty(name, MaterialPropertyType::Mat4, value);
}

void Material::SetTexture2D(const std::string &name,
                            const Ref<Texture2D> &texture, uint32_t slot) {
  MaterialProperty prop;
  prop.Name = name;
  prop.Type = MaterialPropertyType::Texture2D;
  prop.Value = texture;
  prop.TextureSlot =
      (slot == 0 && m_Properties.find(name) == m_Properties.end())
          ? m_NextTextureSlot++
          : slot;
  m_Properties[name] = prop;
}

void Material::SetTextureCube(const std::string &name,
                              const Ref<TextureCube> &texture, uint32_t slot) {
  MaterialProperty prop;
  prop.Name = name;
  prop.Type = MaterialPropertyType::TextureCube;
  prop.Value = texture;
  prop.TextureSlot =
      (slot == 0 && m_Properties.find(name) == m_Properties.end())
          ? m_NextTextureSlot++
          : slot;
  m_Properties[name] = prop;
}

// ============================================================================
// Property Getters
// ============================================================================

template <typename T>
T Material::GetProperty(const std::string &name,
                        MaterialPropertyType type) const {
  auto it = m_Properties.find(name);
  if (it == m_Properties.end()) {
    BE_CORE_WARN("Material '{}': Property '{}' not found", m_Name, name);
    return T{};
  }
  if (it->second.Type != type) {
    BE_CORE_WARN("Material '{}': Property '{}' type mismatch", m_Name, name);
    return T{};
  }
  return std::get<T>(it->second.Value);
}

float Material::GetFloat(const std::string &name) const {
  return GetProperty<float>(name, MaterialPropertyType::Float);
}

glm::vec2 Material::GetFloat2(const std::string &name) const {
  return GetProperty<glm::vec2>(name, MaterialPropertyType::Float2);
}

glm::vec3 Material::GetFloat3(const std::string &name) const {
  return GetProperty<glm::vec3>(name, MaterialPropertyType::Float3);
}

glm::vec4 Material::GetFloat4(const std::string &name) const {
  return GetProperty<glm::vec4>(name, MaterialPropertyType::Float4);
}

int Material::GetInt(const std::string &name) const {
  return GetProperty<int>(name, MaterialPropertyType::Int);
}

bool Material::GetBool(const std::string &name) const {
  return GetProperty<bool>(name, MaterialPropertyType::Bool);
}

glm::mat3 Material::GetMat3(const std::string &name) const {
  return GetProperty<glm::mat3>(name, MaterialPropertyType::Mat3);
}

glm::mat4 Material::GetMat4(const std::string &name) const {
  return GetProperty<glm::mat4>(name, MaterialPropertyType::Mat4);
}

Ref<Texture2D> Material::GetTexture2D(const std::string &name) const {
  return GetProperty<Ref<Texture2D>>(name, MaterialPropertyType::Texture2D);
}

Ref<TextureCube> Material::GetTextureCube(const std::string &name) const {
  return GetProperty<Ref<TextureCube>>(name, MaterialPropertyType::TextureCube);
}

bool Material::HasProperty(const std::string &name) const {
  return m_Properties.find(name) != m_Properties.end();
}

std::vector<std::string> Material::GetPropertyNames() const {
  std::vector<std::string> names;
  names.reserve(m_Properties.size());
  for (const auto &[name, prop] : m_Properties) {
    names.push_back(name);
  }
  return names;
}

// ============================================================================
// Binding
// ============================================================================

void Material::Bind() {
  if (!m_Shader) {
    BE_CORE_ERROR("Material '{}': No shader assigned!", m_Name);
    return;
  }

  m_Shader->Bind();
  ApplyRenderState();
  UploadUniforms();
}

void Material::Unbind() {
  if (m_Shader) {
    m_Shader->Unbind();
  }
}

void Material::UploadUniforms() {
  for (const auto &[name, prop] : m_Properties) {
    switch (prop.Type) {
    case MaterialPropertyType::Float:
      m_Shader->SetFloat(name, std::get<float>(prop.Value));
      break;

    case MaterialPropertyType::Float2:
      m_Shader->SetFloat2(name, std::get<glm::vec2>(prop.Value));
      break;

    case MaterialPropertyType::Float3:
      m_Shader->SetFloat3(name, std::get<glm::vec3>(prop.Value));
      break;

    case MaterialPropertyType::Float4:
      m_Shader->SetFloat4(name, std::get<glm::vec4>(prop.Value));
      break;

    case MaterialPropertyType::Int:
      m_Shader->SetInt(name, std::get<int>(prop.Value));
      break;

    case MaterialPropertyType::Bool:
      m_Shader->SetBool(name, std::get<bool>(prop.Value));
      break;

    case MaterialPropertyType::Mat3:
      m_Shader->SetMat3(name, std::get<glm::mat3>(prop.Value));
      break;

    case MaterialPropertyType::Mat4:
      m_Shader->SetMat4(name, std::get<glm::mat4>(prop.Value));
      break;

    case MaterialPropertyType::Texture2D: {
      auto texture = std::get<Ref<Texture2D>>(prop.Value);
      if (texture) {
        texture->Bind(prop.TextureSlot);
        m_Shader->SetInt(name, static_cast<int>(prop.TextureSlot));
      }
      break;
    }

    case MaterialPropertyType::TextureCube: {
      auto texture = std::get<Ref<TextureCube>>(prop.Value);
      if (texture) {
        texture->Bind(prop.TextureSlot);
        m_Shader->SetInt(name, static_cast<int>(prop.TextureSlot));
      }
      break;
    }

    case MaterialPropertyType::None:
    default:
      break;
    }
  }
}

void Material::ApplyRenderState() {
  // Depth testing
  if (m_DepthTest) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }

  // Depth writing
  glDepthMask(m_DepthWrite ? GL_TRUE : GL_FALSE);

  // Culling
  switch (m_CullMode) {
  case CullMode::Back:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    break;
  case CullMode::Front:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    break;
  case CullMode::None:
    glDisable(GL_CULL_FACE);
    break;
  }

  // Blending
  switch (m_BlendMode) {
  case BlendMode::Opaque:
    glDisable(GL_BLEND);
    break;
  case BlendMode::Transparent:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  case BlendMode::Additive:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    break;
  case BlendMode::Multiply:
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    break;
  }
}

} // namespace BeEngine
