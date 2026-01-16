// ScriptRegistry.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Core/ScriptInstance.hpp>
#include <Scripting/Core/ScriptTypes.hpp>
#include <mutex>

namespace BeEngine {

/**
 * @brief Central registry for script factories
 *
 * Allows registration of native C++ scripts that can be instantiated by name.
 * Also integrates with language engines for scripted behaviors.
 */
class BE_API ScriptRegistry {
public:
  struct ScriptInfo {
    std::string name;
    std::string displayName;
    std::string category;
    std::string description;
    ScriptLanguage language{ScriptLanguage::Cpp};
    ScriptInstanceFactory factory;
    ScriptFieldMap defaultFields;
  };

  // ===== Registration =====

  /// Register a native C++ script class
  template <typename T> static void Register(const std::string &name) {
    static_assert(std::is_base_of_v<ScriptInstance, T>,
                  "T must derive from ScriptInstance");

    ScriptInfo info;
    info.name = name;
    info.displayName = name;
    info.language = ScriptLanguage::Cpp;
    info.factory =
        [name](Entity entity) -> Ref<ScriptInstance> { // Capture name
      auto instance = CreateRef<T>();
      instance->SetEntity(entity);
      instance->SetScriptName(name); // Use registered name
      instance->SetLanguage(ScriptLanguage::Cpp);
      return instance;
    };

    // Get default fields from a temporary instance
    auto temp = CreateRef<T>();
    info.defaultFields = temp->GetFields();

    RegisterInternal(name, std::move(info));
  }

  /// Register with metadata
  template <typename T>
  static void Register(const std::string &name, const std::string &displayName,
                       const std::string &category,
                       const std::string &description = "") {
    static_assert(std::is_base_of_v<ScriptInstance, T>,
                  "T must derive from ScriptInstance");

    ScriptInfo info;
    info.name = name;
    info.displayName = displayName;
    info.category = category;
    info.description = description;
    info.language = ScriptLanguage::Cpp;
    info.factory =
        [name](Entity entity) -> Ref<ScriptInstance> { // Capture name
      auto instance = CreateRef<T>();
      instance->SetEntity(entity);
      instance->SetScriptName(name);
      instance->SetLanguage(ScriptLanguage::Cpp);
      return instance;
    };

    auto temp = CreateRef<T>();
    info.defaultFields = temp->GetFields();

    RegisterInternal(name, std::move(info));
  }

  /// Register a script from a language engine
  static void RegisterFromEngine(const std::string &name,
                                 ScriptLanguage language,
                                 ScriptInstanceFactory factory,
                                 const ScriptFieldMap &defaultFields = {});

  // To Prevent Double Registry
  template <typename T>
  NODISCARD static bool RegisterOnce(const std::string &name) {
    if (IsRegistered(name)) {
      return false;
    }
    Register<T>(name);
    return true;
  }

  /// Unregister a script
  static void Unregister(const std::string &name);

  /// Unregister all scripts of a specific language
  static void UnregisterLanguage(ScriptLanguage language);

  /// Clear all registrations
  static void Clear();

  // ===== Query =====

  /// Check if a script is registered
  NODISCARD static bool IsRegistered(const std::string &name);

  /// Get script info
  NODISCARD static const ScriptInfo *GetScriptInfo(const std::string &name);

  /// Get all registered script names
  NODISCARD static std::vector<std::string> GetRegisteredScripts();

  /// Get scripts by category
  NODISCARD static std::vector<std::string>
  GetScriptsByCategory(const std::string &category);

  /// Get scripts by language
  NODISCARD static std::vector<std::string>
  GetScriptsByLanguage(ScriptLanguage language);

  /// Get all categories
  NODISCARD static std::vector<std::string> GetCategories();

  // ===== Instance Creation =====

  /// Create an instance of a registered script
  NODISCARD static Ref<ScriptInstance> CreateInstance(const std::string &name,
                                                      Entity entity);

  /// Create instance with initial field values
  NODISCARD static Ref<ScriptInstance>
  CreateInstance(const std::string &name, Entity entity,
                 const ScriptFieldMap &initialFields);

private:
  static void RegisterInternal(const std::string &name, ScriptInfo &&info);

  static std::unordered_map<std::string, ScriptInfo> &GetRegistry() {
    static std::unordered_map<std::string, ScriptInfo> s_Registry;
    return s_Registry;
  }

  static std::mutex &GetMutex() {
    static std::mutex s_Mutex;
    return s_Mutex;
  }
};

} // namespace BeEngine

// ============================================================================
// Registration Macro for C++ Scripts
// ============================================================================

#define BE_REGISTER_SCRIPT(ScriptClass)                                        \
  static struct ScriptClass##_Registrar {                                      \
    ScriptClass##_Registrar() {                                                \
      ::BeEngine::ScriptRegistry::Register<ScriptClass>(#ScriptClass);         \
    }                                                                          \
  } s_##ScriptClass##_Registrar

#define BE_REGISTER_SCRIPT_EX(ScriptClass, DisplayName, Category, Description) \
  static struct ScriptClass##_Registrar {                                      \
    ScriptClass##_Registrar() {                                                \
      ::BeEngine::ScriptRegistry::Register<ScriptClass>(                       \
          #ScriptClass, DisplayName, Category, Description);                   \
    }                                                                          \
  } s_##ScriptClass##_Registrar
