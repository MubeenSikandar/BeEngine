// Material.hpp
#pragma once

#include <Core.hpp>
#include <MaterialSystem/Texture.hpp>
#include <Renderer/Shader.hpp>
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <variant>

namespace BeEngine {

/**
 * @brief Types of material properties
 */
enum class MaterialPropertyType : uint8_t {
  None = 0,
  Float,
  Float2,
  Float3,
  Float4,
  Int,
  Bool,
  Mat3,
  Mat4,
  Texture2D,
  TextureCube
};

/**
 * @brief A single material property value
 */
struct MaterialProperty {
  std::string Name;
  MaterialPropertyType Type = MaterialPropertyType::None;

  // Value storage using variant
  std::variant<float, glm::vec2, glm::vec3, glm::vec4, int, bool, glm::mat3,
               glm::mat4, Ref<Texture2D>, Ref<TextureCube>>
      Value;

  // For texture properties, which slot to bind to
  uint32_t TextureSlot = 0;
};

/**
 * @brief Material definition - shader + properties
 *
 * A Material represents a complete rendering configuration:
 * - Which shader to use
 * - All uniform values (colors, floats, textures, etc.)
 * - Rendering state (blend mode, cull mode, etc.)
 */
class BE_API Material {
public:
  /**
   * @brief Create a material with a shader
   * @param shader The shader this material uses
   * @param name Optional material name
   */
  explicit Material(const Ref<Shader> &shader,
                    std::string name = "Unnamed Material");

  virtual ~Material() = default;

  /**
   * @brief Factory method to create a material
   */
  static Ref<Material> Create(const Ref<Shader> &shader,
                              const std::string &name = "Unnamed Material");

  // ===== Property Setters =====
  void SetFloat(const std::string &name, float value);
  void SetFloat2(const std::string &name, const glm::vec2 &value);
  void SetFloat3(const std::string &name, const glm::vec3 &value);
  void SetFloat4(const std::string &name, const glm::vec4 &value);
  void SetInt(const std::string &name, int value);
  void SetBool(const std::string &name, bool value);
  void SetMat3(const std::string &name, const glm::mat3 &value);
  void SetMat4(const std::string &name, const glm::mat4 &value);
  void SetTexture2D(const std::string &name, const Ref<Texture2D> &texture,
                    uint32_t slot = 0);
  void SetTextureCube(const std::string &name, const Ref<TextureCube> &texture,
                      uint32_t slot = 0);

  // ===== Property Getters =====
  NODISCARD float GetFloat(const std::string &name) const;
  NODISCARD glm::vec2 GetFloat2(const std::string &name) const;
  NODISCARD glm::vec3 GetFloat3(const std::string &name) const;
  NODISCARD glm::vec4 GetFloat4(const std::string &name) const;
  NODISCARD int GetInt(const std::string &name) const;
  NODISCARD bool GetBool(const std::string &name) const;
  NODISCARD glm::mat3 GetMat3(const std::string &name) const;
  NODISCARD glm::mat4 GetMat4(const std::string &name) const;
  NODISCARD Ref<Texture2D> GetTexture2D(const std::string &name) const;
  NODISCARD Ref<TextureCube> GetTextureCube(const std::string &name) const;

  /**
   * @brief Check if a property exists
   */
  NODISCARD bool HasProperty(const std::string &name) const;

  /**
   * @brief Get all property names
   */
  NODISCARD std::vector<std::string> GetPropertyNames() const;

  /**
   * @brief Bind this material for rendering
   * Binds the shader and uploads all uniforms
   */
  void Bind();

  /**
   * @brief Unbind the material
   */
  void Unbind();

  // ===== Accessors =====
  NODISCARD const std::string &GetName() const { return m_Name; }
  void SetName(const std::string &name) { m_Name = name; }

  NODISCARD Ref<Shader> GetShader() const { return m_Shader; }
  void SetShader(const Ref<Shader> &shader) { m_Shader = shader; }

  // ===== Render State =====
  enum class BlendMode : uint8_t { Opaque, Transparent, Additive, Multiply };

  enum class CullMode : uint8_t { Back, Front, None };

  void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }
  NODISCARD BlendMode GetBlendMode() const { return m_BlendMode; }

  void SetCullMode(CullMode mode) { m_CullMode = mode; }
  NODISCARD CullMode GetCullMode() const { return m_CullMode; }

  void SetDepthTest(bool enabled) { m_DepthTest = enabled; }
  NODISCARD bool GetDepthTest() const { return m_DepthTest; }

  void SetDepthWrite(bool enabled) { m_DepthWrite = enabled; }
  NODISCARD bool GetDepthWrite() const { return m_DepthWrite; }

private:
  void UploadUniforms();
  void ApplyRenderState();

  template <typename T>
  void SetProperty(const std::string &name, MaterialPropertyType type,
                   const T &value);

  template <typename T>
  T GetProperty(const std::string &name, MaterialPropertyType type) const;

  std::string m_Name;
  Ref<Shader> m_Shader;
  std::unordered_map<std::string, MaterialProperty> m_Properties;

  // Render state
  BlendMode m_BlendMode = BlendMode::Opaque;
  CullMode m_CullMode = CullMode::Back;
  bool m_DepthTest = true;
  bool m_DepthWrite = true;

  // Track which texture slots are used
  uint32_t m_NextTextureSlot = 0;
};

} // namespace BeEngine
