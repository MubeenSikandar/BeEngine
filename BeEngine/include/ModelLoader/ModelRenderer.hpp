#pragma once

#include <BuiltInShaders/LightManager.hpp>
#include <Core.hpp>
#include <ModelLoader/Model.hpp>

namespace BeEngine {
/**
 * @brief Helper class to render loaded models
 */
class BE_API ModelRenderer {
public:
  /**
   * @brief Render a model with the given transform
   * @param model The model to render
   * @param transform World transform
   * @param viewProjection Camera's view-projection matrix
   * @param cameraPos Camera position (for lighting)
   * @param lightManager Light data
   */
  static void Render(const Ref<Model> &model, const Transform &transform,
                     const glm::mat4 &viewProjection,
                     const glm::vec3 &cameraPos,
                     const LightManager &lightManager);

  /**
   * @brief Render a model with a custom transform matrix
   */
  static void Render(const Ref<Model> &model, const glm::mat4 &modelMatrix,
                     const glm::mat4 &viewProjection,
                     const glm::vec3 &cameraPos,
                     const LightManager &lightManager);

  /**
   * @brief Render only specific meshes from a model
   */
  static void RenderMesh(const ModelMesh &mesh, const glm::mat4 &modelMatrix,
                         const glm::mat4 &viewProjection,
                         const glm::vec3 &cameraPos,
                         const LightManager &lightManager);

private:
  ModelRenderer() = default;
};
} // namespace BeEngine
