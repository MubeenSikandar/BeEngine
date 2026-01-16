// Scene.hpp
#pragma once

#include <Core.hpp>
#include <Logs/Log.hpp>
#include <Scene/ECS/Component.hpp>
#include <Scene/ECS/Entity.hpp>
#include <glm/glm.hpp>
#include <vector>

namespace BeEngine {

// Forward declarations
class SceneRenderer;
class SceneSerializer;

/**
 * @brief Scene - Container for all entities and their components
 *
 * The Scene class manages:
 * - Entity creation and destruction
 * - Component storage and retrieval
 * - Entity hierarchy (parent-child relationships)
 * - Scene lifecycle (start, update, stop)
 */
class BE_API Scene {
public:
  Scene();
  explicit Scene(const std::string &name);
  ~Scene();

  // Disable copy
  Scene(const Scene &) = delete;
  Scene &operator=(const Scene &) = delete;

  // Move semantics
  Scene(Scene &&other) noexcept;
  Scene &operator=(Scene &&other) noexcept;

  // ===== Entity Management =====

  /**
   * @brief Create a new entity
   * @param name Optional name for the entity
   * @return Handle to the new entity
   */
  Entity CreateEntity(const std::string &name = "Entity");

  /**
   * @brief Create entity with a specific UUID (for deserialization)
   */
  Entity CreateEntityWithUUID(uint64_t uuid,
                              const std::string &name = "Entity");

  /**
   * @brief Destroy an entity and all its components
   */
  void DestroyEntity(Entity entity);

  /**
   * @brief Destroy an entity by ID
   */
  void DestroyEntity(EntityID id);

  /**
   * @brief Check if an entity is valid (exists in this scene)
   */
  NODISCARD bool IsEntityValid(EntityID id) const;

  /**
   * @brief Get an entity handle from an ID
   */
  NODISCARD Entity GetEntity(EntityID id);

  /**
   * @brief Find entity by name (returns first match)
   */
  NODISCARD Entity FindEntityByName(const std::string &name);

  /**
   * @brief Find all entities with a given name
   */
  NODISCARD std::vector<Entity> FindEntitiesByName(const std::string &name);

  /**
   * @brief Get all entities in the scene
   */
  NODISCARD std::vector<Entity> GetAllEntities();

  /**
   * @brief Get the number of active entities
   */
  NODISCARD size_t GetEntityCount() const { return m_EntityCount; }

  // Get entity by UUID (for scripting/serialization)
  NODISCARD Entity GetEntityByUUID(uint64_t uuid);

  // Duplicate an entity (useful for editor)
  Entity DuplicateEntity(Entity entity);

  // Clear all entities
  void Clear();

  // ===== Component Management =====

  /**
   * @brief Add a component to an entity
   */
  template <typename T, typename... Args>
  T &AddComponent(EntityID id, Args &&...args) {
    auto *pool = GetOrCreateComponentPool<T>();
    return pool->Add(id.GetIndex(), std::forward<Args>(args)...);
  }

  /**
   * @brief Remove a component from an entity
   */
  template <typename T> void RemoveComponent(EntityID id) {
    auto *pool = GetComponentPool<T>();
    if (pool) {
      pool->Remove(id.GetIndex());
    }
  }

  /**
   * @brief Get a component from an entity
   */
  template <typename T> T &GetComponent(EntityID id) {
    auto *pool = GetComponentPool<T>();
    BE_CORE_ASSERT(pool && pool->Has(id.GetIndex()),
                   "Entity does not have component!");
    return pool->Get(id.GetIndex());
  }

  /**
   * @brief Get a component from an entity (const version)
   */
  template <typename T> const T &GetComponent(EntityID id) const {
    auto *pool = GetComponentPool<T>();
    BE_CORE_ASSERT(pool && pool->Has(id.GetIndex()),
                   "Entity does not have component!");
    return pool->Get(id.GetIndex());
  }
  /**
   * @brief Try to get a component (returns nullptr if not found)
   */
  template <typename T> T *TryGetComponent(EntityID id) {
    auto *pool = GetComponentPool<T>();
    if (!pool) {
      return nullptr;
    }
    return pool->TryGet(id.GetIndex());
  }

  /**
   * @brief Try to get a component (const version)
   */
  template <typename T> const T *TryGetComponent(EntityID id) const {
    auto *pool = GetComponentPool<T>();
    if (!pool) {
      return nullptr;
    }
    return pool->TryGet(id.GetIndex());
  }

  /**
   * @brief Check if an entity has a component
   */
  template <typename T> NODISCARD bool HasComponent(EntityID id) const {
    auto *pool = GetComponentPool<T>();
    return pool && pool->Has(id.GetIndex());
  }

  /**
   * @brief Get the component pool for a type
   */
  template <typename T> ComponentPool<T> *GetComponentPool() {
    auto typeId = GetComponentTypeID<T>();
    auto it = m_ComponentPools.find(typeId);
    if (it == m_ComponentPools.end()) {
      return nullptr;
    }
    return static_cast<ComponentPool<T> *>(it->second.get());
  }

  /**
   * @brief Get the component pool for a type (const version)
   */
  template <typename T> const ComponentPool<T> *GetComponentPool() const {
    auto typeId = GetComponentTypeID<T>();
    auto it = m_ComponentPools.find(typeId);
    if (it == m_ComponentPools.end()) {
      return nullptr;
    }
    return static_cast<const ComponentPool<T> *>(it->second.get());
  }

  // ===== View - Iterate over entities with specific components =====

  /**
   * @brief Iterate over all entities with the specified component
   * @param func Callback function taking (Entity, Component&)
   */
  template <typename T, typename Func> void Each(Func &&func) {
    auto *pool = GetComponentPool<T>();
    if (!pool) {
      return;
    }

    auto &components = pool->GetAll();
    for (size_t i = 0; i < components.size(); i++) {
      uint32_t entityIndex = pool->GetEntityIndex(static_cast<uint32_t>(i));
      EntityID id(entityIndex, m_EntityGenerations[entityIndex]);
      Entity entity(id, this);
      func(entity, components[i]);
    }
  }

  /**
   * @brief Iterate over all entities with multiple components
   * @param func Callback function taking (Entity, Component1&, Component2&,
   * ...)
   */
  template <typename T1, typename T2, typename Func> void Each(Func &&func) {
    auto *pool1 = GetComponentPool<T1>();
    if (!pool1) {
      return;
    }

    auto &components1 = pool1->GetAll();
    for (size_t i = 0; i < components1.size(); i++) {
      uint32_t entityIndex = pool1->GetEntityIndex(static_cast<uint32_t>(i));

      // Check if entity has T2 as well
      auto *comp2 = TryGetComponent<T2>(
          EntityID(entityIndex, m_EntityGenerations[entityIndex]));
      if (!comp2) {
        continue;
      }

      EntityID id(entityIndex, m_EntityGenerations[entityIndex]);
      Entity entity(id, this);
      func(entity, components1[i], *comp2);
    }
  }

  /**
   * @brief Iterate over all entities with three components
   */
  template <typename T1, typename T2, typename T3, typename Func>
  void Each(Func &&func) {
    auto *pool1 = GetComponentPool<T1>();
    if (!pool1) {
      return;
    }

    auto &components1 = pool1->GetAll();
    for (size_t i = 0; i < components1.size(); i++) {
      uint32_t entityIndex = pool1->GetEntityIndex(static_cast<uint32_t>(i));
      EntityID id(entityIndex, m_EntityGenerations[entityIndex]);

      auto *comp2 = TryGetComponent<T2>(id);
      auto *comp3 = TryGetComponent<T3>(id);
      if (!comp2 || !comp3) {
        continue;
      }

      Entity entity(id, this);
      func(entity, components1[i], *comp2, *comp3);
    }
  }

  // ===== Hierarchy =====

  /**
   * @brief Set parent-child relationship between entities
   */
  void SetParent(Entity child, Entity parent);

  /**
   * @brief Remove parent from an entity
   */
  void RemoveParent(Entity child);

  /**
   * @brief Get children of an entity
   */
  NODISCARD std::vector<Entity> GetChildren(Entity parent);

  /**
   * @brief Get root entities (entities without parents)
   */
  NODISCARD std::vector<Entity> GetRootEntities();

  // ===== Scene Lifecycle =====

  /**
   * @brief Called when scene starts playing
   */
  void OnStart();

  /**
   * @brief Called every frame during play
   */
  void OnUpdate(float deltaTime);

  /**
   * @brief Called when scene stops playing
   */
  void OnStop();

  /**
   * @brief Update transforms (called automatically in OnUpdate)
   */
  void UpdateTransforms();

  /**
   * @brief Update cameras (called automatically in OnUpdate)
   */
  void UpdateCameras(float aspectRatio);

  // ===== Scene Properties =====

  NODISCARD const std::string &GetName() const { return m_Name; }
  void SetName(const std::string &name) { m_Name = name; }

  NODISCARD bool IsPlaying() const { return m_IsPlaying; }

  // ===== Primary Camera =====

  /**
   * @brief Get the primary camera entity
   */
  NODISCARD Entity GetPrimaryCamera();

  /**
   * @brief Set an entity as the primary camera
   */
  void SetPrimaryCamera(Entity entity);

  // Internal entity management
  EntityID AllocateEntityID();
  void FreeEntityID(EntityID id);

  // Component pool management
  template <typename T> ComponentPool<T> *GetOrCreateComponentPool() {
    auto typeId = GetComponentTypeID<T>();
    auto it = m_ComponentPools.find(typeId);
    if (it == m_ComponentPools.end()) {
      auto pool = std::make_unique<ComponentPool<T>>();
      auto *rawPtr = pool.get();
      m_ComponentPools[typeId] = std::move(pool);
      return rawPtr;
    }
    return static_cast<ComponentPool<T> *>(it->second.get());
  }

  // Update hierarchy transforms recursively
  void UpdateEntityTransform(EntityID id, const glm::mat4 &parentTransform);

private:
  std::string m_Name = "Untitled Scene";

  // Entity management
  std::vector<uint32_t> m_EntityGenerations; // Generation counter per slot
  std::deque<uint32_t> m_FreeEntitySlots;    // Recycled entity slots
  size_t m_EntityCount = 0;

  // Component storage
  std::unordered_map<ComponentTypeID, std::unique_ptr<IComponentPool>>
      m_ComponentPools;

  // Scene state
  bool m_IsPlaying = false;

  // Entity validity tracking
  std::vector<bool> m_EntityActive;

  friend class Entity;
  friend class SceneSerializer;
  friend class SceneRenderer;
};

// ============================================================================
// Entity Template Implementations
// ============================================================================

template <typename T, typename... Args>
T &Entity::AddComponent(Args &&...args) {
  BE_CORE_ASSERT(m_Scene, "Entity has no scene!");
  BE_CORE_ASSERT(IsValid(), "Entity is not valid!");
  return m_Scene->AddComponent<T>(m_ID, std::forward<Args>(args)...);
}

template <typename T> void Entity::RemoveComponent() {
  BE_CORE_ASSERT(m_Scene, "Entity has no scene!");
  BE_CORE_ASSERT(IsValid(), "Entity is not valid!");
  m_Scene->RemoveComponent<T>(m_ID);
}

template <typename T> T &Entity::GetComponent() {
  BE_CORE_ASSERT(m_Scene, "Entity has no scene!");
  BE_CORE_ASSERT(IsValid(), "Entity is not valid!");
  return m_Scene->GetComponent<T>(m_ID);
}

template <typename T> const T &Entity::GetComponent() const {
  BE_CORE_ASSERT(m_Scene, "Entity has no scene!");
  BE_CORE_ASSERT(IsValid(), "Entity is not valid!");
  return m_Scene->GetComponent<T>(m_ID);
}

template <typename T> T *Entity::TryGetComponent() {
  if (!m_Scene || !IsValid()) {
    return nullptr;
  }
  return m_Scene->TryGetComponent<T>(m_ID);
}

template <typename T> const T *Entity::TryGetComponent() const {
  if (!m_Scene || !IsValid()) {
    return nullptr;
  }
  return m_Scene->TryGetComponent<T>(m_ID);
}

template <typename T> bool Entity::HasComponent() const {
  if (!m_Scene || !IsValid()) {
    return false;
  }
  return m_Scene->HasComponent<T>(m_ID);
}

} // namespace BeEngine
