#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
Ref<Mesh> MeshFactory::CreateCube(bool invertNormals) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  float n = invertNormals ? -1.0F : 1.0F;

  // 6 Faces, 4 vertices each = 24 vertices
  // Each Face has unique normals For Flat shading

  // Front Face (+Z)
  vertices.push_back(
      {{-0.5F, -0.5F, 0.5F}, {0, 0, n}, {1, 0, 0, 1}, {0, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, -0.5F, 0.5F}, {0, 0, n}, {1, 0, 0, 1}, {1, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, 0.5F, 0.5F}, {0, 0, n}, {1, 0, 0, 1}, {1, 1}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, 0.5F, 0.5F}, {0, 0, n}, {1, 0, 0, 1}, {0, 1}, {1, 1, 1, 1}});

  // Back Face (-Z)
  vertices.push_back(
      {{0.5F, -0.5F, -0.5F}, {0, 0, -n}, {-1, 0, 0, 1}, {0, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, -0.5F, -0.5F}, {0, 0, -n}, {-1, 0, 0, 1}, {1, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, 0.5F, -0.5F}, {0, 0, -n}, {-1, 0, 0, 1}, {1, 1}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, 0.5F, -0.5F}, {0, 0, -n}, {-1, 0, 0, 1}, {0, 1}, {1, 1, 1, 1}});

  // Top Face (+Y)
  vertices.push_back(
      {{-0.5F, 0.5F, 0.5F}, {0, n, 0}, {1, 0, 0, 1}, {0, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, 0.5F, 0.5F}, {0, n, 0}, {1, 0, 0, 1}, {1, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, 0.5F, -0.5F}, {0, n, 0}, {1, 0, 0, 1}, {1, 1}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, 0.5F, -0.5F}, {0, n, 0}, {1, 0, 0, 1}, {0, 1}, {1, 1, 1, 1}});

  // Bottom Face (-Y)
  vertices.push_back(
      {{-0.5F, -0.5F, -0.5F}, {0, -n, 0}, {1, 0, 0, 1}, {0, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, -0.5F, -0.5F}, {0, -n, 0}, {1, 0, 0, 1}, {1, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, -0.5F, 0.5F}, {0, -n, 0}, {1, 0, 0, 1}, {1, 1}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, -0.5F, 0.5F}, {0, -n, 0}, {1, 0, 0, 1}, {0, 1}, {1, 1, 1, 1}});

  // Right Face (+X)
  vertices.push_back(
      {{0.5F, -0.5F, 0.5F}, {n, 0, 0}, {0, 0, 1, 1}, {0, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, -0.5F, -0.5F}, {n, 0, 0}, {0, 0, 1, 1}, {1, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, 0.5F, -0.5F}, {n, 0, 0}, {0, 0, 1, 1}, {1, 1}, {1, 1, 1, 1}});
  vertices.push_back(
      {{0.5F, 0.5F, 0.5F}, {n, 0, 0}, {0, 0, 1, 1}, {0, 1}, {1, 1, 1, 1}});

  // LeFt Face (-X)
  vertices.push_back(
      {{-0.5F, -0.5F, -0.5F}, {-n, 0, 0}, {0, 0, -1, 1}, {0, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, -0.5F, 0.5F}, {-n, 0, 0}, {0, 0, -1, 1}, {1, 0}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, 0.5F, 0.5F}, {-n, 0, 0}, {0, 0, -1, 1}, {1, 1}, {1, 1, 1, 1}});
  vertices.push_back(
      {{-0.5F, 0.5F, -0.5F}, {-n, 0, 0}, {0, 0, -1, 1}, {0, 1}, {1, 1, 1, 1}});

  // Indices (6 faces * 2 triangles * 3 vertices = 36)
  for (uint32_t face = 0; face < 6; face++) {
    uint32_t base = face * 4;
    if (invertNormals) {
      indices.push_back(base + 0);
      indices.push_back(base + 2);
      indices.push_back(base + 1);
      indices.push_back(base + 2);
      indices.push_back(base + 0);
      indices.push_back(base + 3);
    } else {
      indices.push_back(base + 0);
      indices.push_back(base + 1);
      indices.push_back(base + 2);
      indices.push_back(base + 2);
      indices.push_back(base + 3);
      indices.push_back(base + 0);
    }
  }

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));
  mesh->SetName("Cube");
  mesh->RecalculateTangents();
  mesh->Upload();

  return mesh;
}

Ref<Mesh> MeshFactory::CreateSphere(float radius, uint32_t segments,
                                    uint32_t rings) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  for (uint32_t ring = 0; ring <= rings; ring++) {
    float phi =
        glm::pi<float>() * static_cast<float>(ring) / static_cast<float>(rings);
    float sinPhi = std::sin(phi);
    float cosPhi = std::cos(phi);

    for (uint32_t seg = 0; seg <= segments; seg++) {
      float theta = 2.0F * glm::pi<float>() * static_cast<float>(seg) /
                    static_cast<float>(segments);
      float sinTheta = std::sin(theta);
      float cosTheta = std::cos(theta);

      Vertex vertex;
      vertex.Normal = {cosTheta * sinPhi, cosPhi, sinTheta * sinPhi};
      vertex.Position = vertex.Normal * radius;
      vertex.TexCoord = {static_cast<float>(seg) / static_cast<float>(segments),
                         static_cast<float>(ring) / static_cast<float>(rings)};
      vertex.Color = {1, 1, 1, 1};

      // Calculate tangent
      vertex.Tangent = glm::vec4(-sinTheta, 0, cosTheta, 1.0F);

      vertices.push_back(vertex);
    }
  }

  // Generate indices
  for (uint32_t ring = 0; ring < rings; ring++) {
    for (uint32_t seg = 0; seg < segments; seg++) {
      uint32_t current = (ring * (segments + 1)) + seg;
      uint32_t next = current + segments + 1;

      indices.push_back(current);
      indices.push_back(next);
      indices.push_back(current + 1);

      indices.push_back(current + 1);
      indices.push_back(next);
      indices.push_back(next + 1);
    }
  }

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));
  mesh->SetName("Sphere");
  mesh->Upload();

  return mesh;
}

Ref<Mesh> MeshFactory::CreatePlane(float width, float depth,
                                   uint32_t subdivisions) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  float halfWidth = width * 0.5F;
  float halfDepth = depth * 0.5F;

  uint32_t vertCountX = subdivisions + 1;
  uint32_t vertCountZ = subdivisions + 1;

  for (uint32_t z = 0; z < vertCountZ; z++) {
    for (uint32_t x = 0; x < vertCountX; x++) {
      float u = static_cast<float>(x) / static_cast<float>(subdivisions);
      float v = static_cast<float>(z) / static_cast<float>(subdivisions);

      Vertex vertex;
      vertex.Position = {-halfWidth + (u * width), 0.0F,
                         -halfDepth + (v * depth)};
      vertex.Normal = {0, 1, 0};
      vertex.Tangent = {1, 0, 0, 1};
      vertex.TexCoord = {u, v};
      vertex.Color = {1, 1, 1, 1};

      vertices.push_back(vertex);
    }
  }

  // Generate indices
  for (uint32_t z = 0; z < subdivisions; z++) {
    for (uint32_t x = 0; x < subdivisions; x++) {
      uint32_t topLeft = (z * vertCountX) + x;
      uint32_t topRight = topLeft + 1;
      uint32_t bottomLeft = ((z + 1) * vertCountX) + x;
      uint32_t bottomRight = bottomLeft + 1;

      indices.push_back(topLeft);
      indices.push_back(bottomLeft);
      indices.push_back(topRight);

      indices.push_back(topRight);
      indices.push_back(bottomLeft);
      indices.push_back(bottomRight);
    }
  }

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));
  mesh->SetName("Plane");
  mesh->Upload();

  return mesh;
}

Ref<Mesh> MeshFactory::CreateQuad(float width, float height) {
  float hw = width * 0.5F;
  float hh = height * 0.5F;

  std::vector<Vertex> vertices = {
      Vertex{
          .Position = {-hw, -hh, 0.0F},
          .TexCoord = {0.0F, 0.0F},
      },
      Vertex{
          .Position = {hw, -hh, 0.0F},
          .TexCoord = {1.0F, 0.0F},
      },
      Vertex{
          .Position = {hw, hh, 0.0F},
          .TexCoord = {1.0F, 1.0F},
      },
      Vertex{
          .Position = {-hw, hh, 0.0F},
          .TexCoord = {0.0F, 1.0F},
      },
  };

  std::vector<uint32_t> indices = {0, 1, 2, 2, 3, 0};

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));
  mesh->SetName("Quad");
  mesh->Upload();

  return mesh;
}

// Cylinder and other primitives can be added similarly...

Ref<Mesh> MeshFactory::CreateCylinder(float radius, float height,
                                      uint32_t segments, bool caps) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  float halfHeight = height * 0.5F;

  // Side vertices
  for (uint32_t i = 0; i <= segments; i++) {
    float theta = 2.0F * glm::pi<float>() * static_cast<float>(i) /
                  static_cast<float>(segments);
    float cosT = std::cos(theta);
    float sinT = std::sin(theta);

    // Bottom vertex
    Vertex bottom;
    bottom.Position = {radius * cosT, -halfHeight, radius * sinT};
    bottom.Normal = {cosT, 0, sinT};
    bottom.Tangent = {-sinT, 0, cosT, 1};
    bottom.TexCoord = {static_cast<float>(i) / segments, 0};
    bottom.Color = {1, 1, 1, 1};
    vertices.push_back(bottom);

    // Top vertex
    Vertex top;
    top.Position = {radius * cosT, halfHeight, radius * sinT};
    top.Normal = {cosT, 0, sinT};
    top.Tangent = {-sinT, 0, cosT, 1};
    top.TexCoord = {static_cast<float>(i) / segments, 1};
    top.Color = {1, 1, 1, 1};
    vertices.push_back(top);
  }

  // Side indices
  for (uint32_t i = 0; i < segments; i++) {
    uint32_t base = i * 2;
    indices.push_back(base);
    indices.push_back(base + 2);
    indices.push_back(base + 1);

    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
  }

  if (caps) {
    auto centerBottom = static_cast<uint32_t>(vertices.size());
    uint32_t centerTop = centerBottom + 1;

    // Center vertices for caps
    Vertex centerB;
    centerB.Position = {0, -halfHeight, 0};
    centerB.Normal = {0, -1, 0};
    centerB.Tangent = {1, 0, 0, 1};
    centerB.TexCoord = {0.5F, 0.5F};
    centerB.Color = {1, 1, 1, 1};
    vertices.push_back(centerB);

    Vertex centerT;
    centerT.Position = {0, halfHeight, 0};
    centerT.Normal = {0, 1, 0};
    centerT.Tangent = {1, 0, 0, 1};
    centerT.TexCoord = {0.5F, 0.5F};
    centerT.Color = {1, 1, 1, 1};
    vertices.push_back(centerT);

    // Cap vertices
    auto capStartBottom = static_cast<uint32_t>(vertices.size());
    uint32_t capStartTop = capStartBottom + segments + 1;

    for (uint32_t i = 0; i <= segments; i++) {
      float theta = 2.0F * glm::pi<float>() * static_cast<float>(i) /
                    static_cast<float>(segments);
      float cosT = std::cos(theta);
      float sinT = std::sin(theta);

      // Bottom cap
      Vertex capB;
      capB.Position = {radius * cosT, -halfHeight, radius * sinT};
      capB.Normal = {0, -1, 0};
      capB.Tangent = {1, 0, 0, 1};
      capB.TexCoord = {(cosT * 0.5F) + 0.5F, (sinT * 0.5F) + 0.5F};
      capB.Color = {1, 1, 1, 1};
      vertices.push_back(capB);
    }

    for (uint32_t i = 0; i <= segments; i++) {
      float theta = 2.0F * glm::pi<float>() * static_cast<float>(i) /
                    static_cast<float>(segments);
      float cosT = std::cos(theta);
      float sinT = std::sin(theta);

      // Top cap
      Vertex capT;
      capT.Position = {radius * cosT, halfHeight, radius * sinT};
      capT.Normal = {0, 1, 0};
      capT.Tangent = {1, 0, 0, 1};
      capT.TexCoord = {(cosT * 0.5F) + 0.5F, sinT * 0.5F + 0.5F};
      capT.Color = {1, 1, 1, 1};
      vertices.push_back(capT);
    }

    // Cap indices
    for (uint32_t i = 0; i < segments; i++) {
      // Bottom cap (reverse winding)
      indices.push_back(centerBottom);
      indices.push_back(capStartBottom + i + 1);
      indices.push_back(capStartBottom + i);

      // Top cap
      indices.push_back(centerTop);
      indices.push_back(capStartTop + i);
      indices.push_back(capStartTop + i + 1);
    }
  }

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));
  mesh->SetName("Cylinder");
  mesh->Upload();

  return mesh;
}

Ref<Mesh> MeshFactory::CreateCone(float radius, float height, uint32_t segments,
                                  bool cap) {
  // Similar to cylinder but with apex
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  float halfHeight = height * 0.5F;

  // Apex vertex
  uint32_t apexIndex = 0;
  Vertex apex;
  apex.Position = {0, halfHeight, 0};
  apex.Normal = {0, 1, 0}; // Will be adjusted per-face
  apex.Tangent = {1, 0, 0, 1};
  apex.TexCoord = {0.5F, 1};
  apex.Color = {1, 1, 1, 1};
  vertices.push_back(apex);

  // Base vertices
  for (uint32_t i = 0; i <= segments; i++) {
    float theta = 2.0F * glm::pi<float>() * static_cast<float>(i) /
                  static_cast<float>(segments);
    float cosT = std::cos(theta);
    float sinT = std::sin(theta);

    // Calculate normal (pointing outward along cone surface)
    float slope = radius / height;
    glm::vec3 normal = glm::normalize(glm::vec3(cosT, slope, sinT));

    Vertex vertex;
    vertex.Position = {radius * cosT, -halfHeight, radius * sinT};
    vertex.Normal = normal;
    vertex.Tangent = {-sinT, 0, cosT, 1};
    vertex.TexCoord = {static_cast<float>(i) / segments, 0};
    vertex.Color = {1, 1, 1, 1};
    vertices.push_back(vertex);
  }

  // Side indices
  for (uint32_t i = 0; i < segments; i++) {
    indices.push_back(apexIndex);
    indices.push_back(i + 1);
    indices.push_back(i + 2);
  }

  if (cap) {
    uint32_t centerIndex = static_cast<uint32_t>(vertices.size());

    Vertex center;
    center.Position = {0, -halfHeight, 0};
    center.Normal = {0, -1, 0};
    center.Tangent = {1, 0, 0, 1};
    center.TexCoord = {0.5F, 0.5F};
    center.Color = {1, 1, 1, 1};
    vertices.push_back(center);

    auto capStart = static_cast<uint32_t>(vertices.size());

    for (uint32_t i = 0; i <= segments; i++) {
      float theta = 2.0F * glm::pi<float>() * static_cast<float>(i) /
                    static_cast<float>(segments);
      float cosT = std::cos(theta);
      float sinT = std::sin(theta);

      Vertex capV;
      capV.Position = {radius * cosT, -halfHeight, radius * sinT};
      capV.Normal = {0, -1, 0};
      capV.Tangent = {1, 0, 0, 1};
      capV.TexCoord = {(cosT * 0.5F) + 0.5F, (sinT * 0.5F) + 0.5F};
      capV.Color = {1, 1, 1, 1};
      vertices.push_back(capV);
    }

    for (uint32_t i = 0; i < segments; i++) {
      indices.push_back(centerIndex);
      indices.push_back(capStart + i + 1);
      indices.push_back(capStart + i);
    }
  }

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));
  mesh->SetName("Cone");
  mesh->Upload();

  return mesh;
}

Ref<Mesh> MeshFactory::CreateTorus(float outerRadius, float innerRadius,
                                   uint32_t segments, uint32_t rings) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  for (uint32_t ring = 0; ring <= rings; ring++) {
    float theta = 2.0F * glm::pi<float>() * static_cast<float>(ring) /
                  static_cast<float>(rings);
    float cosT = std::cos(theta);
    float sinT = std::sin(theta);

    for (uint32_t seg = 0; seg <= segments; seg++) {
      float phi = 2.0F * glm::pi<float>() * static_cast<float>(seg) /
                  static_cast<float>(segments);
      float cosP = std::cos(phi);
      float sinP = std::sin(phi);

      // Center of tube section
      glm::vec3 tubeCenter = {outerRadius * cosT, 0, outerRadius * sinT};

      // Position on tube surface
      glm::vec3 tubeOffset = {innerRadius * cosT * cosP, innerRadius * sinP,
                              innerRadius * sinT * cosP};

      Vertex vertex;
      vertex.Position = tubeCenter + tubeOffset;
      vertex.Normal = glm::normalize(tubeOffset);
      vertex.Tangent = glm::vec4(-sinT, 0, cosT, 1);
      vertex.TexCoord = {static_cast<float>(ring) / static_cast<float>(rings),
                         static_cast<float>(seg) /
                             static_cast<float>(segments)};
      vertex.Color = {1, 1, 1, 1};

      vertices.push_back(vertex);
    }
  }

  // Indices
  for (uint32_t ring = 0; ring < rings; ring++) {
    for (uint32_t seg = 0; seg < segments; seg++) {
      uint32_t current = (ring * (segments + 1)) + seg;
      uint32_t next = current + segments + 1;

      indices.push_back(current);
      indices.push_back(next);
      indices.push_back(current + 1);

      indices.push_back(current + 1);
      indices.push_back(next);
      indices.push_back(next + 1);
    }
  }

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));
  mesh->SetName("Torus");
  mesh->Upload();

  return mesh;
}

} // namespace BeEngine
