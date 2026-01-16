// SceneSerializer.hpp
#pragma once

#include <Core.hpp>
#include <Scene/Scene.hpp>
#include <json.hpp>

namespace BeEngine {

/**
 * @brief scene serializer using nlohmann/json
 *
 * Features:
 * - Full JSON serialization/deserialization
 * - Entity hierarchy preservation
 * - Asset path resolution
 * - Schema versioning for backwards compatibility
 * - Binary format support (MessagePack)
 * - Pretty-print or compact output
 *
 */
class BE_API SceneSerializer {
public:
  // Current schema version - increment when format changes
  static constexpr uint32_t SCHEMA_VERSION = 1;

  /**
   * @brief Output format options
   */
  enum class Format : uint8_t {
    JSON,        // Human-readable JSON (default)
    JSONCompact, // Minified JSON (smaller file size)
    Binary       // MessagePack binary format (fastest load/save)
  };

  explicit SceneSerializer(Scene &scene);
  ~SceneSerializer() = default;

  // ===== Serialization =====

  /**
   * @brief Serialize scene to file
   * @param filepath Output file path (.scene, .json, or .bin)
   * @param format Output format
   * @return True on success
   */
  bool Serialize(const std::string &filepath, Format format = Format::JSON);

  /**
   * @brief Serialize scene to JSON string
   * @param pretty Use pretty-printing (default: true)
   * @return JSON string
   */
  NODISCARD std::string SerializeToString(bool pretty = true);

  /**
   * @brief Serialize scene to JSON object
   * @return nlohmann::json object
   */
  NODISCARD nlohmann::json SerializeToJson();

  // ===== Deserialization =====

  /**
   * @brief Deserialize scene from file
   * @param filepath Input file path
   * @return True on success
   */
  bool Deserialize(const std::string &filepath);

  /**
   * @brief Deserialize scene from JSON string
   * @param jsonStr JSON string
   * @return True on success
   */
  bool DeserializeFromString(const std::string &jsonStr);

  /**
   * @brief Deserialize scene from JSON object
   * @param json nlohmann::json object
   * @return True on success
   */
  bool DeserializeFromJson(const nlohmann::json &json);

  // ===== Prefabs =====

  /**
   * @brief Serialize a single entity (and children) as a prefab
   * @param entity Entity to serialize
   * @return JSON prefab data
   */
  NODISCARD nlohmann::json SerializeEntityToPrefab(Entity entity);

  /**
   * @brief Save prefab to file
   */
  bool SavePrefab(Entity entity, const std::string &filepath);

  /**
   * @brief Load prefab from file
   */
  NODISCARD nlohmann::json LoadPrefab(const std::string &filepath);

  /**
   * @brief Instantiate an entity from a prefab
   * @param prefab JSON prefab data
   * @param parent Optional parent entity
   * @return The instantiated root entity
   */
  Entity InstantiatePrefab(const nlohmann::json &prefab,
                           Entity parent = Entity());

  // ===== Configuration =====

  /**
   * @brief Set asset base path for resolving relative paths
   */
  void SetAssetBasePath(const std::string &path) { m_AssetBasePath = path; }

  /**
   * @brief Get current asset base path
   */
  NODISCARD const std::string &GetAssetBasePath() const {
    return m_AssetBasePath;
  }

  // Validation
  NODISCARD bool ValidateJson(const nlohmann::json &json) const;

  // Migration for old schema versions
  bool MigrateSchema(nlohmann::json &json, uint32_t fromVersion);

  // Progress callback for large scenes
  using ProgressCallback =
      std::function<void(float progress, const std::string &status)>;
  void SetProgressCallback(ProgressCallback callback);

private:
  // ===== Entity Serialization =====
  nlohmann::json SerializeEntity(Entity entity);

  // ===== Component Serialization =====
  nlohmann::json SerializeTransform(const TransformComponent &transform);
  nlohmann::json SerializeCamera(const CameraComponent &camera);
  nlohmann::json SerializeMeshRenderer(const MeshRendererComponent &renderer);
  nlohmann::json SerializeModelRenderer(const ModelRendererComponent &renderer);
  nlohmann::json
  SerializeDirectionalLight(const DirectionalLightComponent &light);
  nlohmann::json SerializePointLight(const PointLightComponent &light);
  nlohmann::json SerializeSpotLight(const SpotLightComponent &light);
  nlohmann::json SerializeRigidbody(const RigidbodyComponent &rb);
  nlohmann::json SerializeBoxCollider(const BoxColliderComponent &collider);
  nlohmann::json
  SerializeSphereCollider(const SphereColliderComponent &collider);
  nlohmann::json SerializeNativeScript(const NativeScriptComponent &script);
  nlohmann::json SerializeAudioSource(const AudioSourceComponent &audio);

  // ===== Entity Deserialization =====
  Entity DeserializeEntity(const nlohmann::json &json,
                           Entity parent = Entity());

  // ===== Component Deserialization =====
  void DeserializeTransform(const nlohmann::json &json,
                            TransformComponent &transform);
  void DeserializeCamera(const nlohmann::json &json, CameraComponent &camera);
  void DeserializeMeshRenderer(const nlohmann::json &json,
                               MeshRendererComponent &renderer);
  void DeserializeModelRenderer(const nlohmann::json &json,
                                ModelRendererComponent &renderer);
  void DeserializeDirectionalLight(const nlohmann::json &json,
                                   DirectionalLightComponent &light);
  void DeserializePointLight(const nlohmann::json &json,
                             PointLightComponent &light);
  void DeserializeSpotLight(const nlohmann::json &json,
                            SpotLightComponent &light);
  void DeserializeRigidbody(const nlohmann::json &json, RigidbodyComponent &rb);
  void DeserializeBoxCollider(const nlohmann::json &json,
                              BoxColliderComponent &collider);
  void DeserializeSphereCollider(const nlohmann::json &json,
                                 SphereColliderComponent &collider);
  void DeserializeNativeScript(const nlohmann::json &json,
                               NativeScriptComponent &script);
  void DeserializeAudioSource(const nlohmann::json &json,
                              AudioSourceComponent &audio);

  // ===== GLM Type Helpers =====
  static nlohmann::json Vec3ToJson(const glm::vec3 &v);
  static nlohmann::json Vec4ToJson(const glm::vec4 &v);
  static nlohmann::json QuatToJson(const glm::quat &q);
  static glm::vec3 JsonToVec3(const nlohmann::json &j);
  static glm::vec4 JsonToVec4(const nlohmann::json &j);
  static glm::quat JsonToQuat(const nlohmann::json &j);

  // ===== Path Utilities =====
  NODISCARD std::string ResolveAssetPath(const std::string &relativePath) const;
  NODISCARD std::string MakeRelativePath(const std::string &absolutePath) const;

  Scene &m_Scene;
  std::string m_AssetBasePath = "Assets/";

  ProgressCallback m_ProgressCallback;
};
} // namespace BeEngine
