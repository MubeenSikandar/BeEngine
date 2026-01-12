#pragma once

#include <Core.hpp>
#include <MaterialSystem/Material.hpp>
#include <Math/Transform.hpp>
#include <MeshSystem/Mesh.hpp>
#include <ModelLoader/Model.hpp>
#include <Scene/ECS/Entity.hpp>
#include <cstdint>
#include <typeindex>
#include <vector>

namespace BeEngine {
// ============================================================================
// Component Type System
// ============================================================================

/**
 * @brief Unique identifier for component types
 */
using ComponentTypeID = std::type_index;

/**
 * @brief Get the type ID for a component type
 */
template <typename T> inline ComponentTypeID GetComponentTypeID() {
  return std::type_index(typeid(T));
}

/**
 * @brief Base interface for component pools (type-erased storage)
 */
class IComponentPool {
public:
  virtual ~IComponentPool() = default;
  virtual void Remove(uint32_t index) = 0;
  virtual void Clear() = 0;
  NODISCARD virtual size_t Size() const = 0;
  NODISCARD virtual bool Has(uint32_t index) const = 0;
  virtual void *GetRaw(uint32_t index) = 0;
  NODISCARD virtual const void *GetRaw(uint32_t index) const = 0;
};

/**
 * @brief Type-safe component pool that stores components contiguously
 */
template <typename T> class ComponentPool : public IComponentPool {
public:
  ComponentPool() { m_Components.reserve(256); }

  template <typename... Args> T &Add(uint32_t entityIndex, Args &&...args) {
    // Ensure we have space
    if (entityIndex >= m_EntityToComponent.size()) {
      m_EntityToComponent.resize(entityIndex + 1, INVALID_INDEX);
    }

    // Check if already exists
    if (m_EntityToComponent[entityIndex] != INVALID_INDEX) {
      return m_Components[m_EntityToComponent[entityIndex]];
    }

    // Add new component
    auto componentIndex = static_cast<uint32_t>(m_Components.size());
    m_Components.emplace_back(std::forward<Args>(args)...);
    m_ComponentToEntity.push_back(entityIndex);
    m_EntityToComponent[entityIndex] = componentIndex;

    return m_Components.back();
  }

  void Remove(uint32_t entityIndex) override {
    if (entityIndex >= m_EntityToComponent.size()) {
      return;
    }

    uint32_t componentIndex = m_EntityToComponent[entityIndex];
    if (componentIndex == INVALID_INDEX) {
      return;
    }

    // Swap with last element for O(1) removal
    auto lastComponentIndex = static_cast<uint32_t>(m_Components.size() - 1);
    if (componentIndex != lastComponentIndex) {
      // Move last component to this slot
      m_Components[componentIndex] =
          std::move(m_Components[lastComponentIndex]);

      // Update the entity that owned the last component
      uint32_t movedEntity = m_ComponentToEntity[lastComponentIndex];
      m_ComponentToEntity[componentIndex] = movedEntity;
      m_EntityToComponent[movedEntity] = componentIndex;
    }

    // Remove last element
    m_Components.pop_back();
    m_ComponentToEntity.pop_back();
    m_EntityToComponent[entityIndex] = INVALID_INDEX;
  }

  void Clear() override {
    m_Components.clear();
    m_ComponentToEntity.clear();
    m_EntityToComponent.clear();
  }

  NODISCARD size_t Size() const override { return m_Components.size(); }

  NODISCARD bool Has(uint32_t entityIndex) const override {
    return entityIndex < m_EntityToComponent.size() &&
           m_EntityToComponent[entityIndex] != INVALID_INDEX;
  }

  NODISCARD T &Get(uint32_t entityIndex) {
    return m_Components[m_EntityToComponent[entityIndex]];
  }

  NODISCARD const T &Get(uint32_t entityIndex) const {
    return m_Components[m_EntityToComponent[entityIndex]];
  }

  NODISCARD T *TryGet(uint32_t entityIndex) {
    if (!Has(entityIndex)) {
      return nullptr;
    }
    return &m_Components[m_EntityToComponent[entityIndex]];
  }

  NODISCARD const T *TryGet(uint32_t entityIndex) const {
    if (!Has(entityIndex)) {
      return nullptr;
    }
    return &m_Components[m_EntityToComponent[entityIndex]];
  }

  void *GetRaw(uint32_t entityIndex) override {
    if (!Has(entityIndex)) {
      return nullptr;
    }
    return &m_Components[m_EntityToComponent[entityIndex]];
  }

  NODISCARD const void *GetRaw(uint32_t entityIndex) const override {
    if (!Has(entityIndex)) {
      return nullptr;
    }
    return &m_Components[m_EntityToComponent[entityIndex]];
  }

  // Iteration
  NODISCARD std::vector<T> &GetAll() { return m_Components; }
  NODISCARD const std::vector<T> &GetAll() const { return m_Components; }

  // Get entity index for a component index
  NODISCARD uint32_t GetEntityIndex(uint32_t componentIndex) const {
    return m_ComponentToEntity[componentIndex];
  }

private:
  static constexpr uint32_t INVALID_INDEX =
      std::numeric_limits<uint32_t>::max();

  std::vector<T> m_Components;
  std::vector<uint32_t> m_ComponentToEntity; // Component index -> Entity index
  std::vector<uint32_t> m_EntityToComponent; // Entity index -> Component index
};

// ============================================================================
// Built-in Components
// ============================================================================

/**
 * @brief Tag component for entity identification
 */
struct BE_API TagComponent {
  std::string Tag = "Entity";
  bool Enabled = true;

  TagComponent() = default;
  explicit TagComponent(std::string tag) : Tag(std::move(tag)) {}
};

/**
 * @brief Transform component - position, rotation, scale
 */
struct BE_API TransformComponent {
  Transform LocalTransform;

  // Cached world transform (updated by TransformSystem)
  glm::mat4 WorldMatrix = glm::mat4(1.0F);

  // Hierarchy
  EntityID Parent;
  std::vector<EntityID> Children;

  TransformComponent() = default;
  explicit TransformComponent(const glm::vec3 &position) {
    LocalTransform.SetPosition(position);
  }
  TransformComponent(const glm::vec3 &position, const glm::vec3 &rotation,
                     const glm::vec3 &scale) {
    LocalTransform.SetPosition(position);
    LocalTransform.SetRotation(rotation);
    LocalTransform.SetScale(scale);
  }

  // Convenience accessors
  NODISCARD glm::vec3 GetPosition() const {
    return LocalTransform.GetPosition();
  }
  NODISCARD glm::quat GetRotation() const {
    return LocalTransform.GetRotation();
  }
  NODISCARD glm::vec3 GetScale() const { return LocalTransform.GetScale(); }

  void SetPosition(const glm::vec3 &pos) { LocalTransform.SetPosition(pos); }
  void SetRotation(const glm::vec3 &euler) {
    LocalTransform.SetRotation(euler);
  }
  void SetRotation(const glm::quat &quat) { LocalTransform.SetRotation(quat); }
  void SetScale(const glm::vec3 &scale) { LocalTransform.SetScale(scale); }
  void SetScale(float uniform) { LocalTransform.SetScale(uniform); }
};

/**
 * @brief Mesh renderer component
 */
struct BE_API MeshRendererComponent {
  Ref<Mesh> MeshData;
  Ref<Material> MaterialData;

  bool CastShadows = true;
  bool ReceiveShadows = true;
  bool Visible = true;

  MeshRendererComponent() = default;
  MeshRendererComponent(Ref<Mesh> mesh, Ref<Material> material)
      : MeshData(std::move(mesh)), MaterialData(std::move(material)) {}
};

/**
 * @brief Model renderer component (for loaded glTF/FBX models)
 */
struct BE_API ModelRendererComponent {
  Ref<Model> ModelData;
  bool Visible = true;

  ModelRendererComponent() = default;
  explicit ModelRendererComponent(Ref<Model> model)
      : ModelData(std::move(model)) {}
};

/**
 * @brief Camera component
 */
struct BE_API CameraComponent {
  enum class ProjectionType : uint8_t { Perspective, Orthographic };

  ProjectionType Projection = ProjectionType::Perspective;

  // Perspective settings
  float FOV = 45.0F;
  float NearClip = 0.1F;
  float FarClip = 1000.0F;

  // Orthographic settings
  float OrthographicSize = 10.0F;

  // Common
  float AspectRatio = 16.0F / 9.0F;
  bool Primary = false; // Is this the main camera?
  bool FixedAspectRatio = false;

  // Computed matrices (updated by CameraSystem)
  glm::mat4 ViewMatrix = glm::mat4(1.0F);
  glm::mat4 ProjectionMatrix = glm::mat4(1.0F);
  glm::mat4 ViewProjectionMatrix = glm::mat4(1.0F);

  CameraComponent() = default;

  NODISCARD glm::mat4 GetProjectionMatrix() const {
    if (Projection == ProjectionType::Perspective) {
      return glm::perspective(glm::radians(FOV), AspectRatio, NearClip,
                              FarClip);
    }
    float orthoLeft = -OrthographicSize * AspectRatio * 0.5F;
    float orthoRight = OrthographicSize * AspectRatio * 0.5F;
    float orthoBottom = -OrthographicSize * 0.5F;
    float orthoTop = OrthographicSize * 0.5F;
    return glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, NearClip,
                      FarClip);
  }
};

/**
 * @brief Directional light component
 */
struct BE_API DirectionalLightComponent {
  glm::vec3 Color = glm::vec3(1.0F);
  float Intensity = 1.0F;
  bool CastShadows = true;

  DirectionalLightComponent() = default;
  DirectionalLightComponent(const glm::vec3 &color, float intensity)
      : Color(color), Intensity(intensity) {}
};

/**
 * @brief Point light component
 */
struct BE_API PointLightComponent {
  glm::vec3 Color = glm::vec3(1.0F);
  float Intensity = 1.0F;
  float Range = 10.0F;
  bool CastShadows = false;

  PointLightComponent() = default;
  PointLightComponent(const glm::vec3 &color, float intensity, float range)
      : Color(color), Intensity(intensity), Range(range) {}
};

/**
 * @brief Spot light component
 */
struct BE_API SpotLightComponent {
  glm::vec3 Color = glm::vec3(1.0F);
  float Intensity = 1.0F;
  float Range = 10.0F;
  float InnerConeAngle = 12.5F; // Degrees
  float OuterConeAngle = 17.5F; // Degrees
  bool CastShadows = false;

  SpotLightComponent() = default;
};

/**
 * @brief Script component for native C++ scripts
 */
struct BE_API NativeScriptComponent {
  std::string ScriptName;

  // Function pointers for script callbacks
  std::function<void()> OnCreate;
  std::function<void()> OnDestroy;
  std::function<void(float)> OnUpdate;

  NativeScriptComponent() = default;
  explicit NativeScriptComponent(std::string name)
      : ScriptName(std::move(name)) {}
};

/**
 * @brief Rigidbody component (for future physics)
 */
struct BE_API RigidbodyComponent {
  enum class BodyType : uint8_t { Static, Dynamic, Kinematic };

  BodyType Type = BodyType::Dynamic;
  float Mass = 1.0F;
  float LinearDrag = 0.0F;
  float AngularDrag = 0.05F;
  bool UseGravity = true;
  bool IsKinematic = false;

  // Runtime physics state (managed by physics engine)
  glm::vec3 LinearVelocity = glm::vec3(0.0F);
  glm::vec3 AngularVelocity = glm::vec3(0.0F);

  RigidbodyComponent() = default;
};

/**
 * @brief Box collider component
 */
struct BE_API BoxColliderComponent {
  glm::vec3 Size = glm::vec3(1.0F);
  glm::vec3 Offset = glm::vec3(0.0F);
  bool IsTrigger = false;

  BoxColliderComponent() = default;
  explicit BoxColliderComponent(const glm::vec3 &size) : Size(size) {}
};

/**
 * @brief Sphere collider component
 */
struct BE_API SphereColliderComponent {
  float Radius = 0.5F;
  glm::vec3 Offset = glm::vec3(0.0F);
  bool IsTrigger = false;

  SphereColliderComponent() = default;
  explicit SphereColliderComponent(float radius) : Radius(radius) {}
};

/**
 * @brief Audio source component (for future audio)
 */
struct BE_API AudioSourceComponent {
  std::string AudioClipPath;
  float Volume = 1.0F;
  float Pitch = 1.0F;
  bool Loop = false;
  bool PlayOnAwake = false;
  bool Spatial = true; // 3D audio
  float MinDistance = 1.0F;
  float MaxDistance = 500.0F;

  AudioSourceComponent() = default;
};

/**
 * @brief UI Canvas component (for future UI)
 */
struct BE_API UICanvasComponent {
  enum class RenderMode : uint8_t { ScreenSpace, WorldSpace };

  RenderMode Mode = RenderMode::ScreenSpace;
  int SortOrder = 0;

  UICanvasComponent() = default;
};

} // namespace BeEngine
