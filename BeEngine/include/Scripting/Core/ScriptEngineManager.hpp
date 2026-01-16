// ScriptEngineManager.hpp
#pragma once

#include <Core.hpp>
#include <Scene/SceneManager.hpp>
#include <Scripting/Core/ScriptEngine.hpp>
#include <map>

namespace BeEngine {

/**
 * @brief Central manager for all script engines and script execution
 *
 * Responsibilities:
 * - Initialize/shutdown language engines
 * - Route script creation to appropriate engine
 * - Execute script lifecycle methods
 * - Handle hot reloading
 * - Profiling and error collection
 */
class BE_API ScriptEngineManager {
public:
  ScriptEngineManager();
  ~ScriptEngineManager();

  // Non-copyable
  ScriptEngineManager(const ScriptEngineManager &) = delete;
  ScriptEngineManager &operator=(const ScriptEngineManager &) = delete;

  ScriptEngineManager(ScriptEngineManager &&) = delete;
  ScriptEngineManager &operator=(ScriptEngineManager &&) = delete;

  // ===== Lifecycle =====

  /// Initialize all registered engines
  NODISCARD bool Initialize();

  /// Shutdown all engines
  void Shutdown();

  /// Set the scene manager (for accessing entities)
  void SetSceneManager(SceneManager *sceneManager);

  /// Set the active scene
  void SetActiveScene(Scene *scene);

  NODISCARD bool IsPaused() const { return m_Paused; }

  // ===== Engine Management =====

  /// Register a language engine
  void RegisterEngine(Ref<ScriptEngine> engine);

  /// Get engine by language
  NODISCARD ScriptEngine *GetEngine(ScriptLanguage language);

  /// Get all registered engines
  NODISCARD const std::vector<Ref<ScriptEngine>> &GetEngines() const {
    return m_Engines;
  }

  /// Check if a language is supported
  NODISCARD bool IsLanguageSupported(ScriptLanguage language) const;

  // ===== Script Loading =====

  /// Load a script file (auto-detects language)
  NODISCARD bool LoadScript(const std::filesystem::path &filePath);

  /// Load all scripts from a directory
  void LoadScriptsFromDirectory(const std::filesystem::path &directory,
                                bool recursive = true);

  /// Unload a script
  void UnloadScript(const std::string &scriptName);

  // ===== Instance Management =====

  /// Create a script instance for an entity
  NODISCARD Ref<ScriptInstance> CreateInstance(const std::string &scriptName,
                                               Entity entity);

  /// Initialize all script components in the active scene
  void InitializeSceneScripts();

  /// Cleanup all script components in the active scene
  void CleanupSceneScripts();

  NODISCARD Scene *GetActiveScene() const { return m_ActiveScene; }

  NODISCARD std::vector<Ref<ScriptInstance>>
  GetEntityScripts(Entity entity) const;

  // ===== Script Execution =====

  /// Update all scripts (call every frame)
  void OnUpdate(float deltaTime);

  /// Fixed update all scripts (call from physics loop)
  void OnFixedUpdate(float fixedDelta);

  /// Late update all scripts
  void OnLateUpdate(float deltaTime);

  /// Called when an entity is created
  void OnEntityCreated(Entity entity);

  /// Called when an entity is destroyed
  void OnEntityDestroyed(Entity entity);

  void Start();

  void Pause();

  void Resume();

  // ===== Hot Reload =====

  /// Enable/disable hot reload
  void SetHotReloadEnabled(bool enabled);
  NODISCARD bool IsHotReloadEnabled() const { return m_HotReloadEnabled; }

  /// Manually trigger hot reload check
  void CheckForModifications();

  /// Reload a specific script
  NODISCARD bool ReloadScript(const std::string &scriptName);

  /// Reload all modified scripts
  void ReloadModifiedScripts();

  /// Set callback for reload events
  void SetReloadCallback(ScriptReloadCallback callback) {
    m_ReloadCallback = std::move(callback);
  }

  // ===== Error Handling =====

  void SetErrorCallback(ScriptErrorCallback callback);
  NODISCARD const std::vector<ScriptError> &GetAllErrors() const {
    return m_AllErrors;
  }
  void ClearAllErrors();

  // ===== Profiling =====

  void SetProfilingEnabled(bool enabled);
  NODISCARD bool IsProfilingEnabled() const { return m_ProfilingEnabled; }

  /// Get aggregated profile data
  NODISCARD std::vector<ScriptProfileData> GetProfileData() const;

  /// Reset all profile data
  void ResetProfileData();

  // ===== Statistics =====

  struct Stats {
    size_t totalScriptsLoaded{0};
    size_t totalInstances{0};
    size_t activeInstances{0};
    size_t errorCount{0};
    double lastUpdateTime{0.0};
    double lastFixedUpdateTime{0.0};
  };

  NODISCARD Stats GetStats() const;

private:
  void InitializeEntityScripts(Entity entity);
  void CleanupEntityScripts(Entity entity);
  NODISCARD ScriptLanguage
  DetectLanguage(const std::filesystem::path &filePath) const;

  std::vector<Ref<ScriptEngine>> m_Engines;
  std::map<ScriptLanguage, ScriptEngine *> m_EngineMap;

  SceneManager *m_SceneManager{nullptr};
  Scene *m_ActiveScene{nullptr};

  bool m_Initialized{false};
  bool m_HotReloadEnabled{true};
  bool m_ProfilingEnabled{false};
  bool m_Paused{false};

  std::vector<ScriptError> m_AllErrors;
  ScriptErrorCallback m_ErrorCallback;
  ScriptReloadCallback m_ReloadCallback;

  // Timing for profiling
  double m_LastUpdateTime{0.0};
  double m_LastFixedUpdateTime{0.0};
};
} // namespace BeEngine
