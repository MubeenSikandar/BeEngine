#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

SceneSerializer::SceneSerializer(Scene &scene) : m_Scene(scene) {}

// ============================================================================
// GLM Type Helpers
// ============================================================================

nlohmann::json SceneSerializer::Vec3ToJson(const glm::vec3 &v) {
  return nlohmann::json::array({v.x, v.y, v.z});
}

nlohmann::json SceneSerializer::Vec4ToJson(const glm::vec4 &v) {
  return nlohmann::json::array({v.x, v.y, v.z, v.w});
}

nlohmann::json SceneSerializer::QuatToJson(const glm::quat &q) {
  // Store as [x, y, z, w] (same as glm internal order)
  return nlohmann::json::array({q.x, q.y, q.z, q.w});
}

glm::vec3 SceneSerializer::JsonToVec3(const nlohmann::json &j) {
  if (!j.is_array() || j.size() < 3) {
    return glm::vec3(0.0F);
  }
  return {glm::vec3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>())};
}

glm::vec4 SceneSerializer::JsonToVec4(const nlohmann::json &j) {
  if (!j.is_array() || j.size() < 4) {
    return {glm::vec4(0.0F, 0.0F, 0.0F, 1.0F)};
  }
  return {glm::vec4(j[0].get<float>(), j[1].get<float>(), j[2].get<float>(),
                    j[3].get<float>())};
}

glm::quat SceneSerializer::JsonToQuat(const nlohmann::json &j) {
  if (!j.is_array() || j.size() < 4) {
    return {glm::quat(1.0F, 0.0F, 0.0F, 0.0F)};
  }
  // [x, y, z, w] -> glm::quat(w, x, y, z)
  return {glm::quat(j[3].get<float>(), j[0].get<float>(), j[1].get<float>(),
                    j[2].get<float>())};
}

// ============================================================================
// Path Utilities
// ============================================================================

std::string
SceneSerializer::ResolveAssetPath(const std::string &relativePath) const {
  if (relativePath.empty()) {
    return "";
  }

  // If already absolute, return as-is
  std::filesystem::path path(relativePath);
  if (path.is_absolute()) {
    return relativePath;
  }

  return m_AssetBasePath + relativePath;
}

std::string
SceneSerializer::MakeRelativePath(const std::string &absolutePath) const {
  if (absolutePath.empty()) {
    return "";
  }

  // Try to make relative to asset base path
  if (absolutePath.find(m_AssetBasePath) == 0) {
    return absolutePath.substr(m_AssetBasePath.length());
  }

  return absolutePath;
}

// ============================================================================
// Component Serialization
// ============================================================================

nlohmann::json
SceneSerializer::SerializeTransform(const TransformComponent &transform) {
  nlohmann::json j;
  j["position"] = Vec3ToJson(transform.GetPosition());
  j["rotation"] = QuatToJson(transform.GetRotation());
  j["scale"] = Vec3ToJson(transform.GetScale());
  return j;
}

nlohmann::json SceneSerializer::SerializeCamera(const CameraComponent &camera) {
  nlohmann::json j;
  j["projection"] =
      camera.Projection == CameraComponent::ProjectionType::Perspective
          ? "perspective"
          : "orthographic";
  j["fov"] = camera.FOV;
  j["nearClip"] = camera.NearClip;
  j["farClip"] = camera.FarClip;
  j["orthographicSize"] = camera.OrthographicSize;
  j["primary"] = camera.Primary;
  j["fixedAspectRatio"] = camera.FixedAspectRatio;
  return j;
}

nlohmann::json
SceneSerializer::SerializeMeshRenderer(const MeshRendererComponent &renderer) {
  nlohmann::json j;
  j["visible"] = renderer.Visible;
  j["castShadows"] = renderer.CastShadows;
  j["receiveShadows"] = renderer.ReceiveShadows;
  // TODO: Serialize mesh and material asset paths when AssetManager is
  // implemented j["mesh"] = renderer.MeshData ?
  // renderer.MeshData->GetAssetPath() : ""; j["material"] =
  // renderer.MaterialData ? renderer.MaterialData->GetAssetPath() : "";
  return j;
}

nlohmann::json SceneSerializer::SerializeModelRenderer(
    const ModelRendererComponent &renderer) {
  nlohmann::json j;
  j["visible"] = renderer.Visible;
  if (renderer.ModelData) {
    j["modelPath"] = MakeRelativePath(renderer.ModelData->GetFilepath());
  }
  return j;
}

nlohmann::json SceneSerializer::SerializeDirectionalLight(
    const DirectionalLightComponent &light) {
  nlohmann::json j;
  j["color"] = Vec3ToJson(light.Color);
  j["intensity"] = light.Intensity;
  j["castShadows"] = light.CastShadows;
  return j;
}

nlohmann::json
SceneSerializer::SerializePointLight(const PointLightComponent &light) {
  nlohmann::json j;
  j["color"] = Vec3ToJson(light.Color);
  j["intensity"] = light.Intensity;
  j["range"] = light.Range;
  j["castShadows"] = light.CastShadows;
  return j;
}

nlohmann::json
SceneSerializer::SerializeSpotLight(const SpotLightComponent &light) {
  nlohmann::json j;
  j["color"] = Vec3ToJson(light.Color);
  j["intensity"] = light.Intensity;
  j["range"] = light.Range;
  j["innerConeAngle"] = light.InnerConeAngle;
  j["outerConeAngle"] = light.OuterConeAngle;
  j["castShadows"] = light.CastShadows;
  return j;
}

nlohmann::json
SceneSerializer::SerializeRigidbody(const RigidbodyComponent &rb) {
  nlohmann::json j;

  std::string bodyType = "dynamic";
  switch (rb.Type) {
  case RigidbodyComponent::BodyType::Static:
    bodyType = "static";
    break;
  case RigidbodyComponent::BodyType::Kinematic:
    bodyType = "kinematic";
    break;
  default:
    break;
  }

  j["type"] = bodyType;
  j["mass"] = rb.Mass;
  j["linearDrag"] = rb.LinearDrag;
  j["angularDrag"] = rb.AngularDrag;
  j["useGravity"] = rb.UseGravity;
  j["isKinematic"] = rb.IsKinematic;
  return j;
}

nlohmann::json
SceneSerializer::SerializeBoxCollider(const BoxColliderComponent &collider) {
  nlohmann::json j;
  j["size"] = Vec3ToJson(collider.Size);
  j["offset"] = Vec3ToJson(collider.Offset);
  j["isTrigger"] = collider.IsTrigger;
  return j;
}

nlohmann::json SceneSerializer::SerializeSphereCollider(
    const SphereColliderComponent &collider) {
  nlohmann::json j;
  j["radius"] = collider.Radius;
  j["offset"] = Vec3ToJson(collider.Offset);
  j["isTrigger"] = collider.IsTrigger;
  return j;
}

nlohmann::json
SceneSerializer::SerializeNativeScript(const NativeScriptComponent &script) {
  nlohmann::json j;
  j["scriptName"] = script.ScriptName;
  // Note: Function pointers cannot be serialized - scripts must be re-bound on
  // load
  return j;
}

nlohmann::json
SceneSerializer::SerializeAudioSource(const AudioSourceComponent &audio) {
  nlohmann::json j;
  j["audioClipPath"] = MakeRelativePath(audio.AudioClipPath);
  j["volume"] = audio.Volume;
  j["pitch"] = audio.Pitch;
  j["loop"] = audio.Loop;
  j["playOnAwake"] = audio.PlayOnAwake;
  j["spatial"] = audio.Spatial;
  j["minDistance"] = audio.MinDistance;
  j["maxDistance"] = audio.MaxDistance;
  return j;
}

// ============================================================================
// Entity Serialization
// ============================================================================

nlohmann::json SceneSerializer::SerializeEntity(Entity entity) {
  nlohmann::json j;

  // Basic info
  auto &tag = entity.GetComponent<TagComponent>();
  j["uuid"] = entity.GetUUID();
  j["tag"] = tag.Tag;
  j["enabled"] = tag.Enabled;

  // Transform (always present)
  auto &transform = entity.GetComponent<TransformComponent>();
  j["transform"] = SerializeTransform(transform);

  // Optional components
  if (entity.HasComponent<CameraComponent>()) {
    j["camera"] = SerializeCamera(entity.GetComponent<CameraComponent>());
  }

  if (entity.HasComponent<MeshRendererComponent>()) {
    j["meshRenderer"] =
        SerializeMeshRenderer(entity.GetComponent<MeshRendererComponent>());
  }

  if (entity.HasComponent<ModelRendererComponent>()) {
    j["modelRenderer"] =
        SerializeModelRenderer(entity.GetComponent<ModelRendererComponent>());
  }

  if (entity.HasComponent<DirectionalLightComponent>()) {
    j["directionalLight"] = SerializeDirectionalLight(
        entity.GetComponent<DirectionalLightComponent>());
  }

  if (entity.HasComponent<PointLightComponent>()) {
    j["pointLight"] =
        SerializePointLight(entity.GetComponent<PointLightComponent>());
  }

  if (entity.HasComponent<SpotLightComponent>()) {
    j["spotLight"] =
        SerializeSpotLight(entity.GetComponent<SpotLightComponent>());
  }

  if (entity.HasComponent<RigidbodyComponent>()) {
    j["rigidbody"] =
        SerializeRigidbody(entity.GetComponent<RigidbodyComponent>());
  }

  if (entity.HasComponent<BoxColliderComponent>()) {
    j["boxCollider"] =
        SerializeBoxCollider(entity.GetComponent<BoxColliderComponent>());
  }

  if (entity.HasComponent<SphereColliderComponent>()) {
    j["sphereCollider"] =
        SerializeSphereCollider(entity.GetComponent<SphereColliderComponent>());
  }

  if (entity.HasComponent<NativeScriptComponent>()) {
    j["nativeScript"] =
        SerializeNativeScript(entity.GetComponent<NativeScriptComponent>());
  }

  if (entity.HasComponent<AudioSourceComponent>()) {
    j["audioSource"] =
        SerializeAudioSource(entity.GetComponent<AudioSourceComponent>());
  }

  // Children (recursive)
  auto children = m_Scene.GetChildren(entity);
  if (!children.empty()) {
    nlohmann::json childrenJson = nlohmann::json::array();
    for (Entity child : children) {
      childrenJson.push_back(SerializeEntity(child));
    }
    j["children"] = childrenJson;
  }

  return j;
}

// ============================================================================
// Scene Serialization
// ============================================================================

nlohmann::json SceneSerializer::SerializeToJson() {
  nlohmann::json root;

  // Header
  root["schemaVersion"] = SCHEMA_VERSION;
  root["scene"] = m_Scene.GetName();

  // Entities (only root entities - children are nested)
  nlohmann::json entities = nlohmann::json::array();
  for (Entity entity : m_Scene.GetRootEntities()) {
    entities.push_back(SerializeEntity(entity));
  }
  root["entities"] = entities;

  // Metadata
  root["metadata"] = {
      {"entityCount", m_Scene.GetEntityCount()},
      {"timestamp",
       std::chrono::system_clock::now().time_since_epoch().count()}};

  return root;
}

std::string SceneSerializer::SerializeToString(bool pretty) {
  nlohmann::json j = SerializeToJson();
  return pretty ? j.dump(2) : j.dump();
}

bool SceneSerializer::Serialize(const std::string &filepath, Format format) {
  try {
    nlohmann::json j = SerializeToJson();

    std::ofstream file;

    if (format == Format::Binary) {
      file.open(filepath, std::ios::binary);
      if (!file.is_open()) {
        BE_CORE_ERROR("Could not open file for writing: {}", filepath);
        return false;
      }

      // Write as MessagePack
      std::vector<uint8_t> msgpack = nlohmann::json::to_msgpack(j);
      file.write(reinterpret_cast<const char *>(msgpack.data()),
                 static_cast<std::streamsize>(msgpack.size()));
    } else {
      file.open(filepath);
      if (!file.is_open()) {
        BE_CORE_ERROR("Could not open file for writing: {}", filepath);
        return false;
      }

      // Write as JSON
      int indent = (format == Format::JSON) ? 2 : -1;
      file << j.dump(indent);
    }

    file.close();
    BE_CORE_INFO("Scene '{}' serialized to: {}", m_Scene.GetName(), filepath);
    return true;

  } catch (const std::exception &e) {
    BE_CORE_ERROR("Failed to serialize scene: {}", e.what());
    return false;
  }
}

// ============================================================================
// Prefab Serialization
// ============================================================================

nlohmann::json SceneSerializer::SerializeEntityToPrefab(Entity entity) {
  nlohmann::json prefab;
  prefab["schemaVersion"] = SCHEMA_VERSION;
  prefab["type"] = "prefab";
  prefab["entity"] = SerializeEntity(entity);
  return prefab;
}

bool SceneSerializer::SavePrefab(Entity entity, const std::string &filepath) {
  try {
    nlohmann::json prefab = SerializeEntityToPrefab(entity);

    std::ofstream file(filepath);
    if (!file.is_open()) {
      BE_CORE_ERROR("Could not open prefab file for writing: {}", filepath);
      return false;
    }

    file << prefab.dump(2);
    file.close();

    auto &tag = entity.GetComponent<TagComponent>();
    BE_CORE_INFO("Prefab '{}' saved to: {}", tag.Tag, filepath);
    return true;

  } catch (const std::exception &e) {
    BE_CORE_ERROR("Failed to save prefab: {}", e.what());
    return false;
  }
}

nlohmann::json SceneSerializer::LoadPrefab(const std::string &filepath) {
  try {
    std::ifstream file(filepath);
    if (!file.is_open()) {
      BE_CORE_ERROR("Could not open prefab file: {}", filepath);
      return {};
    }

    nlohmann::json prefab = nlohmann::json::parse(file);
    file.close();

    if (!prefab.contains("type") || prefab["type"] != "prefab") {
      BE_CORE_ERROR("Invalid prefab file: {}", filepath);
      return {};
    }

    return prefab;

  } catch (const std::exception &e) {
    BE_CORE_ERROR("Failed to load prefab: {}", e.what());
    return {};
  }
}

} // namespace BeEngine
