// ModelLoader/Model.hpp
#pragma once

#include <BuiltInShaders/PBRMaterial.hpp>
#include <Core.hpp>
#include <Math/Transform.hpp>
#include <MeshSystem/Mesh.hpp>

namespace BeEngine {

class GLTFLoader;

/**
 * @brief A single mesh within a model, with its associated material
 */
struct ModelMesh {
  Ref<Mesh> MeshData;
  Ref<PBRMaterial> Material;
  std::string Name;
  int32_t MaterialIndex = -1;
};

/**
 * @brief A node in the model's scene hierarchy
 */
struct ModelNode {
  std::string Name;
  Transform LocalTransform;
  glm::mat4 GlobalTransform = glm::mat4(1.0F);

  int32_t MeshIndex = -1; // -1 means no mesh
  int32_t ParentIndex = -1;
  std::vector<int32_t> ChildrenIndices;
};

/**
 * @brief Loaded 3D model containing meshes, materials, and scene hierarchy
 *
 * Supports:
 * - Multiple meshes with different materials
 * - Scene hierarchy (parent-child transforms)
 * - PBR materials with textures
 * - Embedded and external textures
 */
class BE_API Model {
public:
  Model() = default;
  ~Model() = default;

  BE_DISALLOW_COPY(Model)
  Model(Model &&) = default;
  Model &operator=(Model &&) = default;

  /**
   * @brief Load a model from file
   * @param filepath Path to .gltf or .glb file
   * @return Loaded model, or nullptr on failure
   */
  NODISCARD static Ref<Model> Load(const std::string &filepath);

  /**
   * @brief Load a model asynchronously (future implementation)
   */
  // static std::future<Ref<Model>> LoadAsync(const std::string& filepath);

  // ===== Accessors =====
  NODISCARD const std::vector<ModelMesh> &GetMeshes() const { return m_Meshes; }
  NODISCARD std::vector<ModelMesh> &GetMeshes() { return m_Meshes; }

  NODISCARD const std::vector<ModelNode> &GetNodes() const { return m_Nodes; }
  NODISCARD std::vector<ModelNode> &GetNodes() { return m_Nodes; }

  NODISCARD const std::vector<Ref<PBRMaterial>> &GetMaterials() const {
    return m_Materials;
  }

  NODISCARD const std::vector<Ref<Texture2D>> &GetTextures() const {
    return m_Textures;
  }

  NODISCARD const std::string &GetName() const { return m_Name; }
  NODISCARD const std::string &GetFilepath() const { return m_Filepath; }

  NODISCARD const AABB &GetBoundingBox() const { return m_BoundingBox; }

  NODISCARD uint32_t GetMeshCount() const {
    return static_cast<uint32_t>(m_Meshes.size());
  }
  NODISCARD uint32_t GetNodeCount() const {
    return static_cast<uint32_t>(m_Nodes.size());
  }
  NODISCARD uint32_t GetMaterialCount() const {
    return static_cast<uint32_t>(m_Materials.size());
  }
  NODISCARD uint32_t GetTextureCount() const {
    return static_cast<uint32_t>(m_Textures.size());
  }

  /**
   * @brief Get total vertex count across all meshes
   */
  NODISCARD uint32_t GetTotalVertexCount() const;

  /**
   * @brief Get total triangle count across all meshes
   */
  NODISCARD uint32_t GetTotalTriangleCount() const;

  /**
   * @brief Get root node indices (nodes with no parent)
   */
  NODISCARD std::vector<int32_t> GetRootNodes() const;

  /**
   * @brief Update global transforms based on hierarchy
   */
  void UpdateTransforms();

  /**
   * @brief Check if model loaded successfully
   */
  NODISCARD bool IsValid() const { return !m_Meshes.empty(); }

private:
  friend class GLTFLoader;

  std::string m_Name = "Unnamed Model";
  std::string m_Filepath;

  std::vector<ModelMesh> m_Meshes;
  std::vector<ModelNode> m_Nodes;
  std::vector<Ref<PBRMaterial>> m_Materials;
  std::vector<Ref<Texture2D>> m_Textures;

  AABB m_BoundingBox;

  void CalculateBoundingBox();
  void UpdateNodeTransform(int32_t nodeIndex, const glm::mat4 &parentTransform);
};
} // namespace BeEngine
