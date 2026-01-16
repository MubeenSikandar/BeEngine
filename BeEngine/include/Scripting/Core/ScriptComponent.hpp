// ScriptComponent.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Core/ScriptInstance.hpp>
#include <chrono>
#include <vector>

namespace BeEngine {
/**
 * @brief ECS Component for attaching scripts to entities
 *
 * An entity can have multiple scripts attached via a single ScriptComponent.
 */
class BE_API ScriptComponent {
public:
  // ===== Script Instances =====
  std::vector<Ref<ScriptInstance>> Scripts;

  // ===== Serialized Data =====
  // Used for saving/loading scripts before instances are created
  struct ScriptData {
    std::string scriptName;
    ScriptLanguage language{ScriptLanguage::None};
    ScriptFieldMap fields;
    bool enabled{true};
  };
  std::vector<ScriptData> ScriptDatas;

  // ===== Methods =====

  ScriptComponent() = default;

  /// Add a script by name
  void AddScript(const std::string &scriptName) {
    ScriptData data;
    data.scriptName = scriptName;
    data.enabled = true;
    ScriptDatas.push_back(std::move(data));
  }

  /// Add a script instance
  void AddInstance(Ref<ScriptInstance> instance) {
    if (instance) {
      Scripts.push_back(std::move(instance));
    }
  }

  /// Remove a script by name
  void RemoveScript(const std::string &scriptName) {
    // Remove instance
    Scripts.erase(std::remove_if(Scripts.begin(), Scripts.end(),
                                 [&](const Ref<ScriptInstance> &inst) {
                                   return inst &&
                                          inst->GetScriptName() == scriptName;
                                 }),
                  Scripts.end());

    // Remove data
    ScriptDatas.erase(std::remove_if(ScriptDatas.begin(), ScriptDatas.end(),
                                     [&](const ScriptData &data) {
                                       return data.scriptName == scriptName;
                                     }),
                      ScriptDatas.end());
  }

  /// Remove a script instance
  void RemoveInstance(const Ref<ScriptInstance> &instance) {
    Scripts.erase(std::remove(Scripts.begin(), Scripts.end(), instance),
                  Scripts.end());
  }

  /// Get script instance by name
  NODISCARD Ref<ScriptInstance> GetScript(const std::string &scriptName) {
    for (auto &script : Scripts) {
      if (script && script->GetScriptName() == scriptName) {
        return script;
      }
    }
    return nullptr;
  }

  NODISCARD Ref<ScriptInstance> GetScriptAt(size_t index) {
    return index < Scripts.size() ? Scripts[index] : nullptr;
  }

  /// Check if entity has a specific script
  NODISCARD bool HasScript(const std::string &scriptName) const {
    for (const auto &script : Scripts) {
      if (script && script->GetScriptName() == scriptName) {
        return true;
      }
    }
    for (const auto &data : ScriptDatas) {
      if (data.scriptName == scriptName) {
        return true;
      }
    }
    return false;
  }

  template <typename T> NODISCARD Ref<T> GetScript() {
    static_assert(std::is_base_of_v<ScriptInstance, T>,
                  "T must derive from ScriptInstance");
    for (auto &script : Scripts) {
      if (auto typed = std::dynamic_pointer_cast<T>(script)) {
        return typed;
      }
    }
    return nullptr;
  }

  template <typename T> NODISCARD bool HasScript() const {
    static_assert(std::is_base_of_v<ScriptInstance, T>,
                  "T must derive from ScriptInstance");
    for (const auto &script : Scripts) {
      if (std::dynamic_pointer_cast<T>(script)) {
        return true;
      }
    }
    return false;
  }

  /// Get number of scripts
  NODISCARD size_t GetScriptCount() const { return Scripts.size(); }

  /// Clear all scripts
  void Clear() {
    for (auto &script : Scripts) {
      if (script && script->GetState() == ScriptState::Running) {
        script->OnDestroy();
      }
    }
    Scripts.clear();
    ScriptDatas.clear();
  }

  void SetScriptEnabled(const std::string &scriptName, bool enabled) {
    for (auto &script : Scripts) {
      if (script && script->GetScriptName() == scriptName) {
        script->SetEnabled(enabled); // This calls OnEnable/OnDisable internally
        break;
      }
    }
    // Also update ScriptDatas for serialization
    for (auto &data : ScriptDatas) {
      if (data.scriptName == scriptName) {
        data.enabled = enabled;
        break;
      }
    }
  }

  // ===== Lifecycle Dispatch =====

  void OnCreate() {
    for (auto &script : Scripts) {
      if (script && script->IsEnabled()) {
        script->SetState(ScriptState::Running);
        script->OnCreate();
      }
    }
  }

  void OnStart() {
    for (auto &script : Scripts) {
      if (script && script->IsEnabled() &&
          script->GetState() == ScriptState::Running) {
        script->OnStart();
      }
    }
  }

  void OnDestroy() {
    for (auto &script : Scripts) {
      if (script) {
        script->OnDestroy();
        script->SetState(ScriptState::Unloaded);
      }
    }
  }

  void OnUpdate(float deltaTime) {
    for (auto &script : Scripts) {
      if (script && script->IsEnabled() &&
          script->GetState() == ScriptState::Running) {
        auto start = std::chrono::high_resolution_clock::now();
        script->OnUpdate(deltaTime);
        auto end = std::chrono::high_resolution_clock::now();

        auto &profile = script->GetProfileData();
        profile.updateTime +=
            std::chrono::duration<double, std::micro>(end - start).count();
        profile.updateCalls++;
      }
    }
  }

  void OnFixedUpdate(float fixedDelta) {
    for (auto &script : Scripts) {
      if (script && script->IsEnabled() &&
          script->GetState() == ScriptState::Running) {
        auto start = std::chrono::high_resolution_clock::now();
        script->OnFixedUpdate(fixedDelta);
        auto end = std::chrono::high_resolution_clock::now();

        auto &profile = script->GetProfileData();
        profile.fixedUpdateTime +=
            std::chrono::duration<double, std::micro>(end - start).count();
        profile.fixedUpdateCalls++;
      }
    }
  }

  void OnLateUpdate(float deltaTime) {
    for (auto &script : Scripts) {
      if (script && script->IsEnabled() &&
          script->GetState() == ScriptState::Running) {
        auto start = std::chrono::high_resolution_clock::now();
        script->OnLateUpdate(deltaTime);
        auto end = std::chrono::high_resolution_clock::now();

        auto &profile = script->GetProfileData();
        profile.lateUpdateTime +=
            std::chrono::duration<double, std::micro>(end - start).count();
        profile.lateUpdateCalls++;
      }
    }
  }
};
} // namespace BeEngine
