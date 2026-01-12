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
    if (m_ActiveScene == m_Scenes[name].get()) {
      m_ActiveScene = nullptr;
    }
  }

  Scene *scenePtr = scene.get();
  m_Scenes[name] = std::move(scene);

  BE_CORE_INFO("Loaded scene: '{}' from {}", name, filepath);

  // Set as active if no active scene
  if (m_ActiveScene == nullptr) {
    SetActiveScene(scenePtr);
  }

  return scenePtr;
}

} // namespace BeEngine
