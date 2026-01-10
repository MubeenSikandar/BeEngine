// BeEngine/ModelLoader/Model.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

Ref<Model> Model::Load(const std::string &filepath) {
  return GLTFLoader::Load(filepath);
}

uint32_t Model::GetTotalVertexCount() const {
  uint32_t total = 0;
  for (const auto &mesh : m_Meshes) {
    if (mesh.MeshData) {
      total += mesh.MeshData->GetVertexCount();
    }
  }
  return total;
}

uint32_t Model::GetTotalTriangleCount() const {
  uint32_t total = 0;
  for (const auto &mesh : m_Meshes) {
    if (mesh.MeshData) {
      total += mesh.MeshData->GetTriangleCount();
    }
  }
  return total;
}

std::vector<int32_t> Model::GetRootNodes() const {
  std::vector<int32_t> roots;
  for (size_t i = 0; i < m_Nodes.size(); i++) {
    if (m_Nodes[i].ParentIndex == -1) {
      roots.push_back(static_cast<int32_t>(i));
    }
  }
  return roots;
}

void Model::UpdateTransforms() {
  auto roots = GetRootNodes();
  for (int32_t rootIndex : roots) {
    UpdateNodeTransform(rootIndex, glm::mat4(1.0F));
  }
}

void Model::UpdateNodeTransform(int32_t nodeIndex,
                                const glm::mat4 &parentTransform) {
  if (nodeIndex < 0 || nodeIndex >= static_cast<int32_t>(m_Nodes.size())) {
    return;
  }

  auto &node = m_Nodes[nodeIndex];
  node.GlobalTransform = parentTransform * node.LocalTransform.GetWorldMatrix();

  for (int32_t childIndex : node.ChildrenIndices) {
    UpdateNodeTransform(childIndex, node.GlobalTransform);
  }
}

void Model::CalculateBoundingBox() {
  m_BoundingBox = AABB();

  for (const auto &mesh : m_Meshes) {
    if (mesh.MeshData) {
      m_BoundingBox.Expand(mesh.MeshData->GetBoundingBox());
    }
  }
}

} // namespace BeEngine
