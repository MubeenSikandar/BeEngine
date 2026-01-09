#pragma once

#include "Core.hpp"
#include "MeshSystem/Vertex.hpp"
#include "Renderer/VertexArray.hpp"
#include <glm/glm.hpp>
#include <string>
namespace BeEngine {

/**
 * @brief Axis-Aligned Bounding Box
 */
struct AABB {
  glm::vec3 Min = glm::vec3(std::numeric_limits<float>::max());
  glm::vec3 Max = glm::vec3(std::numeric_limits<float>::lowest());

  NODISCARD BE_FORCE_INLINE glm::vec3 GetCenter() const {
    return (Min + Max) * 0.5f;
  }
  NODISCARD BE_FORCE_INLINE glm::vec3 GetSize() const { return Max - Min; }
  NODISCARD BE_FORCE_INLINE glm::vec3 GetExtents() const {
    return GetSize() * 0.5f;
  }

  void Expand(const glm::vec3 &point) {
    Min = glm::min(Min, point);
    Max = glm::max(Max, point);
  }

  void Expand(const AABB &other) {
    Min = glm::min(Min, other.Min);
    Max = glm::max(Max, other.Max);
  }

  NODISCARD bool Contains(const glm::vec3 &point) const {
    return point.x >= Min.x && point.x <= Max.x && point.y >= Min.y &&
           point.y <= Max.y && point.z >= Min.z && point.z <= Max.z;
  }

  NODISCARD bool Intersects(const AABB &other) const {
    return Min.x <= other.Max.x && Max.x >= other.Min.x &&
           Min.y <= other.Max.y && Max.y >= other.Min.y &&
           Min.z <= other.Max.z && Max.z >= other.Min.z;
  }
};

/**
 * @brief A submesh represents a portion of a mesh with its own material
 */
struct SubMesh {
  uint32_t BaseVertex = 0;
  uint32_t BaseIndex = 0;
  uint32_t IndexCount = 0;
  uint32_t MaterialIndex = 0;
  std::string Name;
  AABB BoundingBox;
};

/**
 * @brief Mesh - Contains geometry data for rendering
 *
 * A mesh consists of:
 * - Vertices (position, normal, tangent, UV, color)
 * - Indices (triangle connectivity)
 * - Submeshes (for multi-material support)
 * - Bounding box (for culling)
 */
class BE_API Mesh {
public:
  Mesh() = default;
  Mesh(const std::vector<Vertex> &vertices,
       const std::vector<uint32_t> &indices);
  Mesh(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices);
  ~Mesh() = default;

  // Use your macro instead of manual delete
  BE_DISALLOW_COPY(Mesh)

  // Allow move
  Mesh(Mesh &&other) noexcept = default;
  Mesh &operator=(Mesh &&other) noexcept = default;

  /**
   * @brief Create mesh from vertices and indices
   */
  NODISCARD static Ref<Mesh> Create(const std::vector<Vertex> &vertices,
                                    const std::vector<uint32_t> &indices);

  /**
   * @brief Upload mesh data to GPU
   * Call this after modifying vertices/indices
   */
  void Upload();

  /**
   * @brief Bind the mesh for rendering
   */
  void Bind() const;

  /**
   * @brief Unbind the mesh
   */
  void Unbind() const;

  // Geometry Modification

  /**
   * @brief Recalculate normals from triangle faces
   * @param smooth If true, average normals at shared vertices
   */
  void RecalculateNormals(bool smooth = true);

  /**
   * @brief Recalculate tangents for normal mapping
   */
  void RecalculateTangents();

  /**
   * @brief Recalculate bounding box
   */
  void RecalculateBounds();

  /**
   * @brief Transform all vertices
   */
  void Transform(const glm::mat4 &matrix);

  /**
   * @brief Flip normals (inside out)
   */
  void FlipNormals();

  /**
   * @brief Flip winding order (CCW <-> CW)
   */
  void FlipWindingOrder();

  // Accessors
  NODISCARD const std::vector<Vertex> &GetVertices() const {
    return m_Vertices;
  }
  NODISCARD std::vector<Vertex> &GetVertices() { return m_Vertices; }

  NODISCARD const std::vector<uint32_t> &GetIndices() const {
    return m_Indices;
  }
  NODISCARD std::vector<uint32_t> &GetIndices() { return m_Indices; }

  NODISCARD const std::vector<SubMesh> &GetSubMeshes() const {
    return m_SubMeshes;
  }
  NODISCARD std::vector<SubMesh> &GetSubMeshes() { return m_SubMeshes; }

  NODISCARD const AABB &GetBoundingBox() const { return m_BoundingBox; }

  NODISCARD BE_FORCE_INLINE uint32_t GetVertexCount() const {
    return static_cast<uint32_t>(m_Vertices.size());
  }
  NODISCARD BE_FORCE_INLINE uint32_t GetIndexCount() const {
    return static_cast<uint32_t>(m_Indices.size());
  }
  NODISCARD BE_FORCE_INLINE uint32_t GetTriangleCount() const {
    return GetIndexCount() / 3;
  }

  NODISCARD const std::string &GetName() const { return m_Name; }
  void SetName(const std::string &name) { m_Name = name; }

  NODISCARD BE_FORCE_INLINE bool IsUploaded() const { return m_IsUploaded; }

  NODISCARD const Ref<VertexArray> &GetVertexArray() const {
    return m_VertexArray;
  }

private:
  std::string m_Name = "Unnamed Mesh";

  // CPU data
  std::vector<Vertex> m_Vertices;
  std::vector<uint32_t> m_Indices;
  std::vector<SubMesh> m_SubMeshes;
  AABB m_BoundingBox;

  // GPU resources
  Ref<VertexArray> m_VertexArray;
  Ref<VertexBuffer> m_VertexBuffer;
  Ref<IndexBuffer> m_IndexBuffer;

  bool m_IsUploaded = false;
};

} // namespace BeEngine
