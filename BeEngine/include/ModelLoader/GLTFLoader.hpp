// ModelLoader/GLTFLoader.hpp
#pragma once

#include <Core.hpp>
#include <ModelLoader/Model.hpp>
#include <tiny_gltf.h>

namespace tinygltf {
class Model;
struct Primitive;
} // namespace tinygltf

namespace BeEngine {

/**
 * @brief Loads glTF 2.0 models (.gltf and .glb files)
 *
 * Supported features:
 * - glTF 2.0 and glb binary format
 * - PBR materials (metallic-roughness workflow)
 * - Multiple meshes and primitives
 * - Scene hierarchy
 * - Embedded and external textures
 * - Normal maps, occlusion maps, emissive maps
 *
 * Not yet supported:
 * - Animations
 * - Skinning
 * - Morph targets
 * - Multiple scenes
 */
class BE_API GLTFLoader {
public:
  BE_DISALLOW_COPY_AND_MOVE(GLTFLoader)

  /**
   * @brief Load a glTF or glb model
   * @param filepath Path to the model file
   * @return Loaded model, or nullptr on failure
   */
  NODISCARD static Ref<Model> Load(const std::string &filepath);

  /**
   * @brief Check if a file is a supported glTF format
   */
  NODISCARD static bool IsSupported(const std::string &filepath);

private:
  GLTFLoader() = default;

  // Internal loading helpers
  static void
  LoadNode(const tinygltf::Model &model, int nodeIndex, int parentIndex,
           Ref<Model> &outModel, const std::string &directory,
           std::vector<Ref<Texture2D>> &textureCache,
           std::unordered_map<int32_t, Ref<PBRMaterial>> &materialCache);

  static Ref<Mesh> LoadPrimitive(const tinygltf::Model &model,
                                 const tinygltf::Primitive &primitive);

  static Ref<PBRMaterial>
  LoadMaterial(const tinygltf::Model &model, int materialIndex,
               const std::string &directory,
               std::vector<Ref<Texture2D>> &textureCache);

  static Ref<Texture2D> LoadTexture(const tinygltf::Model &model,
                                    int textureIndex,
                                    const std::string &directory, bool sRGB);
};
} // namespace BeEngine
