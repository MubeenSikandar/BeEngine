#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

void ShaderLibrary::Add(const std::string &name, const Ref<Shader> &shader) {
  if (Exists(name)) {
    BE_CORE_WARN("ShaderLibrary: Shader '{}' already exists, overwriting",
                 name);
  }
  m_Shaders[name] = shader;
  BE_CORE_TRACE("ShaderLibrary: Added shader '{}'", name);
}

void ShaderLibrary::Add(const Ref<Shader> &shader) {
  // Use a default name based on the shader's address
  std::string name =
      "Shader_" + std::to_string(reinterpret_cast<uintptr_t>(shader.get()));
  Add(name, shader);
}

Ref<Shader> ShaderLibrary::Load(const std::filesystem::path &filepath) {
  std::string name = filepath.stem().string();
  return Load(name, filepath);
}

Ref<Shader> ShaderLibrary::Load(const std::string &name,
                                const std::filesystem::path &filepath) {
  if (!std::filesystem::exists(filepath)) {
    BE_CORE_ERROR("ShaderLibrary: File not found: {}", filepath.string());
    return nullptr;
  }

  // Read file content
  std::ifstream file(filepath);
  if (!file.is_open()) {
    BE_CORE_ERROR("ShaderLibrary: Could not open file: {}", filepath.string());
    return nullptr;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();
  file.close();

  // Parse shader source (expects #type vertex / #type fragment markers)
  std::string vertexSrc;
  std::string fragmentSrc;

  const std::string typeToken = "#type";
  size_t pos = source.find(typeToken);

  while (pos != std::string::npos) {
    // Find end of line
    size_t eol = source.find_first_of("\r\n", pos);
    if (eol == std::string::npos) {
      BE_CORE_ERROR("ShaderLibrary: Syntax error in shader file");
      return nullptr;
    }

    // Extract shader type
    size_t begin = pos + typeToken.length() + 1;
    std::string type = source.substr(begin, eol - begin);

    // Trim whitespace
    type.erase(0, type.find_first_not_of(" \t"));
    type.erase(type.find_last_not_of(" \t") + 1);

    // Find next #type or end of file
    size_t nextLinePos = source.find_first_not_of("\r\n", eol);
    pos = source.find(typeToken, nextLinePos);

    std::string shaderCode;
    if (pos == std::string::npos) {
      shaderCode = source.substr(nextLinePos);
    } else {
      shaderCode = source.substr(nextLinePos, pos - nextLinePos);
    }

    // Assign to correct shader type
    if (type == "vertex") {
      vertexSrc = shaderCode;
    } else if (type == "fragment" || type == "pixel") {
      fragmentSrc = shaderCode;
    } else {
      BE_CORE_WARN("ShaderLibrary: Unknown shader type '{}' in {}", type,
                   filepath.string());
    }
  }

  if (vertexSrc.empty() || fragmentSrc.empty()) {
    BE_CORE_ERROR("ShaderLibrary: Missing vertex or fragment shader in {}",
                  filepath.string());
    return nullptr;
  }

  auto shader = Shader::Create(vertexSrc, fragmentSrc);
  Add(name, shader);

  BE_CORE_INFO("ShaderLibrary: Loaded shader '{}' from {}", name,
               filepath.string());
  return shader;
}

Ref<Shader> ShaderLibrary::Load(const std::string &name,
                                const std::string &vertexSrc,
                                const std::string &fragmentSrc) {
  auto shader = Shader::Create(vertexSrc, fragmentSrc);
  Add(name, shader);
  return shader;
}

Ref<Shader> ShaderLibrary::Get(const std::string &name) const {
  auto it = m_Shaders.find(name);
  if (it == m_Shaders.end()) {
    BE_CORE_ERROR("ShaderLibrary: Shader '{}' not found", name);
    return nullptr;
  }
  return it->second;
}

bool ShaderLibrary::Exists(const std::string &name) const {
  return m_Shaders.find(name) != m_Shaders.end();
}

void ShaderLibrary::Remove(const std::string &name) {
  auto it = m_Shaders.find(name);
  if (it != m_Shaders.end()) {
    m_Shaders.erase(it);
    BE_CORE_TRACE("ShaderLibrary: Removed shader '{}'", name);
  }
}

void ShaderLibrary::Clear() {
  m_Shaders.clear();
  BE_CORE_TRACE("ShaderLibrary: Cleared all shaders");
}

std::vector<std::string> ShaderLibrary::GetAllNames() const {
  std::vector<std::string> names;
  names.reserve(m_Shaders.size());
  for (const auto &[name, shader] : m_Shaders) {
    names.push_back(name);
  }
  return names;
}

} // namespace BeEngine
