// BeEngine/ModelLoader/GLTFLoader
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

namespace {
// ============================================================
// Helper Functions
// ============================================================

std::string GetDirectory(const std::string &filepath) {
  std::filesystem::path path(filepath);
  return path.parent_path().string();
}

std::string GetFilename(const std::string &filepath) {
  std::filesystem::path path(filepath);
  return path.stem().string();
}

bool EndsWith(const std::string &str, const std::string &suffix) {
  if (suffix.size() > str.size()) {
    return false;
  }
  return std::equal(
      suffix.rbegin(), suffix.rend(), str.rbegin(),
      [](char a, char b) { return std::tolower(a) == std::tolower(b); });
}

// ============================================================
// Accessor Reading Helpers (these DON'T need Model access)
// ============================================================

std::vector<glm::vec2> ReadVec2Accessor(const tinygltf::Model &model,
                                        int accessorIndex) {
  std::vector<glm::vec2> result;
  if (accessorIndex < 0) {
    return result;
  }

  const auto &accessor = model.accessors[accessorIndex];
  const auto &bufferView = model.bufferViews[accessor.bufferView];
  const auto &buffer = model.buffers[bufferView.buffer];

  const size_t elementSize = sizeof(float) * 2;
  const size_t byteStride =
      bufferView.byteStride ? bufferView.byteStride : elementSize;
  const uint8_t *data =
      buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

  result.resize(accessor.count);
  for (size_t i = 0; i < accessor.count; i++) {
    const auto *ptr = reinterpret_cast<const float *>(data + (i * byteStride));
    result[i] = glm::vec2(ptr[0], ptr[1]);
  }
  return result;
}

std::vector<glm::vec3> ReadVec3Accessor(const tinygltf::Model &model,
                                        int accessorIndex) {
  std::vector<glm::vec3> result;
  if (accessorIndex < 0) {
    return result;
  }

  const auto &accessor = model.accessors[accessorIndex];
  const auto &bufferView = model.bufferViews[accessor.bufferView];
  const auto &buffer = model.buffers[bufferView.buffer];

  const size_t elementSize = sizeof(float) * 3;
  const size_t byteStride =
      bufferView.byteStride ? bufferView.byteStride : elementSize;
  const uint8_t *data =
      buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

  result.resize(accessor.count);
  for (size_t i = 0; i < accessor.count; i++) {
    const auto *ptr = reinterpret_cast<const float *>(data + (i * byteStride));
    result[i] = glm::vec3(ptr[0], ptr[1], ptr[2]);
  }
  return result;
}

std::vector<glm::vec4> ReadVec4Accessor(const tinygltf::Model &model,
                                        int accessorIndex) {
  std::vector<glm::vec4> result;
  if (accessorIndex < 0) {
    return result;
  }
  const auto &accessor = model.accessors[accessorIndex];
  const auto &bufferView = model.bufferViews[accessor.bufferView];
  const auto &buffer = model.buffers[bufferView.buffer];

  const size_t elementSize = sizeof(float) * 4;
  const size_t byteStride =
      bufferView.byteStride ? bufferView.byteStride : elementSize;
  const uint8_t *data =
      buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

  result.resize(accessor.count);
  for (size_t i = 0; i < accessor.count; i++) {
    const auto *ptr = reinterpret_cast<const float *>(data + (i * byteStride));
    result[i] = glm::vec4(ptr[0], ptr[1], ptr[2], ptr[3]);
  }
  return result;
}

std::vector<uint32_t> ReadIndices(const tinygltf::Model &model,
                                  int accessorIndex) {
  std::vector<uint32_t> result;
  if (accessorIndex < 0) {
    return result;
  }

  const auto &accessor = model.accessors[accessorIndex];
  const auto &bufferView = model.bufferViews[accessor.bufferView];
  const auto &buffer = model.buffers[bufferView.buffer];

  const uint8_t *data =
      buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

  result.resize(accessor.count);

  switch (accessor.componentType) {
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
    for (size_t i = 0; i < accessor.count; i++) {
      result[i] = static_cast<uint32_t>(data[i]);
    }
    break;
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
    const auto *ptr = reinterpret_cast<const uint16_t *>(data);
    for (size_t i = 0; i < accessor.count; i++) {
      result[i] = static_cast<uint32_t>(ptr[i]);
    }
    break;
  }
  case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT: {
    const auto *ptr = reinterpret_cast<const uint32_t *>(data);
    for (size_t i = 0; i < accessor.count; i++) {
      result[i] = ptr[i];
    }
    break;
  }
  default:
    BE_CORE_ERROR("Unsupported index component type: {}",
                  accessor.componentType);
    break;
  }

  return result;
}

} // anonymous namespace

// ============================================================
// GLTFLoader MEMBER FUNCTIONS
// ============================================================

Ref<Texture2D> GLTFLoader::LoadTexture(const tinygltf::Model &model,
                                       int textureIndex,
                                       const std::string &directory,
                                       bool sRGB) {
  BE_CORE_TRACE("    LoadTexture: index={}, sRGB={}", textureIndex, sRGB);

  if (textureIndex < 0) {
    BE_CORE_TRACE("    Invalid texture index");
    return nullptr;
  }

  const auto &texture = model.textures[textureIndex];
  if (texture.source < 0) {
    BE_CORE_TRACE("    Invalid texture source");
    return nullptr;
  }

  const auto &image = model.images[texture.source];
  BE_CORE_TRACE("    Image: {}x{}, {} components, embedded={}", image.width,
                image.height, image.component, !image.image.empty());

  // Check if image has embedded data
  if (!image.image.empty()) {
    BE_CORE_TRACE("    Creating texture from embedded data ({} bytes)",
                  image.image.size());

    auto tex = Texture2D::Create(static_cast<uint32_t>(image.width),
                                 static_cast<uint32_t>(image.height),
                                 image.component == 4, sRGB);

    if (tex) {
      BE_CORE_TRACE("    Uploading texture data...");
      tex->SetData(image.image.data(),
                   static_cast<uint32_t>(image.image.size()));
      BE_CORE_TRACE("    Texture upload complete");
    }
    return tex;
  }

  // Load from external file
  if (!image.uri.empty()) {
    std::string texturePath = directory + "/" + image.uri;
    BE_CORE_TRACE("    Loading external texture: {}", texturePath);
    return Texture2D::Create(texturePath, sRGB);
  }

  BE_CORE_WARN("Could not load texture: no data or URI");
  return nullptr;
}

Ref<PBRMaterial>
GLTFLoader::LoadMaterial(const tinygltf::Model &model, int materialIndex,
                         const std::string &directory,
                         std::vector<Ref<Texture2D>> &textureCache) {

  BE_CORE_TRACE("LoadMaterial: index={}", materialIndex);

  // Determine name FIRST, before creating any material
  std::string name = "Default Material";
  if (materialIndex >= 0 &&
      materialIndex < static_cast<int>(model.materials.size())) {
    const auto &gltfMat = model.materials[materialIndex];
    name = gltfMat.name.empty() ? ("Material_" + std::to_string(materialIndex))
                                : gltfMat.name;
  }

  BE_CORE_TRACE("  Creating PBRMaterial: {}", name);

  // Create exactly ONE material
  auto material = CreateRef<PBRMaterial>(name);

  BE_CORE_TRACE("  PBRMaterial created successfully");

  // Configure for default case
  if (materialIndex < 0 ||
      materialIndex >= static_cast<int>(model.materials.size())) {
    material->SetAlbedoColor(glm::vec4(0.8F, 0.8F, 0.8F, 1.0F));
    material->SetMetallic(0.0F);
    material->SetRoughness(0.5F);
    return material;
  }

  const auto &gltfMat = model.materials[materialIndex];
  const auto &pbr = gltfMat.pbrMetallicRoughness;

  BE_CORE_TRACE("  Setting albedo color");
  material->SetAlbedoColor(
      glm::vec4(static_cast<float>(pbr.baseColorFactor[0]),
                static_cast<float>(pbr.baseColorFactor[1]),
                static_cast<float>(pbr.baseColorFactor[2]),
                static_cast<float>(pbr.baseColorFactor[3])));

  BE_CORE_TRACE("  Loading base color texture (index={})",
                pbr.baseColorTexture.index);
  if (pbr.baseColorTexture.index >= 0) {
    auto tex = LoadTexture(model, pbr.baseColorTexture.index, directory, true);
    BE_CORE_TRACE("  Base color texture loaded: {}", tex ? "success" : "null");
    if (tex) {
      material->SetAlbedoMap(tex);
      textureCache.push_back(tex);
    }
  }

  BE_CORE_TRACE("  Setting metallic/roughness factors");
  material->SetMetallic(static_cast<float>(pbr.metallicFactor));
  material->SetRoughness(static_cast<float>(pbr.roughnessFactor));

  BE_CORE_TRACE("  Loading metallic-roughness texture (index={})",
                pbr.metallicRoughnessTexture.index);
  if (pbr.metallicRoughnessTexture.index >= 0) {
    auto tex = LoadTexture(model, pbr.metallicRoughnessTexture.index, directory,
                           false);
    BE_CORE_TRACE("  Metallic-roughness texture loaded: {}",
                  tex ? "success" : "null");
    if (tex) {
      material->SetMetallicMap(tex);
      material->SetRoughnessMap(tex);
      textureCache.push_back(tex);
    }
  }

  BE_CORE_TRACE("  Loading normal texture (index={})",
                gltfMat.normalTexture.index);
  if (gltfMat.normalTexture.index >= 0) {
    auto tex =
        LoadTexture(model, gltfMat.normalTexture.index, directory, false);
    BE_CORE_TRACE("  Normal texture loaded: {}", tex ? "success" : "null");
    if (tex) {
      material->SetNormalMap(tex);
      textureCache.push_back(tex);
    }
  }

  BE_CORE_TRACE("  Loading occlusion texture (index={})",
                gltfMat.occlusionTexture.index);
  if (gltfMat.occlusionTexture.index >= 0) {
    auto tex =
        LoadTexture(model, gltfMat.occlusionTexture.index, directory, false);
    BE_CORE_TRACE("  Occlusion texture loaded: {}", tex ? "success" : "null");
    if (tex) {
      material->SetAOMap(tex);
      material->SetAO(1.0F);
      textureCache.push_back(tex);
    }
  }

  BE_CORE_TRACE("  Setting emissive");
  material->SetEmissiveColor(
      glm::vec3(static_cast<float>(gltfMat.emissiveFactor[0]),
                static_cast<float>(gltfMat.emissiveFactor[1]),
                static_cast<float>(gltfMat.emissiveFactor[2])));

  BE_CORE_TRACE("  Loading emissive texture (index={})",
                gltfMat.emissiveTexture.index);
  if (gltfMat.emissiveTexture.index >= 0) {
    auto tex =
        LoadTexture(model, gltfMat.emissiveTexture.index, directory, true);
    BE_CORE_TRACE("  Emissive texture loaded: {}", tex ? "success" : "null");
    if (tex) {
      material->SetEmissiveMap(tex);
      material->SetEmissiveIntensity(1.0F);
      textureCache.push_back(tex);
    }
  }

  BE_CORE_TRACE("  Setting alpha mode and double-sided");
  if (gltfMat.alphaMode == "BLEND") {
    material->SetTransparent(true);
  }
  material->SetDoubleSided(gltfMat.doubleSided);

  BE_CORE_TRACE("  LoadMaterial complete for: {}", name);
  return material;
}

Ref<Mesh> GLTFLoader::LoadPrimitive(const tinygltf::Model &model,
                                    const tinygltf::Primitive &primitive) {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;

  int posAccessor = -1;
  int normalAccessor = -1;
  int tangentAccessor = -1;
  int texCoordAccessor = -1;
  int colorAccessor = -1;

  for (const auto &[name, index] : primitive.attributes) {
    if (name == "POSITION") {
      posAccessor = index;
    } else if (name == "NORMAL") {
      normalAccessor = index;
    } else if (name == "TANGENT") {
      tangentAccessor = index;
    } else if (name == "TEXCOORD_0") {
      texCoordAccessor = index;
    } else if (name == "COLOR_0") {
      colorAccessor = index;
    }
  }

  if (posAccessor < 0) {
    BE_CORE_ERROR("Mesh primitive has no POSITION attribute!");
    return nullptr;
  }

  auto positions = ReadVec3Accessor(model, posAccessor);
  if (positions.empty()) {
    BE_CORE_ERROR("Failed to read positions!");
    return nullptr;
  }

  auto normals = ReadVec3Accessor(model, normalAccessor);
  auto tangents = ReadVec4Accessor(model, tangentAccessor);
  auto texCoords = ReadVec2Accessor(model, texCoordAccessor);
  auto colors = ReadVec4Accessor(model, colorAccessor);

  vertices.resize(positions.size());

  for (size_t i = 0; i < positions.size(); i++) {
    vertices[i].Position = positions[i];
    vertices[i].Normal =
        (i < normals.size()) ? normals[i] : glm::vec3(0.0F, 1.0F, 0.0F);
    vertices[i].Tangent =
        (i < tangents.size()) ? tangents[i] : glm::vec4(1.0F, 0.0F, 0.0F, 1.0F);
    vertices[i].TexCoord =
        (i < texCoords.size()) ? texCoords[i] : glm::vec2(0.0F);
    vertices[i].Color = (i < colors.size()) ? colors[i] : glm::vec4(1.0F);
  }

  if (primitive.indices >= 0) {
    indices = ReadIndices(model, primitive.indices);
  } else {
    indices.resize(positions.size());
    for (size_t i = 0; i < positions.size(); i++) {
      indices[i] = static_cast<uint32_t>(i);
    }
  }

  auto mesh = CreateRef<Mesh>(std::move(vertices), std::move(indices));

  if (normals.empty()) {
    mesh->RecalculateNormals(true);
  }
  if (tangents.empty()) {
    mesh->RecalculateTangents();
  }

  mesh->Upload();
  return mesh;
}

void GLTFLoader::LoadNode(
    const tinygltf::Model &model, int nodeIndex, int parentIndex,
    Ref<Model> &outModel, const std::string &directory,
    std::vector<Ref<Texture2D>> &textureCache,
    std::unordered_map<int32_t, Ref<PBRMaterial>> &materialCache) {
  const auto &gltfNode = model.nodes[nodeIndex];

  ModelNode node;
  node.Name = gltfNode.name.empty() ? ("Node_" + std::to_string(nodeIndex))
                                    : gltfNode.name;
  node.ParentIndex = parentIndex;

  // Parse transform
  if (!gltfNode.matrix.empty()) {
    glm::mat4 matrix = glm::make_mat4(gltfNode.matrix.data());

    glm::vec3 translation;
    glm::vec3 scale;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::quat rotation;
    glm::decompose(matrix, scale, rotation, translation, skew, perspective);

    node.LocalTransform.SetPosition(translation);
    node.LocalTransform.SetRotation(rotation);
    node.LocalTransform.SetScale(scale);
  } else {
    if (!gltfNode.translation.empty()) {
      node.LocalTransform.SetPosition(
          glm::vec3(static_cast<float>(gltfNode.translation[0]),
                    static_cast<float>(gltfNode.translation[1]),
                    static_cast<float>(gltfNode.translation[2])));
    }

    if (!gltfNode.rotation.empty()) {
      glm::quat rotation(static_cast<float>(gltfNode.rotation[3]),
                         static_cast<float>(gltfNode.rotation[0]),
                         static_cast<float>(gltfNode.rotation[1]),
                         static_cast<float>(gltfNode.rotation[2]));
      node.LocalTransform.SetRotation(rotation);
    }

    if (!gltfNode.scale.empty()) {
      node.LocalTransform.SetScale(
          glm::vec3(static_cast<float>(gltfNode.scale[0]),
                    static_cast<float>(gltfNode.scale[1]),
                    static_cast<float>(gltfNode.scale[2])));
    }
  }

  // Process mesh
  if (gltfNode.mesh >= 0) {
    const auto &gltfMesh = model.meshes[gltfNode.mesh];

    for (size_t primIdx = 0; primIdx < gltfMesh.primitives.size(); primIdx++) {
      const auto &primitive = gltfMesh.primitives[primIdx];

      if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1) {
        BE_CORE_WARN("Skipping non-triangle primitive (mode={})",
                     primitive.mode);
        continue;
      }

      auto mesh = LoadPrimitive(model, primitive);
      if (!mesh) {
        continue;
      }

      ModelMesh modelMesh;
      modelMesh.MeshData = mesh;
      modelMesh.MaterialIndex = primitive.material;
      modelMesh.Name = gltfMesh.name.empty()
                           ? ("Mesh_" + std::to_string(gltfNode.mesh) + "_" +
                              std::to_string(primIdx))
                           : gltfMesh.name + "_" + std::to_string(primIdx);

      // USE CACHED MATERIAL instead of creating new one!
      auto it = materialCache.find(primitive.material);
      if (it != materialCache.end()) {
        modelMesh.Material = it->second;
      } else {
        // Create and cache if not found
        auto mat =
            LoadMaterial(model, primitive.material, directory, textureCache);
        materialCache[primitive.material] = mat;
        modelMesh.Material = mat;
      }

      if (node.MeshIndex < 0) {
        node.MeshIndex = static_cast<int32_t>(outModel->m_Meshes.size());
      }

      outModel->m_Meshes.push_back(modelMesh);
    }
  }

  auto thisNodeIndex = static_cast<int32_t>(outModel->m_Nodes.size());
  outModel->m_Nodes.push_back(node);

  if (parentIndex >= 0) {
    outModel->m_Nodes[parentIndex].ChildrenIndices.push_back(thisNodeIndex);
  }

  for (int childIndex : gltfNode.children) {
    LoadNode(model, childIndex, thisNodeIndex, outModel, directory,
             textureCache, materialCache);
  }
}

bool GLTFLoader::IsSupported(const std::string &filepath) {
  return EndsWith(filepath, ".gltf") || EndsWith(filepath, ".glb");
}

Ref<Model> GLTFLoader::Load(const std::string &filepath) {
  if (!IsSupported(filepath)) {
    BE_CORE_ERROR("Unsupported model format: {}", filepath);
    return nullptr;
  }

  BE_CORE_INFO("Loading model: {}", filepath);

  tinygltf::Model gltfModel;
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;

  bool success = false;

  if (EndsWith(filepath, ".glb")) {
    success = loader.LoadBinaryFromFile(&gltfModel, &err, &warn, filepath);
  } else {
    success = loader.LoadASCIIFromFile(&gltfModel, &err, &warn, filepath);
  }

  if (!warn.empty()) {
    BE_CORE_WARN("glTF Warning: {}", warn);
  }

  if (!err.empty()) {
    BE_CORE_ERROR("glTF Error: {}", err);
  }

  if (!success) {
    BE_CORE_ERROR("Failed to load model: {}", filepath);
    return nullptr;
  }

  auto model = CreateRef<Model>();
  model->m_Filepath = filepath;
  model->m_Name = GetFilename(filepath);

  std::string directory = GetDirectory(filepath);
  std::vector<Ref<Texture2D>> textureCache;

  // NEW: Material cache to avoid creating duplicate materials
  std::unordered_map<int32_t, Ref<PBRMaterial>> materialCache;

  int sceneIndex = gltfModel.defaultScene >= 0 ? gltfModel.defaultScene : 0;

  if (sceneIndex >= 0 &&
      sceneIndex < static_cast<int>(gltfModel.scenes.size())) {
    const auto &scene = gltfModel.scenes[sceneIndex];

    for (int nodeIndex : scene.nodes) {

      LoadNode(gltfModel, nodeIndex, -1, model, directory, textureCache,
               materialCache);
    }
  } else if (!gltfModel.nodes.empty()) {
    for (size_t i = 0; i < gltfModel.nodes.size(); i++) {
      bool isRoot = true;
      for (const auto &node : gltfModel.nodes) {
        for (int childIdx : node.children) {
          if (childIdx == static_cast<int>(i)) {
            isRoot = false;
            break;
          }
        }
        if (!isRoot) {
          break;
        }
      }

      if (isRoot) {

        LoadNode(gltfModel, static_cast<int>(i), -1, model, directory,
                 textureCache, materialCache);
      }
    }
  }

  model->m_Textures = std::move(textureCache);

  // Copy materials from cache to model (no duplicates this way)
  for (const auto &[idx, mat] : materialCache) {
    if (mat) {
      model->m_Materials.push_back(mat);
    }
  }

  model->UpdateTransforms();
  model->CalculateBoundingBox();

  BE_CORE_INFO("Model loaded: {} meshes, {} nodes, {} materials, {} textures",
               model->GetMeshCount(), model->GetNodeCount(),
               model->GetMaterialCount(), model->GetTextureCount());

  BE_CORE_INFO("  Total: {} vertices, {} triangles",
               model->GetTotalVertexCount(), model->GetTotalTriangleCount());

  return model;
}

} // namespace BeEngine
