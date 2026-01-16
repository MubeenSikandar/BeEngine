// ScriptField.hpp
#pragma once

#include "Core.hpp"
#include <Scripting/Core/ScriptTypes.hpp>
#include <glm/glm.hpp>
#include <string>
#include <variant>

namespace BeEngine {
/**
 * @brief Represents a single field in a script that can be exposed to the
 * editor
 */

struct ScriptField {
  std::string name;
  ScriptFieldType type{ScriptFieldType::None};
  ScriptFieldValue value;
  ScriptFieldValue defaultValue;

  // Editor metadata
  std::string displayName;
  std::string tooltip;
  std::string category;
  bool isPublic{true};
  bool isReadOnly{false};
  bool isSerializable{true};

  // Validation
  bool hasRange{false};
  double minValue{0.0};
  double maxValue{0.0};

  // For entity/asset references
  std::string assetType; // e.g., "Texture", "Material", "Prefab"

  ScriptField() = default;

  ScriptField(const std::string &fieldName, ScriptFieldType fieldType)
      : name(fieldName), type(fieldType), displayName(fieldName) {}

  // Type-safe setters
  void SetBool(bool v) {
    value = v;
    type = ScriptFieldType::Bool;
  }

  void SetInt(int32_t v) {
    value = v;
    type = ScriptFieldType::Int32;
  }

  void SetFloat(float v) {
    value = v;
    type = ScriptFieldType::Float;
  }

  void SetDouble(double v) {
    value = v;
    type = ScriptFieldType::Double;
  }

  void SetString(const std::string &v) {
    value = v;
    type = ScriptFieldType::String;
  }

  void SetVec2(const glm::vec2 &v) {
    value = v;
    type = ScriptFieldType::Vec2;
  }

  void SetVec3(const glm::vec3 &v) {
    value = v;
    type = ScriptFieldType::Vec3;
  }

  void SetVec4(const glm::vec4 &v) {
    value = v;
    type = ScriptFieldType::Vec4;
  }

  void SetColor(const Color &v) {
    value = v;
    type = ScriptFieldType::Color;
  }

  void SetColor(const glm::vec4 &v) {
    value = Color(v);
    type = ScriptFieldType::Color;
  }

  void SetQuat(const glm::quat &v) {
    value = v;
    type = ScriptFieldType::Quat;
  }

  void SetEntity(UUID v) {
    value = EntityRef{v};
    type = ScriptFieldType::Entity;
  }

  void SetAsset(UUID v) {
    value = AssetRef{v};
    type = ScriptFieldType::Asset;
  }

  void SetPrefab(UUID v) {
    value = PrefabRef{v};
    type = ScriptFieldType::Prefab;
  }

  template <typename T> void SetValue(T v) {
    value = v;
    type = GetScriptFieldType<T>();
  }

  // Type-safe getters
  NODISCARD bool GetBool(bool fallback = false) const {
    if (const auto *v = std::get_if<bool>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD int32_t GetInt(int32_t fallback = 0) const {
    if (const auto *v = std::get_if<int32_t>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD float GetFloat(float fallback = 0.0F) const {
    if (const auto *v = std::get_if<float>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD double GetDouble(double fallback = 0.0) const {
    if (const auto *v = std::get_if<double>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD std::string GetString(const std::string &fallback = "") const {
    if (const auto *v = std::get_if<std::string>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD glm::vec2
  GetVec2(const glm::vec2 &fallback = glm::vec2(0.0F)) const {
    if (const auto *v = std::get_if<glm::vec2>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD glm::vec3
  GetVec3(const glm::vec3 &fallback = glm::vec3(0.0F)) const {
    if (const auto *v = std::get_if<glm::vec3>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD glm::vec4
  GetVec4(const glm::vec4 &fallback = glm::vec4(0.0F)) const {
    if (const auto *v = std::get_if<glm::vec4>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD glm::quat
  GetQuat(const glm::quat &fallback = glm::identity<glm::quat>()) const {
    if (const auto *v = std::get_if<glm::quat>(&value)) {
      return *v;
    }
    return fallback;
  }

  NODISCARD Color GetColor(const Color &fallback = Color()) const {
    if (const auto *v = std::get_if<Color>(&value)) {
      return *v;
    }
    return fallback;
  }

  template <typename T> NODISCARD T GetValue(T fallback = T{}) const {
    if (const auto *v = std::get_if<T>(&value)) {
      return *v;
    }
    return fallback;
  }

  // Entity/Asset stored as uint64_t
  NODISCARD UUID GetEntity(UUID fallback = InvalidUUID) const {
    if (const auto *v = std::get_if<EntityRef>(&value)) {
      return v->id;
    }
    return fallback;
  }

  NODISCARD UUID GetAsset(UUID fallback = InvalidUUID) const {
    if (const auto *v = std::get_if<AssetRef>(&value)) {
      return v->id;
    }
    return fallback;
  }

  NODISCARD UUID GetPrefab(UUID fallback = InvalidUUID) const {
    if (const auto *v = std::get_if<PrefabRef>(&value)) {
      return v->id;
    }
    return fallback;
  }

  // Range helpers
  ScriptField &WithRange(double min, double max) {
    hasRange = true;
    minValue = min;
    maxValue = max;
    return *this;
  }

  ScriptField &WithTooltip(const std::string &tip) {
    tooltip = tip;
    return *this;
  }

  ScriptField &WithCategory(const std::string &cat) {
    category = cat;
    return *this;
  }
};

/**
 * @brief Collection of script fields with lookup
 */
class ScriptFieldMap {
public:
  using FieldMap = std::unordered_map<std::string, ScriptField>;
  using iterator = FieldMap::iterator;
  using const_iterator = FieldMap::const_iterator;

  void Add(const ScriptField &field) {
    auto [it, inserted] = m_Fields.try_emplace(field.name, field);
    if (inserted) {
      m_Order.push_back(field.name);
    } else {
      it->second = field;
    }
  }

  void Add(const std::string &name, ScriptFieldType type) {
    ScriptField field(name, type);
    Add(field);
  }

  NODISCARD bool Has(const std::string &name) const {
    return m_Fields.contains(name);
  }

  ScriptField *Get(const std::string &name) {
    auto it = m_Fields.find(name);
    return it != m_Fields.end() ? &it->second : nullptr;
  }

  NODISCARD const ScriptField *Get(const std::string &name) const {
    auto it = m_Fields.find(name);
    return it != m_Fields.end() ? &it->second : nullptr;
  }

  void Remove(const std::string &name) {
    m_Fields.erase(name);
    m_Order.erase(std::remove(m_Order.begin(), m_Order.end(), name),
                  m_Order.end());
  }

  void Clear() {
    m_Fields.clear();
    m_Order.clear();
  }

  NODISCARD size_t Size() const { return m_Fields.size(); }
  NODISCARD bool Empty() const { return m_Fields.empty(); }

  // Ordered iteration (preserves add order)
  NODISCARD const std::vector<std::string> &GetOrder() const { return m_Order; }

  // Standard iteration
  iterator begin() { return m_Fields.begin(); }
  iterator end() { return m_Fields.end(); }
  NODISCARD const_iterator begin() const { return m_Fields.begin(); }
  NODISCARD const_iterator end() const { return m_Fields.end(); }

  // Copy fields from another map (for serialization)
  void CopyValuesFrom(const ScriptFieldMap &other) {
    for (const auto &[name, field] : other.m_Fields) {
      if (auto *myField = Get(name)) {
        if (myField->type == field.type) {
          myField->value = field.value;
        }
      }
    }
  }

private:
  FieldMap m_Fields;
  std::vector<std::string> m_Order; // Maintains insertion order
};

} // namespace BeEngine
