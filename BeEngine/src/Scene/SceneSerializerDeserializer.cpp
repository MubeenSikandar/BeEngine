#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

// ============================================================================
// Component Deserialization
// ============================================================================

void SceneSerializer::DeserializeTransform(const nlohmann::json &j,
                                           TransformComponent &transform) {
  if (j.contains("position")) {
    transform.SetPosition(JsonToVec3(j["position"]));
  }
  if (j.contains("rotation")) {
    transform.SetRotation(JsonToQuat(j["rotation"]));
  }
  if (j.contains("scale")) {
    transform.SetScale(JsonToVec3(j["scale"]));
  }
}

void SceneSerializer::DeserializeCamera(const nlohmann::json &j,
                                        CameraComponent &camera) {
  if (j.contains("projection")) {
    std::string proj = j["projection"].get<std::string>();
    camera.Projection = (proj == "perspective")
                            ? CameraComponent::ProjectionType::Perspective
                            : CameraComponent::ProjectionType::Orthographic;
  }
  if (j.contains("fov")) {
    camera.FOV = j["fov"].get<float>();
  }
  if (j.contains("nearClip")) {
    camera.NearClip = j["nearClip"].get<float>();
  }
  if (j.contains("farClip")) {
    camera.FarClip = j["farClip"].get<float>();
  }
  if (j.contains("orthographicSize")) {
    camera.OrthographicSize = j["orthographicSize"].get<float>();
  }
  if (j.contains("primary")) {
    camera.Primary = j["primary"].get<bool>();
  }
  if (j.contains("fixedAspectRatio")) {
    camera.FixedAspectRatio = j["fixedAspectRatio"].get<bool>();
  }
}

void SceneSerializer::DeserializeMeshRenderer(const nlohmann::json &j,
                                              MeshRendererComponent &renderer) {
  if (j.contains("visible")) {
    renderer.Visible = j["visible"].get<bool>();
  }
  if (j.contains("castShadows")) {
    renderer.CastShadows = j["castShadows"].get<bool>();
  }
  if (j.contains("receiveShadows")) {
    renderer.ReceiveShadows = j["receiveShadows"].get<bool>();
    // TODO: Load mesh and material from AssetManager when implemented}
  }
}

void SceneSerializer::DeserializeModelRenderer(
    const nlohmann::json &j, ModelRendererComponent &renderer) {
  if (j.contains("visible")) {
    renderer.Visible = j["visible"].get<bool>();
  }
  if (j.contains("modelPath")) {
    std::string modelPath = j["modelPath"].get<std::string>();
    if (!modelPath.empty()) {
      std::string fullPath = ResolveAssetPath(modelPath);
      renderer.ModelData = Model::Load(fullPath);
      if (!renderer.ModelData || !renderer.ModelData->IsValid()) {
        BE_CORE_WARN("Failed to load model: {}", fullPath);
      }
    }
  }
}

void SceneSerializer::DeserializeDirectionalLight(
    const nlohmann::json &j, DirectionalLightComponent &light) {
  if (j.contains("color")) {
    light.Color = JsonToVec3(j["color"]);
  }
  if (j.contains("intensity")) {
    light.Intensity = j["intensity"].get<float>();
  }
  if (j.contains("castShadows")) {
    light.CastShadows = j["castShadows"].get<bool>();
  }
}

void SceneSerializer::DeserializePointLight(const nlohmann::json &j,
                                            PointLightComponent &light) {
  if (j.contains("color")) {
    light.Color = JsonToVec3(j["color"]);
  }
  if (j.contains("intensity")) {
    light.Intensity = j["intensity"].get<float>();
  }
  if (j.contains("range")) {
    light.Range = j["range"].get<float>();
  }
  if (j.contains("castShadows")) {
    light.CastShadows = j["castShadows"].get<bool>();
  }
}

void SceneSerializer::DeserializeSpotLight(const nlohmann::json &j,
                                           SpotLightComponent &light) {
  if (j.contains("color")) {
    light.Color = JsonToVec3(j["color"]);
  }
  if (j.contains("intensity")) {
    light.Intensity = j["intensity"].get<float>();
  }
  if (j.contains("range")) {
    light.Range = j["range"].get<float>();
  }
  if (j.contains("innerConeAngle")) {
    light.InnerConeAngle = j["innerConeAngle"].get<float>();
  }
  if (j.contains("outerConeAngle")) {
    light.OuterConeAngle = j["outerConeAngle"].get<float>();
  }
  if (j.contains("castShadows")) {
    light.CastShadows = j["castShadows"].get<bool>();
  }
}

void SceneSerializer::DeserializeRigidbody(const nlohmann::json &j,
                                           RigidbodyComponent &rb) {
  if (j.contains("type")) {
    std::string type = j["type"].get<std::string>();
    if (type == "static") {
      rb.Type = RigidbodyComponent::BodyType::Static;
    } else if (type == "kinematic") {
      rb.Type = RigidbodyComponent::BodyType::Kinematic;
    } else {
      rb.Type = RigidbodyComponent::BodyType::Dynamic;
    }
  }
  if (j.contains("mass")) {
    rb.Mass = j["mass"].get<float>();
  }
  if (j.contains("linearDrag")) {
    rb.LinearDrag = j["linearDrag"].get<float>();
  }
  if (j.contains("angularDrag")) {
    rb.AngularDrag = j["angularDrag"].get<float>();
  }
  if (j.contains("useGravity")) {
    rb.UseGravity = j["useGravity"].get<bool>();
  }
  if (j.contains("isKinematic")) {
    rb.IsKinematic = j["isKinematic"].get<bool>();
  }
}

void SceneSerializer::DeserializeBoxCollider(const nlohmann::json &j,
                                             BoxColliderComponent &collider) {
  if (j.contains("size")) {
    collider.Size = JsonToVec3(j["size"]);
  }
  if (j.contains("offset")) {
    collider.Offset = JsonToVec3(j["offset"]);
  }
  if (j.contains("isTrigger")) {
    collider.IsTrigger = j["isTrigger"].get<bool>();
  }
}

void SceneSerializer::DeserializeSphereCollider(
    const nlohmann::json &j, SphereColliderComponent &collider) {
  if (j.contains("radius")) {
    collider.Radius = j["radius"].get<float>();
  }
  if (j.contains("offset")) {
    collider.Offset = JsonToVec3(j["offset"]);
  }
  if (j.contains("isTrigger")) {
    collider.IsTrigger = j["isTrigger"].get<bool>();
  }
}

void SceneSerializer::DeserializeNativeScript(const nlohmann::json &j,
                                              NativeScriptComponent &script) {
  if (j.contains("scriptName")) {
    script.ScriptName = j["scriptName"].get<std::string>();
  }
  // Note: Script callbacks must be re-bound programmatically after loading
}

void SceneSerializer::DeserializeAudioSource(const nlohmann::json &j,
                                             AudioSourceComponent &audio) {
  if (j.contains("audioClipPath")) {
    audio.AudioClipPath =
        ResolveAssetPath(j["audioClipPath"].get<std::string>());
  }
  if (j.contains("volume")) {
    audio.Volume = j["volume"].get<float>();
  }
  if (j.contains("pitch")) {
    audio.Pitch = j["pitch"].get<float>();
  }
  if (j.contains("loop")) {
    audio.Loop = j["loop"].get<bool>();
  }
  if (j.contains("playOnAwake")) {
    audio.PlayOnAwake = j["playOnAwake"].get<bool>();
  }
  if (j.contains("spatial")) {
    audio.Spatial = j["spatial"].get<bool>();
  }
  if (j.contains("minDistance")) {
    audio.MinDistance = j["minDistance"].get<float>();
  }
  if (j.contains("maxDistance")) {
    audio.MaxDistance = j["maxDistance"].get<float>();
  }
}

// ============================================================================
// Entity Deserialization
// ============================================================================

Entity SceneSerializer::DeserializeEntity(const nlohmann::json &j,
                                          Entity parent) {
  // Get entity name
  std::string tag = j.value("tag", "Entity");
  bool enabled = j.value("enabled", true);

  // Create entity (optionally with specific UUID for deterministic loading)
  Entity entity;
  if (j.contains("uuid")) {
    uint64_t uuid = j["uuid"].get<uint64_t>();
    entity = m_Scene.CreateEntityWithUUID(uuid, tag);
  } else {
    entity = m_Scene.CreateEntity(tag);
  }

  // Set enabled state
  entity.GetComponent<TagComponent>().Enabled = enabled;

  // Transform (always present)
  if (j.contains("transform")) {
    DeserializeTransform(j["transform"],
                         entity.GetComponent<TransformComponent>());
  }

  // Set parent if provided
  if (parent.IsValid()) {
    m_Scene.SetParent(entity, parent);
  }

  // Optional components
  if (j.contains("camera")) {
    auto &camera = entity.AddComponent<CameraComponent>();
    DeserializeCamera(j["camera"], camera);
  }

  if (j.contains("meshRenderer")) {
    auto &renderer = entity.AddComponent<MeshRendererComponent>();
    DeserializeMeshRenderer(j["meshRenderer"], renderer);
  }

  if (j.contains("modelRenderer")) {
    auto &renderer = entity.AddComponent<ModelRendererComponent>();
    DeserializeModelRenderer(j["modelRenderer"], renderer);
  }

  if (j.contains("directionalLight")) {
    auto &light = entity.AddComponent<DirectionalLightComponent>();
    DeserializeDirectionalLight(j["directionalLight"], light);
  }

  if (j.contains("pointLight")) {
    auto &light = entity.AddComponent<PointLightComponent>();
    DeserializePointLight(j["pointLight"], light);
  }

  if (j.contains("spotLight")) {
    auto &light = entity.AddComponent<SpotLightComponent>();
    DeserializeSpotLight(j["spotLight"], light);
  }

  if (j.contains("rigidbody")) {
    auto &rb = entity.AddComponent<RigidbodyComponent>();
    DeserializeRigidbody(j["rigidbody"], rb);
  }

  if (j.contains("boxCollider")) {
    auto &collider = entity.AddComponent<BoxColliderComponent>();
    DeserializeBoxCollider(j["boxCollider"], collider);
  }

  if (j.contains("sphereCollider")) {
    auto &collider = entity.AddComponent<SphereColliderComponent>();
    DeserializeSphereCollider(j["sphereCollider"], collider);
  }

  if (j.contains("nativeScript")) {
    auto &script = entity.AddComponent<NativeScriptComponent>();
    DeserializeNativeScript(j["nativeScript"], script);
  }

  if (j.contains("audioSource")) {
    auto &audio = entity.AddComponent<AudioSourceComponent>();
    DeserializeAudioSource(j["audioSource"], audio);
  }

  // Deserialize children recursively
  if (j.contains("children") && j["children"].is_array()) {
    for (const auto &childJson : j["children"]) {
      DeserializeEntity(childJson, entity);
    }
  }

  return entity;
}

// ============================================================================
// Scene Deserialization
// ============================================================================

bool SceneSerializer::DeserializeFromJson(const nlohmann::json &json) {
  try {
    // Check schema version
    uint32_t version = json.value("schemaVersion", 1u);
    if (version > SCHEMA_VERSION) {
      BE_CORE_WARN(
          "Scene file has newer schema version ({}) than supported ({})",
          version, SCHEMA_VERSION);
    }

    // Scene name
    if (json.contains("scene")) {
      m_Scene.SetName(json["scene"].get<std::string>());
    }

    // Deserialize entities
    if (json.contains("entities") && json["entities"].is_array()) {
      for (const auto &entityJson : json["entities"]) {
        DeserializeEntity(entityJson);
      }
    }

    // Update transforms after all entities are loaded
    m_Scene.UpdateTransforms();

    BE_CORE_INFO("Scene '{}' deserialized: {} entities", m_Scene.GetName(),
                 m_Scene.GetEntityCount());
    return true;

  } catch (const std::exception &e) {
    BE_CORE_ERROR("Failed to deserialize scene: {}", e.what());
    return false;
  }
}

bool SceneSerializer::DeserializeFromString(const std::string &jsonStr) {
  try {
    nlohmann::json j = nlohmann::json::parse(jsonStr);
    return DeserializeFromJson(j);
  } catch (const std::exception &e) {
    BE_CORE_ERROR("Failed to parse JSON: {}", e.what());
    return false;
  }
}

bool SceneSerializer::Deserialize(const std::string &filepath) {
  try {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
      BE_CORE_ERROR("Could not open file for reading: {}", filepath);
      return false;
    }

    // Check if binary (MessagePack) or JSON
    // MessagePack files typically start with specific bytes
    char firstByte;
    file.get(firstByte);
    file.seekg(0);

    nlohmann::json j;

    // Simple heuristic: JSON starts with '{' or whitespace
    if (firstByte == '{' || firstByte == ' ' || firstByte == '\n' ||
        firstByte == '\r' || firstByte == '\t') {
      // Parse as JSON
      j = nlohmann::json::parse(file);
    } else {
      // Try to parse as MessagePack
      std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
      j = nlohmann::json::from_msgpack(data);
    }

    file.close();

    // Set asset base path from file location
    std::filesystem::path filePath(filepath);
    m_AssetBasePath = filePath.parent_path().string() + "/";

    return DeserializeFromJson(j);

  } catch (const std::exception &e) {
    BE_CORE_ERROR("Failed to deserialize scene from '{}': {}", filepath,
                  e.what());
    return false;
  }
}

// ============================================================================
// Prefab Instantiation
// ============================================================================

Entity SceneSerializer::InstantiatePrefab(const nlohmann::json &prefab,
                                          Entity parent) {
  if (!prefab.contains("entity")) {
    BE_CORE_ERROR("Invalid prefab: missing 'entity' field");
    return {Entity()};
  }

  // Don't use UUIDs from prefab - generate new ones for each instance
  nlohmann::json entityJson = prefab["entity"];

  // Remove UUIDs recursively to force new ID generation
  std::function<void(nlohmann::json &)> removeUUIDs = [&](nlohmann::json &j) {
    j.erase("uuid");
    if (j.contains("children") && j["children"].is_array()) {
      for (auto &child : j["children"]) {
        removeUUIDs(child);
      }
    }
  };
  removeUUIDs(entityJson);

  return DeserializeEntity(entityJson, parent);
}

} // namespace BeEngine
