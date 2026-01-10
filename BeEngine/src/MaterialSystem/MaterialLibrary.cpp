#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

void MaterialLibrary::Add(const std::string &name,
                          const Ref<Material> &material) {
  if (Exists(name)) {
    BE_CORE_WARN("MaterialLibrary: Material '{}' already exists, overwriting",
                 name);
  }
  m_Materials[name] = material;
  material->SetName(name);
  BE_CORE_TRACE("MaterialLibrary: Added material '{}'", name);
}

Ref<Material> MaterialLibrary::Create(const std::string &name,
                                      const Ref<Shader> &shader) {
  auto material = Material::Create(shader, name);
  Add(name, material);
  return material;
}

Ref<Material> MaterialLibrary::Get(const std::string &name) const {
  auto it = m_Materials.find(name);
  if (it == m_Materials.end()) {
    BE_CORE_ERROR("MaterialLibrary: Material '{}' not found", name);
    return nullptr;
  }
  return it->second;
}

bool MaterialLibrary::Exists(const std::string &name) const {
  return m_Materials.find(name) != m_Materials.end();
}

void MaterialLibrary::Remove(const std::string &name) {
  auto it = m_Materials.find(name);
  if (it != m_Materials.end()) {
    m_Materials.erase(it);
    BE_CORE_TRACE("MaterialLibrary: Removed material '{}'", name);
  }
}

void MaterialLibrary::Clear() {
  m_Materials.clear();
  BE_CORE_TRACE("MaterialLibrary: Cleared all materials");
}

std::vector<std::string> MaterialLibrary::GetAllNames() const {
  std::vector<std::string> names;
  names.reserve(m_Materials.size());
  for (const auto &[name, mat] : m_Materials) {
    names.push_back(name);
  }
  return names;
}
} // namespace BeEngine
