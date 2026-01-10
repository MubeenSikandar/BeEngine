// BeEngine/ModelLoader/ModelRenderer.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

void ModelRenderer::Render(const Ref<Model> &model, const Transform &transform,
                           const glm::mat4 &viewProjection,
                           const glm::vec3 &cameraPos,
                           const LightManager &lightManager) {
  Render(model, transform.GetWorldMatrix(), viewProjection, cameraPos,
         lightManager);
}

void ModelRenderer::Render(const Ref<Model> &model,
                           const glm::mat4 &modelMatrix,
                           const glm::mat4 &viewProjection,
                           const glm::vec3 &cameraPos,
                           const LightManager &lightManager) {
  if (!model || !model->IsValid()) {
    return;
  }

  const auto &nodes = model->GetNodes();
  const auto &meshes = model->GetMeshes();

  // Render each node with a mesh
  for (const auto &node : nodes) {
    if (node.MeshIndex < 0) {
      continue;
    }

    // Combine model transform with node's global transform
    glm::mat4 finalTransform = modelMatrix * node.GlobalTransform;

    // Find all meshes associated with this node
    // (In our loader, we store the first mesh index in node.MeshIndex)
    for (size_t i = node.MeshIndex; i < meshes.size(); i++) {
      const auto &mesh = meshes[i];

      // Check if this mesh belongs to this node (simple heuristic)
      // A more robust solution would store mesh-node relationships properly
      RenderMesh(mesh, finalTransform, viewProjection, cameraPos, lightManager);

      // Only render the first mesh per node in this simple version
      break;
    }
  }

  // Also render any meshes not associated with nodes
  if (nodes.empty()) {
    for (const auto &mesh : meshes) {
      RenderMesh(mesh, modelMatrix, viewProjection, cameraPos, lightManager);
    }
  }
}

void ModelRenderer::RenderMesh(const ModelMesh &mesh,
                               const glm::mat4 &modelMatrix,
                               const glm::mat4 &viewProjection,
                               const glm::vec3 &cameraPos,
                               const LightManager &lightManager) {
  if (!mesh.MeshData || !mesh.Material) {
    return;
  }

  // Bind material
  mesh.Material->Bind();

  auto shader = mesh.Material->GetShader();
  if (!shader) {
    return;
  }

  // Upload lighting
  lightManager.UploadToShader(shader, cameraPos);

  // Upload transforms
  shader->SetMat4("u_ViewProjection", viewProjection);
  shader->SetMat4("u_Model", modelMatrix);

  glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
  shader->SetMat3("u_NormalMatrix", normalMatrix);

  // Draw
  mesh.MeshData->Bind();
  glDrawElements(GL_TRIANGLES,
                 static_cast<GLsizei>(mesh.MeshData->GetIndexCount()),
                 GL_UNSIGNED_INT, nullptr);
}

} // namespace BeEngine
