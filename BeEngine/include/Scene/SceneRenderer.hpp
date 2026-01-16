// SceneRenderer.hpp
#pragma once

#include <BuiltInShaders/LightManager.hpp>
#include <Core.hpp>
#include <Scene/Scene.hpp>

namespace BeEngine {

/**
 * @brief Renders a scene using its entities and components
 */
class BE_API SceneRenderer {
public:
  SceneRenderer() = default;
  ~SceneRenderer() = default;

  /**
   * @brief Render the scene
   * @param scene The scene to render
   * @param overrideCamera Optional camera to use instead of scene's primary
   * camera
   */
  void RenderScene(Scene &scene, Entity overrideCamera = Entity());

  /**
   * @brief Render the scene with explicit camera matrices
   */
  void RenderScene(Scene &scene, const glm::mat4 &viewProjection,
                   const glm::vec3 &cameraPosition);

  /**
   * @brief Set viewport dimensions (for camera aspect ratio)
   */
  void SetViewport(uint32_t width, uint32_t height);

  /**
   * @brief Get the light manager used by the renderer
   */
  NODISCARD LightManager &GetLightManager() { return m_LightManager; }
  NODISCARD const LightManager &GetLightManager() const {
    return m_LightManager;
  }

  // Render statistics
  struct RenderStats {
    uint32_t DrawCalls = 0;
    uint32_t TrianglesRendered = 0;
    uint32_t EntitiesRendered = 0;
    float RenderTimeMs = 0.0f;
  };

  NODISCARD const RenderStats &GetStats() const { return m_Stats; }
  void ResetStats();

  // Culling (for optimization)
  void SetFrustumCulling(bool enabled);

  // Debug rendering
  void SetDebugMode(bool enabled);
  void DrawDebugBounds(bool enabled);

private:
  /**
   * @brief Collect lights from the scene into the light manager
   */
  void CollectLights(Scene &scene);

  /**
   * @brief Render all mesh renderers
   */
  void RenderMeshes(Scene &scene, const glm::mat4 &viewProjection,
                    const glm::vec3 &cameraPosition);

  /**
   * @brief Render all model renderers
   */
  void RenderModels(Scene &scene, const glm::mat4 &viewProjection,
                    const glm::vec3 &cameraPosition);

  LightManager m_LightManager;
  uint32_t m_ViewportWidth = 1280;
  uint32_t m_ViewportHeight = 720;

  RenderStats m_Stats;
  bool m_FrustumCullingEnabled = true;
  bool m_DebugMode = false;
};
} // namespace BeEngine
