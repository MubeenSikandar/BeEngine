// ScripEngineManager.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
ScriptEngineManager::ScriptEngineManager() {
  BE_CORE_TRACE("[ScriptEngineManager] Created");
}

ScriptEngineManager::~ScriptEngineManager() {
  if (m_Initialized) {
    Shutdown();
  }
}

bool ScriptEngineManager::Initialize() {
  if (m_Initialized) {
    BE_CORE_WARN("[ScriptEngineManager] Already initialized");
    return true;
  }

  BE_CORE_INFO("[ScriptEngineManager] Initializing...");

  // Initialize all registered engines
  for (auto &engine : m_Engines) {
    if (!engine->Initialize()) {
      BE_CORE_ERROR("[ScriptEngineManager] Failed to initialize engine: {}",
                    engine->GetName());
      // Continue with other engines
    } else {
      BE_CORE_INFO("[ScriptEngineManager] Initialized engine: {} ({})",
                   engine->GetName(), engine->GetVersion());
    }
  }

  m_Initialized = true;
  BE_CORE_INFO("[ScriptEngineManager] Initialized with {} engines",
               m_Engines.size());

  return true;
}

void ScriptEngineManager::Shutdown() {
  if (!m_Initialized) {
    return;
  }

  BE_CORE_INFO("[ScriptEngineManager] Shutting down...");

  // Cleanup scene scripts
  if (m_ActiveScene) {
    CleanupSceneScripts();
  }

  // Shutdown all engines
  for (auto &engine : m_Engines) {
    engine->Shutdown();
  }

  m_Engines.clear();
  m_EngineMap.clear();
  m_Initialized = false;

  BE_CORE_INFO("[ScriptEngineManager] Shutdown complete");
}

void ScriptEngineManager::Start() {
  if (!m_ActiveScene || m_Paused) {
    return;
  }

  BE_CORE_TRACE("[ScriptEngineManager] Starting scene scripts...");

  m_ActiveScene->Each<ScriptComponent>(
      [](Entity entity, ScriptComponent &scriptComp) { scriptComp.OnStart(); });
}

void ScriptEngineManager::Pause() {
  m_Paused = true;
  BE_CORE_TRACE("[ScriptEngineManager] Paused");
}

void ScriptEngineManager::Resume() {
  m_Paused = false;
  BE_CORE_TRACE("[ScriptEngineManager] Resumed");
}

void ScriptEngineManager::SetSceneManager(SceneManager *sceneManager) {
  m_SceneManager = sceneManager;
}

void ScriptEngineManager::SetActiveScene(Scene *scene) {
  if (m_ActiveScene == scene) {
    return;
  }

  // Cleanup old scene
  if (m_ActiveScene) {
    CleanupSceneScripts();
  }

  m_ActiveScene = scene;

  // Initialize new scene
  if (m_ActiveScene) {
    InitializeSceneScripts();
  }
}

void ScriptEngineManager::RegisterEngine(Ref<ScriptEngine> engine) {
  if (!engine) {
    return;
  }

  ScriptLanguage lang = engine->GetLanguage();

  // Check for duplicate
  if (m_EngineMap.find(lang) != m_EngineMap.end()) {
    BE_CORE_WARN("[ScriptEngineManager] Replacing engine for language: {}",
                 ScriptLanguageToString(lang));
  }

  m_Engines.push_back(engine);
  m_EngineMap[lang] = engine.get();

  // Set error callback
  engine->SetErrorCallback([this](const ScriptError &error) {
    m_AllErrors.push_back(error);
    if (m_ErrorCallback) {
      m_ErrorCallback(error);
    }
  });

  BE_CORE_INFO("[ScriptEngineManager] Registered engine: {} for {}",
               engine->GetName(), ScriptLanguageToString(lang));
}

ScriptEngine *ScriptEngineManager::GetEngine(ScriptLanguage language) {
  auto it = m_EngineMap.find(language);
  return it != m_EngineMap.end() ? it->second : nullptr;
}

bool ScriptEngineManager::IsLanguageSupported(ScriptLanguage language) const {
  return m_EngineMap.find(language) != m_EngineMap.end();
}

bool ScriptEngineManager::LoadScript(const std::filesystem::path &filePath) {
  ScriptLanguage lang = DetectLanguage(filePath);

  if (lang == ScriptLanguage::None) {
    BE_CORE_ERROR("[ScriptEngineManager] Unknown script type: {}",
                  filePath.string());
    return false;
  }

  auto *engine = GetEngine(lang);
  if (!engine) {
    BE_CORE_ERROR("[ScriptEngineManager] No engine for language: {}",
                  ScriptLanguageToString(lang));
    return false;
  }

  return engine->LoadScript(filePath);
}

void ScriptEngineManager::LoadScriptsFromDirectory(
    const std::filesystem::path &directory, bool recursive) {
  if (!std::filesystem::exists(directory)) {
    BE_CORE_WARN("[ScriptEngineManager] Directory does not exist: {}",
                 directory.string());
    return;
  }

  if (recursive) {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(directory)) {
      if (entry.is_regular_file()) {
        LoadScript(entry.path());
      }
    }
  } else {
    for (const auto &entry : std::filesystem::directory_iterator(directory)) {
      if (entry.is_regular_file()) {
        LoadScript(entry.path());
      }
    }
  }
}

void ScriptEngineManager::UnloadScript(const std::string &scriptName) {
  for (auto &engine : m_Engines) {
    if (engine->IsScriptLoaded(scriptName)) {
      engine->UnloadScript(scriptName);
      break;
    }
  }
}

Ref<ScriptInstance>
ScriptEngineManager::CreateInstance(const std::string &scriptName,
                                    Entity entity) {
  // First try the registry (handles both native and scripted)
  auto instance = ScriptRegistry::CreateInstance(scriptName, entity);
  if (instance) {
    return instance;
  }

  // Try each engine
  for (auto &engine : m_Engines) {
    if (engine->IsScriptLoaded(scriptName)) {
      return engine->CreateInstance(scriptName, entity);
    }
  }

  BE_CORE_ERROR("[ScriptEngineManager] Script not found: {}", scriptName);
  return nullptr;
}

void ScriptEngineManager::InitializeSceneScripts() {
  if (!m_ActiveScene) {
    return;
  }

  BE_CORE_TRACE("[ScriptEngineManager] Initializing scene scripts...");

  m_ActiveScene->Each<ScriptComponent>(
      [this](Entity entity, ScriptComponent &scriptComp) {
        InitializeEntityScripts(entity);
      });
}

void ScriptEngineManager::CleanupSceneScripts() {
  if (!m_ActiveScene) {
    return;
  }

  BE_CORE_TRACE("[ScriptEngineManager] Cleaning up scene scripts...");

  m_ActiveScene->Each<ScriptComponent>(
      [this](Entity entity, ScriptComponent &scriptComp) {
        CleanupEntityScripts(entity);
      });
}

void ScriptEngineManager::InitializeEntityScripts(Entity entity) {
  auto *scriptComp = entity.TryGetComponent<ScriptComponent>();
  if (!scriptComp) {
    return;
  }

  // Create instances from script data
  for (const auto &data : scriptComp->ScriptDatas) {
    auto instance = CreateInstance(data.scriptName, entity);
    if (instance) {
      instance->SetEnabled(data.enabled);
      instance->GetFields().CopyValuesFrom(data.fields);
      scriptComp->AddInstance(std::move(instance));
    }
  }

  // Initialize all instances
  scriptComp->OnCreate();
}

void ScriptEngineManager::CleanupEntityScripts(Entity entity) {
  auto *scriptComp = entity.TryGetComponent<ScriptComponent>();
  if (!scriptComp) {
    return;
  }

  scriptComp->OnDestroy();
  scriptComp->Scripts.clear();
}

void ScriptEngineManager::OnUpdate(float deltaTime) {
  if (!m_ActiveScene || m_Paused) {
    return;
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  m_ActiveScene->Each<ScriptComponent>(
      [deltaTime](Entity entity, ScriptComponent &scriptComp) {
        scriptComp.OnUpdate(deltaTime);
      });

  auto endTime = std::chrono::high_resolution_clock::now();
  m_LastUpdateTime =
      std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

void ScriptEngineManager::OnFixedUpdate(float fixedDelta) {
  if (!m_ActiveScene || m_Paused) {
    return;
  }

  auto startTime = std::chrono::high_resolution_clock::now();

  m_ActiveScene->Each<ScriptComponent>(
      [fixedDelta](Entity entity, ScriptComponent &scriptComp) {
        scriptComp.OnFixedUpdate(fixedDelta);
      });

  auto endTime = std::chrono::high_resolution_clock::now();
  m_LastFixedUpdateTime =
      std::chrono::duration<double, std::milli>(endTime - startTime).count();
}

void ScriptEngineManager::OnLateUpdate(float deltaTime) {
  if (!m_ActiveScene || m_Paused) {
    return;
  }

  m_ActiveScene->Each<ScriptComponent>(
      [deltaTime](Entity entity, ScriptComponent &scriptComp) {
        scriptComp.OnLateUpdate(deltaTime);
      });
}

void ScriptEngineManager::OnEntityCreated(Entity entity) {
  if (entity.HasComponent<ScriptComponent>()) {
    InitializeEntityScripts(entity);
  }
}

void ScriptEngineManager::OnEntityDestroyed(Entity entity) {
  if (entity.HasComponent<ScriptComponent>()) {
    CleanupEntityScripts(entity);
  }
}

void ScriptEngineManager::SetHotReloadEnabled(bool enabled) {
  m_HotReloadEnabled = enabled;
  BE_CORE_INFO("[ScriptEngineManager] Hot reload {}",
               enabled ? "enabled" : "disabled");
}

void ScriptEngineManager::CheckForModifications() {
  if (!m_HotReloadEnabled) {
    return;
  }

  for (auto &engine : m_Engines) {
    if (engine->SupportsHotReload()) {
      engine->ReloadModifiedScripts();
    }
  }
}

bool ScriptEngineManager::ReloadScript(const std::string &scriptName) {
  for (auto &engine : m_Engines) {
    if (engine->IsScriptLoaded(scriptName) && engine->SupportsHotReload()) {
      if (engine->ReloadScript(scriptName)) {
        if (m_ReloadCallback) {
          m_ReloadCallback(scriptName);
        }
        return true;
      }
    }
  }
  return false;
}

void ScriptEngineManager::ReloadModifiedScripts() {
  for (auto &engine : m_Engines) {
    if (engine->SupportsHotReload()) {
      engine->ReloadModifiedScripts();
    }
  }
}

void ScriptEngineManager::SetErrorCallback(ScriptErrorCallback callback) {
  m_ErrorCallback = callback;

  // Propagate to all engines
  for (auto &engine : m_Engines) {
    engine->SetErrorCallback([this](const ScriptError &error) {
      m_AllErrors.push_back(error);
      if (m_ErrorCallback) {
        m_ErrorCallback(error);
      }
    });
  }
}

void ScriptEngineManager::ClearAllErrors() {
  m_AllErrors.clear();
  for (auto &engine : m_Engines) {
    engine->ClearErrors();
  }
}

void ScriptEngineManager::SetProfilingEnabled(bool enabled) {
  m_ProfilingEnabled = enabled;
  for (auto &engine : m_Engines) {
    engine->SetProfilingEnabled(enabled);
  }
}

std::vector<ScriptProfileData> ScriptEngineManager::GetProfileData() const {
  std::vector<ScriptProfileData> data;

  if (!m_ActiveScene) {
    return data;
  }

  const_cast<Scene *>(m_ActiveScene)
      ->Each<ScriptComponent>(
          [&data](Entity entity, ScriptComponent &scriptComp) {
            for (const auto &script : scriptComp.Scripts) {
              if (script) {
                data.push_back(script->GetProfileData());
              }
            }
          });

  return data;
}

void ScriptEngineManager::ResetProfileData() {
  if (!m_ActiveScene) {
    return;
  }

  m_ActiveScene->Each<ScriptComponent>(
      [](Entity entity, ScriptComponent &scriptComp) {
        for (auto &script : scriptComp.Scripts) {
          if (script) {
            script->GetProfileData().Reset();
          }
        }
      });
}

ScriptEngineManager::Stats ScriptEngineManager::GetStats() const {
  Stats stats;

  for (const auto &engine : m_Engines) {
    stats.totalScriptsLoaded += engine->GetLoadedScripts().size();
  }

  if (m_ActiveScene) {
    const_cast<Scene *>(m_ActiveScene)
        ->Each<ScriptComponent>(
            [&stats](Entity entity, ScriptComponent &scriptComp) {
              stats.totalInstances += scriptComp.Scripts.size();
              for (const auto &script : scriptComp.Scripts) {
                if (script && script->IsEnabled()) {
                  stats.activeInstances++;
                }
              }
            });
  }

  stats.errorCount = m_AllErrors.size();
  stats.lastUpdateTime = m_LastUpdateTime;
  stats.lastFixedUpdateTime = m_LastFixedUpdateTime;

  return stats;
}

std::vector<Ref<ScriptInstance>>
ScriptEngineManager::GetEntityScripts(Entity entity) const {
  std::vector<Ref<ScriptInstance>> result;
  if (auto *sc = entity.TryGetComponent<ScriptComponent>()) {
    result = sc->Scripts;
  }
  return result;
}

ScriptLanguage ScriptEngineManager::DetectLanguage(
    const std::filesystem::path &filePath) const {
  std::string ext = filePath.extension().string();

  if (ext == ".lua") {
    return ScriptLanguage::Lua;
  }
  if (ext == ".py") {
    return ScriptLanguage::Python;
  }
  if (ext == ".cs") {
    return ScriptLanguage::CSharp;
  }
  if (ext == ".rs") {
    return ScriptLanguage::Rust;
  }
  if (ext == ".wasm") {
    return ScriptLanguage::WASM;
  }
  if (ext == ".cpp" || ext == ".hpp") {
    return ScriptLanguage::Cpp;
  }
  return ScriptLanguage::None;
}

} // namespace BeEngine
