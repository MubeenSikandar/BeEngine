#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

Mesh::Mesh(const std::vector<Vertex> &vertices,
           const std::vector<uint32_t> &indices)
    : m_Vertices(vertices), m_Indices(indices) {

  if (BE_LIKELY(!m_Vertices.empty())) {
    // Add default submesh covering entire mesh
    SubMesh submesh;
    submesh.BaseVertex = 0;
    submesh.BaseIndex = 0;
    submesh.IndexCount = static_cast<uint32_t>(m_Indices.size());
    submesh.MaterialIndex = 0;
    submesh.Name = "Default";
    m_SubMeshes.push_back(submesh);

    RecalculateBounds();
  }
}

Mesh::Mesh(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices)
    : m_Vertices(std::move(vertices)), m_Indices(std::move(indices)) {

  if (BE_LIKELY(!m_Vertices.empty())) {
    SubMesh submesh;
    submesh.BaseVertex = 0;
    submesh.BaseIndex = 0;
    submesh.IndexCount = static_cast<uint32_t>(m_Indices.size());
    submesh.MaterialIndex = 0;
    submesh.Name = "Default";
    m_SubMeshes.push_back(submesh);

    RecalculateBounds();
  }
}

Ref<Mesh> Mesh::Create(const std::vector<Vertex> &vertices,
                       const std::vector<uint32_t> &indices) {
  auto mesh = CreateRef<Mesh>(vertices, indices);
  mesh->Upload();
  return mesh;
}

void Mesh::Upload() {
  if (BE_UNLIKELY(m_Vertices.empty())) {
    BE_CORE_WARN("Mesh::Upload() called with no vertices");
    return;
  }

  // Create VAO
  m_VertexArray = VertexArray::Create();

  // Create and upload vertex buffer
  m_VertexBuffer = VertexBuffer::Create(
      m_Vertices.data(),
      static_cast<uint32_t>(m_Vertices.size() * sizeof(Vertex)));
  m_VertexBuffer->SetLayout(Vertex::GetLayout());
  m_VertexArray->AddVertexBuffer(m_VertexBuffer);

  // Create and upload index buffer
  if (BE_LIKELY(!m_Indices.empty())) {
    m_IndexBuffer = IndexBuffer::Create(
        m_Indices.data(), static_cast<uint32_t>(m_Indices.size()));
    m_VertexArray->SetIndexBuffer(m_IndexBuffer);
  }

  m_IsUploaded = true;

  BE_CORE_TRACE("Mesh '{}' uploaded: {} vertices, {} indices", m_Name,
                m_Vertices.size(), m_Indices.size());
}

void Mesh::Bind() const {
  BE_CORE_ASSERT(m_VertexArray != nullptr, "Mesh not uploaded before binding!");
  m_VertexArray->Bind();
}

void Mesh::Unbind() const {
  if (BE_LIKELY(m_VertexArray)) {
    m_VertexArray->Unbind();
  }
}

void Mesh::RecalculateNormals(bool smooth) {
  if (BE_UNLIKELY(m_Indices.empty() || m_Vertices.empty())) {
    BE_CORE_WARN("Cannot recalculate normals: mesh has no geometry");
    return;
  }

  // Reset all normals to zero
  for (auto &vertex : m_Vertices) {
    vertex.Normal = glm::vec3(0.0F);
  }

  // Calculate face normals and accumulate
  for (size_t i = 0; i < m_Indices.size(); i += 3) {
    uint32_t i0 = m_Indices[i];
    uint32_t i1 = m_Indices[i + 1];
    uint32_t i2 = m_Indices[i + 2];

    BE_CORE_ASSERT(i0 < m_Vertices.size() && i1 < m_Vertices.size() &&
                       i2 < m_Vertices.size(),
                   "Index out of bounds in mesh!");

    glm::vec3 v0 = m_Vertices[i0].Position;
    glm::vec3 v1 = m_Vertices[i1].Position;
    glm::vec3 v2 = m_Vertices[i2].Position;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

    if (smooth) {
      m_Vertices[i0].Normal += faceNormal;
      m_Vertices[i1].Normal += faceNormal;
      m_Vertices[i2].Normal += faceNormal;
    } else {
      m_Vertices[i0].Normal = faceNormal;
      m_Vertices[i1].Normal = faceNormal;
      m_Vertices[i2].Normal = faceNormal;
    }
  }

  // Normalize accumulated normals
  if (smooth) {
    for (auto &vertex : m_Vertices) {
      float length = glm::length(vertex.Normal);
      if (BE_LIKELY(length > 0.0001F)) {
        vertex.Normal = glm::normalize(vertex.Normal);
      } else {
        vertex.Normal = glm::vec3(0.0F, 1.0F, 0.0F);
      }
    }
  }

  BE_CORE_TRACE("Mesh '{}': Recalculated normals (smooth={})", m_Name, smooth);
}

void Mesh::RecalculateTangents() {
  if (BE_UNLIKELY(m_Indices.empty() || m_Vertices.empty())) {
    BE_CORE_WARN("Cannot recalculate tangents: mesh has no geometry");
    return;
  }

  std::vector<glm::vec3> tangents(m_Vertices.size(), glm::vec3(0.0F));
  std::vector<glm::vec3> bitangents(m_Vertices.size(), glm::vec3(0.0F));

  for (size_t i = 0; i < m_Indices.size(); i += 3) {
    uint32_t i0 = m_Indices[i];
    uint32_t i1 = m_Indices[i + 1];
    uint32_t i2 = m_Indices[i + 2];

    const glm::vec3 &v0 = m_Vertices[i0].Position;
    const glm::vec3 &v1 = m_Vertices[i1].Position;
    const glm::vec3 &v2 = m_Vertices[i2].Position;

    const glm::vec2 &uv0 = m_Vertices[i0].TexCoord;
    const glm::vec2 &uv1 = m_Vertices[i1].TexCoord;
    const glm::vec2 &uv2 = m_Vertices[i2].TexCoord;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;

    glm::vec2 deltaUV1 = uv1 - uv0;
    glm::vec2 deltaUV2 = uv2 - uv0;

    float denom = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
    float f = 1.0F / (denom + 0.0001F); // Avoid division by zero

    glm::vec3 tangent;
    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    glm::vec3 bitangent;
    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

    tangents[i0] += tangent;
    tangents[i1] += tangent;
    tangents[i2] += tangent;

    bitangents[i0] += bitangent;
    bitangents[i1] += bitangent;
    bitangents[i2] += bitangent;
  }

  // Orthonormalize and store
  for (size_t i = 0; i < m_Vertices.size(); i++) {
    const glm::vec3 &n = m_Vertices[i].Normal;
    const glm::vec3 &t = tangents[i];
    const glm::vec3 &b = bitangents[i];

    // Gram-Schmidt orthogonalize
    glm::vec3 tangent = glm::normalize(t - n * glm::dot(n, t));

    // Calculate handedness
    float handedness = (glm::dot(glm::cross(n, t), b) < 0.0F) ? -1.0F : 1.0F;

    m_Vertices[i].Tangent = glm::vec4(tangent, handedness);
  }

  BE_CORE_TRACE("Mesh '{}': Recalculated tangents", m_Name);
}

void Mesh::RecalculateBounds() {
  m_BoundingBox = AABB();

  for (const auto &vertex : m_Vertices) {
    m_BoundingBox.Expand(vertex.Position);
  }

  // Update submesh bounds
  for (auto &submesh : m_SubMeshes) {
    submesh.BoundingBox = AABB();
    uint32_t endIndex = submesh.BaseIndex + submesh.IndexCount;

    BE_CORE_ASSERT(endIndex <= m_Indices.size(),
                   "Submesh indices out of range!");

    for (uint32_t i = submesh.BaseIndex; i < endIndex; i++) {
      BE_CORE_ASSERT(m_Indices[i] < m_Vertices.size(), "Index out of bounds!");
      submesh.BoundingBox.Expand(m_Vertices[m_Indices[i]].Position);
    }
  }
}

void Mesh::Transform(const glm::mat4 &matrix) {
  glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));

  for (auto &vertex : m_Vertices) {
    vertex.Position = glm::vec3(matrix * glm::vec4(vertex.Position, 1.0F));
    vertex.Normal = glm::normalize(normalMatrix * vertex.Normal);
    vertex.Tangent =
        glm::vec4(glm::normalize(normalMatrix * glm::vec3(vertex.Tangent)),
                  vertex.Tangent.w);
  }

  RecalculateBounds();
}

void Mesh::FlipNormals() {
  for (auto &vertex : m_Vertices) {
    vertex.Normal = -vertex.Normal;
  }
  BE_CORE_TRACE("Mesh '{}': Flipped normals", m_Name);
}

void Mesh::FlipWindingOrder() {
  for (size_t i = 0; i < m_Indices.size(); i += 3) {
    std::swap(m_Indices[i + 1], m_Indices[i + 2]);
  }
  BE_CORE_TRACE("Mesh '{}': Flipped winding order", m_Name);
}

} // namespace BeEngine
