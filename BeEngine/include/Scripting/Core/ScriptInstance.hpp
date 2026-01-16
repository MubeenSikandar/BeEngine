// ScriptInstance.hpp

#pragma once

#include <Core.hpp>
#include <Scene/ECS/Entity.hpp>
#include <Scripting/Core/ScriptField.hpp>
#include <Scripting/Core/ScriptTypes.hpp>
#include <glm/glm.hpp>

namespace BeEngine {
/**
 * @brief Base class for script instances attached to entities
 *
 * Each ScriptInstance represents one script attached to one entity.
 * Language-specific engines derive from this to provide their implementation.
 */

struct CollisionInfo {
  Entity other;
  glm::vec3 contactPoint;
  glm::vec3 normal;
  float impulse{0.0F};
};

class BE_API ScriptInstance {
public:
  ScriptInstance() = default;
  virtual ~ScriptInstance() = default;

  // Non-copyable, moveable
  ScriptInstance(const ScriptInstance &) = delete;
  ScriptInstance &operator=(const ScriptInstance &) = delete;
  ScriptInstance(ScriptInstance &&) = default;
  ScriptInstance &operator=(ScriptInstance &&) = default;

  // ===== Lifecycle =====

  virtual void OnCreate() {}
  virtual void OnDestroy() {}
  virtual void OnUpdate(float deltaTime) {}
  virtual void OnFixedUpdate(float fixedDelta) {}
  virtual void OnLateUpdate(float deltaTime) {}
  virtual void OnEnable() {}
  virtual void OnDisable() {}
  virtual void OnStart() {}

  // ===== Physics Callbacks =====

  virtual void OnCollisionEnter(const CollisionInfo &collision) {}
  virtual void OnCollisionStay(Entity other) {}
  virtual void OnCollisionExit(Entity other) {}
  virtual void OnTriggerEnter(Entity other) {}
  virtual void OnTriggerStay(Entity other) {}
  virtual void OnTriggerExit(Entity other) {}

  // ===== State =====

  void SetEntity(Entity entity) { m_Entity = entity; }
  NODISCARD Entity GetEntity() const { return m_Entity; }
  NODISCARD UUID GetEntityUUID() const {
    return m_Entity.IsValid() ? m_Entity.GetUUID()
                              : InvalidUUID; // Use InvalidUUID
  }

  void SetEnabled(bool enabled);
  NODISCARD bool IsEnabled() const { return m_Enabled; }

  NODISCARD ScriptState GetState() const { return m_State; }
  void SetState(ScriptState state) { m_State = state; }

  NODISCARD const std::string &GetScriptName() const { return m_ScriptName; }
  void SetScriptName(const std::string &name) { m_ScriptName = name; }

  NODISCARD const std::string &GetFilePath() const { return m_FilePath; }
  void SetFilePath(const std::string &path) { m_FilePath = path; }

  NODISCARD ScriptLanguage GetLanguage() const { return m_Language; }
  void SetLanguage(ScriptLanguage lang) { m_Language = lang; }

  // ===== Fields =====

  ScriptFieldMap &GetFields() { return m_Fields; }
  NODISCARD const ScriptFieldMap &GetFields() const { return m_Fields; }

  void SetField(const std::string &name, const ScriptFieldValue &value);
  NODISCARD ScriptFieldValue GetField(const std::string &name) const;

  template <typename T>
  void SetFieldValue(const std::string &name, const T &value) {
    if (auto *field = m_Fields.Get(name)) {
      field->value = value;
    }
  }

  template <typename T>
  T GetFieldValue(const std::string &name, const T &defaultValue = T{}) const {
    if (const auto *field = m_Fields.Get(name)) {
      if (auto *v = std::get_if<T>(&field->value)) {
        return *v;
      }
    }
    return defaultValue;
  }

  // ===== Errors =====

  NODISCARD bool HasError() const { return !m_LastError.message.empty(); }
  NODISCARD const ScriptError &GetLastError() const { return m_LastError; }
  void SetError(const ScriptError &error) {
    m_LastError = error;
    m_State = ScriptState::Error;
  }
  void ClearError() {
    m_LastError = ScriptError();
    if (m_State == ScriptState::Error) {
      m_State = ScriptState::Loaded;
    }
  }

  // ===== Profiling =====

  ScriptProfileData &GetProfileData() { return m_ProfileData; }
  NODISCARD const ScriptProfileData &GetProfileData() const {
    return m_ProfileData;
  }

  // ===== Hot Reload Support =====

  /// Save instance state before reload
  virtual void SaveState(ScriptFieldMap &outState) { outState = m_Fields; }

  /// Restore instance state after reload
  virtual void RestoreState(const ScriptFieldMap &inState) {
    m_Fields.CopyValuesFrom(inState);
  }

  /// Called when script is about to be reloaded
  virtual void OnBeforeReload() {}

  /// Called after script has been reloaded
  virtual void OnAfterReload() {}

protected:
  Entity m_Entity;
  std::string m_ScriptName;
  std::string m_FilePath;
  ScriptLanguage m_Language{ScriptLanguage::None};
  ScriptState m_State{ScriptState::Unloaded};
  bool m_Enabled{true};

  ScriptFieldMap m_Fields;
  ScriptError m_LastError;
  ScriptProfileData m_ProfileData;
};

/**
 * @brief Factory function type for creating script instances
 */
using ScriptInstanceFactory = std::function<Ref<ScriptInstance>(Entity entity)>;

} // namespace BeEngine
