#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

void SceneRenderer::SetViewport(uint32_t width, uint32_t height) {
  m_ViewportWidth = width;
  m_ViewportHeight = height;
}

void SceneRenderer::RenderScene(Scene &scene, Entity overrideCamera) {
  // Determine which camera to use
  Entity cameraEntity =
      overrideCamera.IsValid() ? overrideCamera : scene.GetPrimaryCamera();

  if (!cameraEntity.IsValid() ||
      !cameraEntity.HasComponent<CameraComponent>()) {
    BE_CORE_WARN("SceneRenderer: No valid camera found in scene");
    return;
  }

  // Update cameras with current aspect ratio
  float aspectRatio = static_cast<float>(m_ViewportWidth) /
                      static_cast<float>(m_ViewportHeight);
  scene.UpdateCameras(aspectRatio);

  auto &camera = cameraEntity.GetComponent<CameraComponent>();
  auto &cameraTransform = cameraEntity.GetComponent<TransformComponent>();

  auto cameraPosition = glm::vec3(cameraTransform.WorldMatrix[3]);

  RenderScene(scene, camera.ViewProjectionMatrix, cameraPosition);
}

void SceneRenderer::RenderScene(Scene &scene, const glm::mat4 &viewProjection,
                                const glm::vec3 &cameraPosition) {
  // Reset stats at start of frame
  ResetStats();

  auto startTime = std::chrono::high_resolution_clock::now();

  // Collect lights from scene
  CollectLights(scene);

  // Render all renderable objects
  RenderMeshes(scene, viewProjection, cameraPosition);
  RenderModels(scene, viewProjection, cameraPosition);

  // Calculate render time
  auto endTime = std::chrono::high_resolution_clock::now();
  m_Stats.RenderTimeMs =
      std::chrono::duration<float, std::milli>(endTime - startTime).count();
}

void SceneRenderer::CollectLights(Scene &scene) {
  // Reset point lights (keep ambient and directional for manual control)
  m_LightManager.ClearPointLights();
  m_LightManager.ClearSpotLights();

  // Collect directional lights
  scene.Each<DirectionalLightComponent, TransformComponent>(
      [this](Entity entity, DirectionalLightComponent &light,
             TransformComponent &transform) {
        auto &dirLight = m_LightManager.GetDirectionalLight();
        dirLight.Enabled = true;
        dirLight.Color = light.Color;
        dirLight.Intensity = light.Intensity;

        // Direction is the forward vector of the transform (negative Z)
        glm::vec3 forward =
            -glm::normalize(glm::vec3(transform.WorldMatrix[2]));
        dirLight.Direction = forward;
      });

  // Collect point lights
  scene.Each<PointLightComponent, TransformComponent>(
      [this](Entity entity, PointLightComponent &light,
             TransformComponent &transform) {
        PointLight pointLight;
        pointLight.Enabled = true;
        pointLight.Position = glm::vec3(transform.WorldMatrix[3]);
        pointLight.Color = light.Color;
        pointLight.Intensity = light.Intensity;
        pointLight.Range = light.Range;

        m_LightManager.AddPointLight(pointLight);
      });

  // Collect spot lights
  scene.Each<SpotLightComponent, TransformComponent>(
      [this](Entity entity, SpotLightComponent &light,
             TransformComponent &transform) {
        SpotLight spotLight;
        spotLight.Enabled = true;
        spotLight.Position = glm::vec3(transform.WorldMatrix[3]);
        spotLight.Color = light.Color;
        spotLight.Intensity = light.Intensity;
        spotLight.Range = light.Range;
        spotLight.InnerCone = glm::cos(glm::radians(light.InnerConeAngle));
        spotLight.OuterCone = glm::cos(glm::radians(light.OuterConeAngle));

        // Direction is the forward vector
        glm::vec3 forward =
            -glm::normalize(glm::vec3(transform.WorldMatrix[2]));
        spotLight.Direction = forward;

        m_LightManager.AddSpotLight(spotLight);
      });
}

void SceneRenderer::RenderMeshes(Scene &scene, const glm::mat4 &viewProjection,
                                 const glm::vec3 &cameraPosition) {
  scene.Each<MeshRendererComponent, TransformComponent>(
      [this, &viewProjection,
       &cameraPosition](Entity entity, MeshRendererComponent &meshRenderer,
                        TransformComponent &transform) {
        if (!meshRenderer.Visible || !meshRenderer.MeshData ||
            !meshRenderer.MaterialData) {
          return;
        }

        // TODO: Frustum culling would go here
        // if (m_FrustumCullingEnabled && !IsInFrustum(transform,
        // viewProjection)) {
        //   return;
        // }

        // Bind material
        meshRenderer.MaterialData->Bind();
        auto shader = meshRenderer.MaterialData->GetShader();
        if (!shader) {
          return;
        }

        // Upload lighting
        m_LightManager.UploadToShader(shader, cameraPosition);

        // Upload transforms
        shader->SetMat4("u_ViewProjection", viewProjection);
        shader->SetMat4("u_Model", transform.WorldMatrix);

        glm::mat3 normalMatrix =
            glm::transpose(glm::inverse(glm::mat3(transform.WorldMatrix)));
        shader->SetMat3("u_NormalMatrix", normalMatrix);

        // Draw
        meshRenderer.MeshData->Bind();
        uint32_t indexCount = meshRenderer.MeshData->GetIndexCount();
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount),
                       GL_UNSIGNED_INT, nullptr);

        // Track stats
        m_Stats.DrawCalls++;
        m_Stats.TrianglesRendered += indexCount / 3;
        m_Stats.EntitiesRendered++;
      });
}

void SceneRenderer::RenderModels(Scene &scene, const glm::mat4 &viewProjection,
                                 const glm::vec3 &cameraPosition) {
  scene.Each<ModelRendererComponent, TransformComponent>(
      [this, &viewProjection,
       &cameraPosition](Entity entity, ModelRendererComponent &modelRenderer,
                        TransformComponent &transform) {
        if (!modelRenderer.Visible || !modelRenderer.ModelData) {
          return;
        }

        if (!modelRenderer.ModelData->IsValid()) {
          return;
        }

        // Use ModelRenderer to render the model
        ModelRenderer::Render(modelRenderer.ModelData, transform.WorldMatrix,
                              viewProjection, cameraPosition, m_LightManager);

        // Track stats (approximate - ModelRenderer might do multiple draw
        // calls)
        m_Stats.DrawCalls++;
        m_Stats.EntitiesRendered++;
        // Note: Triangle count from models would need ModelRenderer to report
        // back
      });
}

void SceneRenderer::ResetStats() {
  m_Stats.DrawCalls = 0;
  m_Stats.TrianglesRendered = 0;
  m_Stats.EntitiesRendered = 0;
  m_Stats.RenderTimeMs = 0.0F;
}

void SceneRenderer::SetFrustumCulling(bool enabled) {
  m_FrustumCullingEnabled = enabled;
}

void SceneRenderer::SetDebugMode(bool enabled) { m_DebugMode = enabled; }

void SceneRenderer::DrawDebugBounds(bool enabled) {
  // TODO: Implement when debug rendering system is ready
}

} // namespace BeEngine
