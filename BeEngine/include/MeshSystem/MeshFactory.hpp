#pragma once

#include <Core.hpp>
#include <MeshSystem/Mesh.hpp>
#include <cstdint>

namespace BeEngine {

/**
 * @brief Factory for creating primitive meshes
 */
class BE_API MeshFactory {
public:
  BE_DISALLOW_COPY_AND_MOVE(MeshFactory)

  /**
   * @brief Create a unit cube (-0.5 to 0.5)
   * @param invertNormals If true, normals point inward (for skybox)
   */
  NODISCARD static Ref<Mesh> CreateCube(bool invertNormals = false);

  /**
   * @brief Create a UV sphere
   * @param radius Sphere radius
   * @param segments Horizontal segments (longitude)
   * @param rings Vertical rings (latitude)
   */
  NODISCARD static Ref<Mesh> CreateSphere(float radius = 0.5F,
                                          uint32_t segments = 32,
                                          uint32_t rings = 16);

  /**
   * @brief Create a plane on XZ axis
   * @param width Width (X axis)
   * @param depth Depth (Z axis)
   * @param subdivisions Number of subdivisions
   */
  NODISCARD static Ref<Mesh> CreatePlane(float width = 1.0F, float depth = 1.0F,
                                         uint32_t subdivisions = 1);

  /**
   * @brief Create a cylinder
   * @param radius Cylinder radius
   * @param height Cylinder height
   * @param segments Number of radial segments
   * @param caps Whether to include top and bottom caps
   */
  NODISCARD static Ref<Mesh> CreateCylinder(float radius = 0.5F,
                                            float height = 1.0F,
                                            uint32_t segments = 32,
                                            bool caps = true);

  /**
   * @brief Create a cone
   * @param radius Base radius
   * @param height Cone height
   * @param segments Number of radial segments
   * @param cap Whether to include bottom cap
   */
  NODISCARD static Ref<Mesh> CreateCone(float radius = 0.5F,
                                        float height = 1.0F,
                                        uint32_t segments = 32,
                                        bool cap = true);

  /**
   * @brief Create a torus (donut)
   * @param outerRadius Distance from center to tube center
   * @param innerRadius Tube radius
   * @param segments Radial segments
   * @param rings Tube segments
   */
  NODISCARD static Ref<Mesh> CreateTorus(float outerRadius = 0.5F,
                                         float innerRadius = 0.2F,
                                         uint32_t segments = 32,
                                         uint32_t rings = 16);

  /**
   * @brief Create a quad (two triangles)
   * @param width Width
   * @param height Height
   */
  NODISCARD static Ref<Mesh> CreateQuad(float width = 1.0F,
                                        float height = 1.0F);

private:
  MeshFactory() = default; // Static class, no instantiation
};
} // namespace BeEngine
