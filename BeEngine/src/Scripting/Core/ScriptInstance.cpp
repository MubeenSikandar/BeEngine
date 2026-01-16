// ScriptInstance.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
void ScriptInstance::SetEnabled(bool enabled) {
  if (m_Enabled == enabled) {
    return;
  }

  m_Enabled = enabled;

  if (m_State == ScriptState::Running || m_State == ScriptState::Paused) {
    if (enabled) {
      m_State = ScriptState::Running;
      OnEnable();
    } else {
      m_State = ScriptState::Paused;
      OnDisable();
    }
  }
}

void ScriptInstance::SetField(const std::string &name,
                              const ScriptFieldValue &value) {
  if (auto *field = m_Fields.Get(name)) {
    field->value = value;
  } else {
    BE_CORE_WARN("[ScriptInstance] Field '{}' not found in script '{}'", name,
                 m_ScriptName);
  }
}

ScriptFieldValue ScriptInstance::GetField(const std::string &name) const {
  if (const auto *field = m_Fields.Get(name)) {
    return field->value;
  }
  return std::monostate{};
}
} // namespace BeEngine
