// ScriptEngine.hpp
#pragma once

#include <Core.hpp>
#include <Logs/Log.hpp>
#include <Scene/ECS/Entity.hpp>
#include <Scripting/Core/ScriptField.hpp>
#include <Scripting/Core/ScriptTypes.hpp>
#include <filesystem>
#include <unordered_map>
#include <vector>

namespace BeEngine {
/**
 * @brief Abstract base class for language-specific script engines
 *
 * Each language (Lua, Python, C#, Rust) implements this interface.
 * The engine is responsible for:
 * - Loading/unloading scripts
 * - Creating script instances
 * - Executing script code
 * - Hot reloading
 */

struct SandboxConfig {
  bool enabled{true};
  size_t maxMemoryBytes{64 * 1024 * 1024}; // 64MB default
  double maxExecutionTimeMs{16.0};         // One frame at 60fps
  bool allowFileIO{false};
  bool allowNetwork{false};
  std::vector<std::string> allowedModules;
};

class BE_API ScriptEngine {
public:
  ScriptEngine(ScriptLanguage language, const std::string &name)
      : m_Language(language), m_Name(name) {}

  virtual ~ScriptEngine() = default;

  // Non-copyable
  ScriptEngine(const ScriptEngine &) = delete;
  ScriptEngine &operator=(const ScriptEngine &) = delete;

  ScriptEngine(ScriptEngine &&) = default;
  ScriptEngine &operator=(ScriptEngine &&) = default;

  // ===== Lifecycle =====

  /// Initialize the script engine
  virtual bool Initialize() = 0;

  /// Shutdown the script engine
  virtual void Shutdown() = 0;

  /// Check if engine is initialized
  NODISCARD virtual bool IsInitialized() const { return m_Initialized; }

  // ===== Script Loading =====

  /// Load a script from file
  virtual bool LoadScript(const std::filesystem::path &filePath) = 0;

  /// Load a script from string
  virtual bool LoadScriptFromString(const std::string &name,
                                    const std::string &code) = 0;

  /// Unload a specific script
  virtual void UnloadScript(const std::string &scriptName) = 0;

  /// Unload all scripts
  virtual void UnloadAllScripts() = 0;

  /// Check if a script is loaded
  NODISCARD virtual bool
  IsScriptLoaded(const std::string &scriptName) const = 0;

  /// Get list of loaded scripts
  NODISCARD virtual std::vector<std::string> GetLoadedScripts() const = 0;

  NODISCARD virtual bool ValidateScript(const std::filesystem::path &filePath,
                                        std::vector<ScriptError> &outErrors) {
    return true; // Default: assume valid
  }

  /// Compile script (for languages that support it)
  virtual bool CompileScript(const std::filesystem::path &filePath) {
    return LoadScript(filePath); // Default: just load
  }

  // ===== Instance Management =====

  /// Create a new script instance for an entity
  virtual Ref<ScriptInstance> CreateInstance(const std::string &scriptName,
                                             Entity entity) = 0;

  /// Destroy a script instance
  virtual void DestroyInstance(const Ref<ScriptInstance> &instance) = 0;

  // ===== Script Discovery =====

  /// Get metadata for a script
  NODISCARD virtual ScriptMetadata
  GetScriptMetadata(const std::string &scriptName) const = 0;

  /// Get all available scripts (for editor)
  NODISCARD virtual std::vector<ScriptMetadata> GetAvailableScripts() const = 0;

  /// Get exposed fields for a script class
  NODISCARD virtual ScriptFieldMap
  GetScriptFields(const std::string &scriptName) const = 0;

  // ===== Hot Reload =====

  /// Check if hot reload is supported
  NODISCARD virtual bool SupportsHotReload() const { return false; }

  /// Reload a specific script
  virtual bool ReloadScript(const std::string &scriptName) { return false; }

  /// Reload all modified scripts
  virtual void ReloadModifiedScripts() {}

  // ===== Execution =====

  /// Execute a string of code (for console/debugging)
  virtual bool ExecuteString(const std::string &code, std::string &output) {
    output = "Not supported";
    return false;
  }

  // ===== Error Handling =====

  void SetErrorCallback(ScriptErrorCallback callback) {
    m_ErrorCallback = std::move(callback);
  }

  NODISCARD const std::vector<ScriptError> &GetErrors() const {
    return m_Errors;
  }
  void ClearErrors() { m_Errors.clear(); }

  // ===== Profiling =====

  void SetProfilingEnabled(bool enabled) { m_ProfilingEnabled = enabled; }
  NODISCARD bool IsProfilingEnabled() const { return m_ProfilingEnabled; }

  // ===== Sandboxing =====

  void SetSandboxEnabled(bool enabled) { m_SandboxEnabled = enabled; }
  NODISCARD bool IsSandboxEnabled() const { return m_SandboxEnabled; }

  void SetSandboxConfig(const SandboxConfig &config) {
    m_SandboxConfig = config;
  }
  NODISCARD const SandboxConfig &GetSandboxConfig() const {
    return m_SandboxConfig;
  }

  // ===== Accessors =====

  NODISCARD ScriptLanguage GetLanguage() const { return m_Language; }
  NODISCARD const std::string &GetName() const { return m_Name; }
  NODISCARD const std::string &GetVersion() const { return m_Version; }

  // ===== Search Paths =====

  void AddScriptPath(const std::filesystem::path &path) {
    m_ScriptPaths.push_back(path);
  }

  NODISCARD const std::vector<std::filesystem::path> &GetScriptPaths() const {
    return m_ScriptPaths;
  }

protected:
  void ReportError(const ScriptError &error) {
    m_Errors.push_back(error);
    if (m_ErrorCallback) {
      m_ErrorCallback(error);
    }
    BE_CORE_ERROR("[{}] {}", m_Name, error.ToString());
  }

  void ClearAndReportError(const std::string &message,
                           const std::string &file = "", int line = -1) {
    ScriptError error;
    error.message = message;
    error.file = file;
    error.line = line;
    error.language = m_Language;
    ReportError(error);
  }

  ScriptLanguage m_Language{ScriptLanguage::None};
  std::string m_Name;
  std::string m_Version;
  bool m_Initialized{false};
  bool m_ProfilingEnabled{false};
  bool m_SandboxEnabled{true};

  SandboxConfig m_SandboxConfig;

  std::vector<std::filesystem::path> m_ScriptPaths;
  std::vector<ScriptError> m_Errors;
  ScriptErrorCallback m_ErrorCallback;

  // Loaded scripts metadata cache
  std::unordered_map<std::string, ScriptMetadata> m_LoadedScripts;
};
} // namespace BeEngine
