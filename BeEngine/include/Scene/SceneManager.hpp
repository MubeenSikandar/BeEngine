// SceneManager.hpp
#pragma once

#include <Core.hpp>
#include <Scene/Scene.hpp>

namespace BeEngine {
/**
 * @brief Manages multiple scenes and scene transitions
 */
class BE_API SceneManager {
public:
  SceneManager() = default;
  ~SceneManager() = default;

  // Non-copyable
  SceneManager(const SceneManager &) = delete;
  SceneManager &operator=(const SceneManager &) = delete;

  // Movable (explicit implementation to handle std::function vector)
  SceneManager(SceneManager &&other) noexcept
      : m_Scenes(std::move(other.m_Scenes)), m_ActiveScene(other.m_ActiveScene),
        m_SceneChangeCallbacks(std::move(other.m_SceneChangeCallbacks)) {
    other.m_ActiveScene = nullptr;
  }

  SceneManager &operator=(SceneManager &&other) noexcept {
    if (this != &other) {
      m_Scenes = std::move(other.m_Scenes);
      m_ActiveScene = other.m_ActiveScene;
      m_SceneChangeCallbacks = std::move(other.m_SceneChangeCallbacks);
      other.m_ActiveScene = nullptr;
    }
    return *this;
  }

  /**
   * @brief Create a new empty scene
   * @param name Scene name
   * @return Reference to the created scene
   */
  Scene &CreateScene(const std::string &name);

  /**
   * @brief Load a scene from file
   * @param filepath Path to scene file
   * @return Pointer to loaded scene (nullptr on failure)
   */
  Scene *LoadScene(const std::string &filepath);

  /**
   * @brief Save the active scene to file
   * @param filepath Path to save to
   * @return True on success
   */
  bool SaveScene(const std::string &filepath);

  /**
   * @brief Save a specific scene to file
   */
  bool SaveScene(Scene &scene, const std::string &filepath);

  /**
   * @brief Set the active scene
   * @param name Scene name
   * @return True if scene was found and set
   */
  bool SetActiveScene(const std::string &name);

  /**
   * @brief Set the active scene by pointer
   */
  void SetActiveScene(Scene *scene);

  /**
   * @brief Get the currently active scene
   */
  NODISCARD Scene *GetActiveScene() { return m_ActiveScene; }
  NODISCARD const Scene *GetActiveScene() const { return m_ActiveScene; }

  /**
   * @brief Get a scene by name
   */
  NODISCARD Scene *GetScene(const std::string &name);

  /**
   * @brief Check if a scene exists
   */
  NODISCARD bool HasScene(const std::string &name) const;

  /**
   * @brief Remove a scene
   */
  void RemoveScene(const std::string &name);

  /**
   * @brief Remove all scenes
   */
  void Clear();

  /**
   * @brief Get all scene names
   */
  NODISCARD std::vector<std::string> GetSceneNames() const;

  /**
   * @brief Get the number of scenes
   */
  NODISCARD size_t GetSceneCount() const { return m_Scenes.size(); }

  // ===== Callbacks =====

  using SceneChangeCallback =
      std::function<void(Scene *oldScene, Scene *newScene)>;

  /**
   * @brief Register callback for scene changes
   */
  void OnSceneChange(SceneChangeCallback callback) {
    m_SceneChangeCallbacks.push_back(std::move(callback));
  }

private:
  std::unordered_map<std::string, Scope<Scene>> m_Scenes;
  Scene *m_ActiveScene = nullptr;
  std::vector<SceneChangeCallback> m_SceneChangeCallbacks;
};
} // namespace BeEngine
