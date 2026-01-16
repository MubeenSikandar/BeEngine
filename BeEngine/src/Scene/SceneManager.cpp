#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

Scene &SceneManager::CreateScene(const std::string &name) {
  // Check if scene already exists
  if (m_Scenes.find(name) != m_Scenes.end()) {
    BE_CORE_WARN("Scene '{}' already exists, returning existing scene", name);
    return *m_Scenes[name];
  }

  auto scene = CreateScope<Scene>(name);
  Scene *scenePtr = scene.get();
  m_Scenes[name] = std::move(scene);

  BE_CORE_INFO("Created scene: '{}'", name);

  // Set as active if it's the first scene
  if (m_ActiveScene == nullptr) {
    SetActiveScene(scenePtr);
  }

  return *scenePtr;
}

bool SceneManager::SaveScene(const std::string &filepath) {
  if (m_ActiveScene == nullptr) {
    BE_CORE_ERROR("No active scene to save");
    return false;
  }
  return SaveScene(*m_ActiveScene, filepath);
}

bool SceneManager::SaveScene(Scene &scene, const std::string &filepath) {
  // Create parent directories if they don't exist
  std::filesystem::path path(filepath);
  if (path.has_parent_path()) {
    std::filesystem::create_directories(path.parent_path());
  }

  SceneSerializer serializer(scene);
  if (!serializer.Serialize(filepath)) {
    BE_CORE_ERROR("Failed to save scene '{}' to: {}", scene.GetName(),
                  filepath);
    return false;
  }

  BE_CORE_INFO("Saved scene '{}' to: {}", scene.GetName(), filepath);
  return true;
}

bool SceneManager::SetActiveScene(const std::string &name) {
  auto it = m_Scenes.find(name);
  if (it == m_Scenes.end()) {
    BE_CORE_WARN("Scene '{}' not found", name);
    return false;
  }

  SetActiveScene(it->second.get());
  return true;
}

void SceneManager::SetActiveScene(Scene *scene) {
  if (m_ActiveScene == scene) {
    return;
  }

  Scene *oldScene = m_ActiveScene;
  m_ActiveScene = scene;

  // Notify callbacks
  for (auto &callback : m_SceneChangeCallbacks) {
    callback(oldScene, m_ActiveScene);
  }

  if (scene != nullptr) {
    BE_CORE_INFO("Active scene changed to: '{}'", scene->GetName());
  } else {
    BE_CORE_INFO("Active scene cleared");
  }
}

Scene *SceneManager::GetScene(const std::string &name) {
  auto it = m_Scenes.find(name);
  if (it != m_Scenes.end()) {
    return it->second.get();
  }
  return nullptr;
}

bool SceneManager::HasScene(const std::string &name) const {
  return m_Scenes.find(name) != m_Scenes.end();
}

void SceneManager::RemoveScene(const std::string &name) {
  auto it = m_Scenes.find(name);
  if (it == m_Scenes.end()) {
    return;
  }

  // Clear active if this is the active scene
  if (m_ActiveScene == it->second.get()) {
    SetActiveScene(nullptr);
  }

  m_Scenes.erase(it);
  BE_CORE_INFO("Removed scene: '{}'", name);
}

void SceneManager::Clear() {
  SetActiveScene(nullptr);
  m_Scenes.clear();
  BE_CORE_INFO("Cleared all scenes");
}

std::vector<std::string> SceneManager::GetSceneNames() const {
  std::vector<std::string> names;
  names.reserve(m_Scenes.size());
  for (const auto &[name, scene] : m_Scenes) {
    names.push_back(name);
  }
  return names;
}

Scene *SceneManager::LoadScene(const std::string &filepath) {
  auto scene = CreateScope<Scene>();
  SceneSerializer serializer(*scene);

  if (!serializer.Deserialize(filepath)) {
    BE_CORE_ERROR("Failed to load scene from: {}", filepath);
    return nullptr;
  }

  const std::string &name = scene->GetName();

  // Remove existing scene with same name if any
  if (m_Scenes.find(name) != m_Scenes.end()) {
    BE_CORE_WARN("Replacing existing scene: '{}'", name);
    RemoveScene(name);
  }

  Scene *scenePtr = scene.get();
  m_Scenes[name] = std::move(scene);

  BE_CORE_INFO("Loaded scene: '{}' from {}", name, filepath);

  // Set as active
  SetActiveScene(scenePtr);

  return scenePtr;
}

void SceneManager::LoadSceneAsync(const std::string &filepath,
                                  std::function<void(Scene *)> onComplete) {
  m_IsLoading = true;

  std::thread([this, filepath, onComplete]() {
    auto scene = CreateScope<Scene>();
    SceneSerializer serializer(*scene);

    if (serializer.Deserialize(filepath)) {
      std::lock_guard<std::mutex> lock(m_PendingMutex);
      m_PendingLoads.push_back({std::move(scene), onComplete});
    } else {
      BE_CORE_ERROR("Failed to async load scene from: {}", filepath);
      if (onComplete) {
        onComplete(nullptr);
      }
    }

    m_IsLoading = false;
  }).detach();
}

void SceneManager::ProcessPendingLoads() {
  std::lock_guard<std::mutex> lock(m_PendingMutex);

  for (auto &pending : m_PendingLoads) {
    const std::string &name = pending.LoadedScene->GetName();
    Scene *scenePtr = pending.LoadedScene.get();
    m_Scenes[name] = std::move(pending.LoadedScene);

    if (pending.Callback) {
      pending.Callback(scenePtr);
    }
  }

  m_PendingLoads.clear();
}

Scene *SceneManager::LoadSceneAdditive(const std::string &filepath) {
  auto scene = CreateScope<Scene>();
  SceneSerializer serializer(*scene);

  if (!serializer.Deserialize(filepath)) {
    BE_CORE_ERROR("Failed to load scene additively from: {}", filepath);
    return nullptr;
  }

  const std::string &name = scene->GetName();

  // If scene with same name exists, generate unique name
  std::string uniqueName = name;
  int counter = 1;
  while (m_Scenes.find(uniqueName) != m_Scenes.end()) {
    uniqueName = name + "_" + std::to_string(counter++);
  }

  if (uniqueName != name) {
    scene->SetName(uniqueName);
    BE_CORE_WARN("Scene '{}' already exists, loaded as '{}'", name, uniqueName);
  }

  Scene *scenePtr = scene.get();
  m_Scenes[uniqueName] = std::move(scene);

  BE_CORE_INFO("Loaded scene additively: '{}' from {}", uniqueName, filepath);

  // Note: Does NOT change active scene
  return scenePtr;
}

} // namespace BeEngine
