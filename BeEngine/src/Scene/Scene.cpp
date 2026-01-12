#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

// ============================================================================
// Entity Implementation
// ============================================================================

bool Entity::IsValid() const {
  if (m_Scene == nullptr || !m_ID.IsValid()) {
    return false;
  }
  return m_Scene->IsEntityValid(m_ID);
}

// ============================================================================
// Scene Implementation
// ============================================================================

Scene::Scene() { BE_CORE_TRACE("Created scene: {}", m_Name); }

Scene::Scene(const std::string &name) : m_Name(name) {
  BE_CORE_TRACE("Created scene: {}", m_Name);
}

Scene::~Scene() {
  BE_CORE_TRACE("Destroying scene: {}", m_Name);

  // Clear all component pools
  m_ComponentPools.clear();
}

Scene::Scene(Scene &&other) noexcept
    : m_Name(std::move(other.m_Name)),
      m_EntityGenerations(std::move(other.m_EntityGenerations)),
      m_FreeEntitySlots(std::move(other.m_FreeEntitySlots)),
      m_EntityCount(other.m_EntityCount),
      m_ComponentPools(std::move(other.m_ComponentPools)),
      m_IsPlaying(other.m_IsPlaying),
      m_EntityActive(std::move(other.m_EntityActive)) {
  other.m_EntityCount = 0;
  other.m_IsPlaying = false;
}

Scene &Scene::operator=(Scene &&other) noexcept {
  if (this != &other) {
    m_Name = std::move(other.m_Name);
    m_EntityGenerations = std::move(other.m_EntityGenerations);
    m_FreeEntitySlots = std::move(other.m_FreeEntitySlots);
    m_EntityCount = other.m_EntityCount;
    m_ComponentPools = std::move(other.m_ComponentPools);
    m_IsPlaying = other.m_IsPlaying;
    m_EntityActive = std::move(other.m_EntityActive);

    other.m_EntityCount = 0;
    other.m_IsPlaying = false;
  }
  return *this;
}

// ============================================================================
// Entity Management
// ============================================================================

EntityID Scene::AllocateEntityID() {
  uint32_t index;
  uint32_t generation;

  if (!m_FreeEntitySlots.empty()) {
    // Reuse a freed slot
    index = m_FreeEntitySlots.front();
    m_FreeEntitySlots.pop_front();
    generation = m_EntityGenerations[index];
  } else {
    // Allocate new slot
    index = static_cast<uint32_t>(m_EntityGenerations.size());
    m_EntityGenerations.push_back(1); // Start at generation 1
    m_EntityActive.push_back(false);
    generation = 1;
  }

  m_EntityActive[index] = true;
  m_EntityCount++;

  return {index, generation};
}

void Scene::FreeEntityID(EntityID id) {
  if (!IsEntityValid(id)) {
    return;
  }

  uint32_t index = id.GetIndex();

  // Increment generation so old handles become invalid
  m_EntityGenerations[index]++;
  m_EntityActive[index] = false;

  // Add to free list for reuse
  m_FreeEntitySlots.push_back(index);
  m_EntityCount--;
}

Entity Scene::CreateEntity(const std::string &name) {
  EntityID id = AllocateEntityID();
  Entity entity(id, this);

  // Every entity has a TagComponent and TransformComponent by default
  AddComponent<TagComponent>(id, name);
  AddComponent<TransformComponent>(id);

  BE_CORE_TRACE("Created entity '{}' (ID: {})", name, id.GetID());
  return entity;
}

Entity Scene::CreateEntityWithUUID(uint64_t uuid, const std::string &name) {
  EntityID id = EntityID::FromID(uuid);

  // Ensure we have enough slots
  uint32_t index = id.GetIndex();
  while (m_EntityGenerations.size() <= index) {
    m_EntityGenerations.push_back(0);
    m_EntityActive.push_back(false);
  }

  m_EntityGenerations[index] = id.GetGeneration();
  m_EntityActive[index] = true;
  m_EntityCount++;

  Entity entity(id, this);

  AddComponent<TagComponent>(id, name);
  AddComponent<TransformComponent>(id);

  BE_CORE_TRACE("Created entity '{}' with UUID: {}", name, uuid);
  return entity;
}

void Scene::DestroyEntity(Entity entity) { DestroyEntity(entity.GetID()); }

void Scene::DestroyEntity(EntityID id) {
  if (!IsEntityValid(id)) {
    BE_CORE_WARN("Attempted to destroy invalid entity");
    return;
  }

  // Get entity name for logging
  std::string name = "Unknown";
  if (HasComponent<TagComponent>(id)) {
    name = GetComponent<TagComponent>(id).Tag;
  }

  // Remove all components from this entity
  uint32_t index = id.GetIndex();
  for (auto &[typeId, pool] : m_ComponentPools) {
    pool->Remove(index);
  }

  // Remove from hierarchy
  if (HasComponent<TransformComponent>(id)) {
    auto &transform = GetComponent<TransformComponent>(id);

    // Reparent children to this entity's parent
    for (EntityID childId : transform.Children) {
      if (IsEntityValid(childId)) {
        auto &childTransform = GetComponent<TransformComponent>(childId);
        childTransform.Parent = transform.Parent;

        // Add to grandparent's children if it exists
        if (transform.Parent.IsValid() && IsEntityValid(transform.Parent)) {
          auto &parentTransform =
              GetComponent<TransformComponent>(transform.Parent);
          parentTransform.Children.push_back(childId);
        }
      }
    }

    // Remove from parent's children list
    if (transform.Parent.IsValid() && IsEntityValid(transform.Parent)) {
      auto &parentTransform =
          GetComponent<TransformComponent>(transform.Parent);
      auto &siblings = parentTransform.Children;
      siblings.erase(std::remove(siblings.begin(), siblings.end(), id),
                     siblings.end());
    }
  }

  // Free the entity slot
  FreeEntityID(id);

  BE_CORE_TRACE("Destroyed entity '{}'", name);
}

bool Scene::IsEntityValid(EntityID id) const {
  if (!id.IsValid()) {
    return false;
  }

  uint32_t index = id.GetIndex();
  if (index >= m_EntityGenerations.size()) {
    return false;
  }

  // Check generation matches (handle is not stale)
  if (m_EntityGenerations[index] != id.GetGeneration()) {
    return false;
  }

  return m_EntityActive[index];
}

Entity Scene::GetEntity(EntityID id) {
  if (IsEntityValid(id)) {
    return {id, this};
  }
  return {};
}

Entity Scene::FindEntityByName(const std::string &name) {
  auto *pool = GetComponentPool<TagComponent>();
  if (pool == nullptr) {
    return {};
  }

  auto &tags = pool->GetAll();
  for (size_t i = 0; i < tags.size(); i++) {
    if (tags[i].Tag == name) {
      uint32_t entityIndex = pool->GetEntityIndex(static_cast<uint32_t>(i));
      EntityID id(entityIndex, m_EntityGenerations[entityIndex]);
      if (IsEntityValid(id)) {
        return {id, this};
      }
    }
  }
  return {};
}

std::vector<Entity> Scene::FindEntitiesByName(const std::string &name) {
  std::vector<Entity> result;

  auto *pool = GetComponentPool<TagComponent>();
  if (pool == nullptr) {
    return result;
  }

  auto &tags = pool->GetAll();
  for (size_t i = 0; i < tags.size(); i++) {
    if (tags[i].Tag == name) {
      uint32_t entityIndex = pool->GetEntityIndex(static_cast<uint32_t>(i));
      EntityID id(entityIndex, m_EntityGenerations[entityIndex]);
      if (IsEntityValid(id)) {
        result.emplace_back(id, this);
      }
    }
  }
  return result;
}

std::vector<Entity> Scene::GetAllEntities() {
  std::vector<Entity> result;
  result.reserve(m_EntityCount);

  for (size_t i = 0; i < m_EntityActive.size(); i++) {
    if (m_EntityActive[i]) {
      EntityID id(static_cast<uint32_t>(i), m_EntityGenerations[i]);
      result.emplace_back(id, this);
    }
  }
  return result;
}

// ============================================================================
// Hierarchy
// ============================================================================

void Scene::SetParent(Entity child, Entity parent) {
  if (!child.IsValid() || !parent.IsValid()) {
    BE_CORE_WARN("SetParent: Invalid entity");
    return;
  }

  auto &childTransform = child.GetComponent<TransformComponent>();
  auto &parentTransform = parent.GetComponent<TransformComponent>();

  // Remove from old parent
  if (childTransform.Parent.IsValid()) {
    Entity oldParent = GetEntity(childTransform.Parent);
    if (oldParent.IsValid()) {
      auto &oldParentTransform = oldParent.GetComponent<TransformComponent>();
      auto &siblings = oldParentTransform.Children;
      siblings.erase(
          std::remove(siblings.begin(), siblings.end(), child.GetID()),
          siblings.end());
    }
  }

  // Set new parent
  childTransform.Parent = parent.GetID();
  parentTransform.Children.push_back(child.GetID());
}

void Scene::RemoveParent(Entity child) {
  if (!child.IsValid()) {
    return;
  }

  auto &childTransform = child.GetComponent<TransformComponent>();

  if (childTransform.Parent.IsValid()) {
    Entity parent = GetEntity(childTransform.Parent);
    if (parent.IsValid()) {
      auto &parentTransform = parent.GetComponent<TransformComponent>();
      auto &siblings = parentTransform.Children;
      siblings.erase(
          std::remove(siblings.begin(), siblings.end(), child.GetID()),
          siblings.end());
    }
  }

  childTransform.Parent = EntityID();
}

std::vector<Entity> Scene::GetChildren(Entity parent) {
  std::vector<Entity> result;

  if (!parent.IsValid()) {
    return result;
  }

  auto *transform = parent.TryGetComponent<TransformComponent>();
  if (transform == nullptr) {
    return result;
  }

  for (EntityID childId : transform->Children) {
    if (IsEntityValid(childId)) {
      result.emplace_back(childId, this);
    }
  }
  return result;
}

std::vector<Entity> Scene::GetRootEntities() {
  std::vector<Entity> result;

  Each<TransformComponent>([&](Entity entity, TransformComponent &transform) {
    if (!transform.Parent.IsValid()) {
      result.push_back(entity);
    }
  });

  return result;
}

// ============================================================================
// Scene Lifecycle
// ============================================================================

void Scene::OnStart() {
  m_IsPlaying = true;
  BE_CORE_INFO("Scene '{}' started", m_Name);

  // Call OnCreate for all native scripts
  Each<NativeScriptComponent>([](Entity entity, NativeScriptComponent &script) {
    if (script.OnCreate) {
      script.OnCreate();
    }
  });
}

void Scene::OnUpdate(float deltaTime) {
  if (!m_IsPlaying) {
    return;
  }

  // Update transforms
  UpdateTransforms();

  // Call OnUpdate for all native scripts
  Each<NativeScriptComponent>(
      [deltaTime](Entity entity, NativeScriptComponent &script) {
        if (script.OnUpdate) {
          script.OnUpdate(deltaTime);
        }
      });
}

void Scene::OnStop() {
  // Call OnDestroy for all native scripts
  Each<NativeScriptComponent>([](Entity entity, NativeScriptComponent &script) {
    if (script.OnDestroy) {
      script.OnDestroy();
    }
  });

  m_IsPlaying = false;
  BE_CORE_INFO("Scene '{}' stopped", m_Name);
}

void Scene::UpdateTransforms() {
  // Update root entities first, then recursively update children
  auto roots = GetRootEntities();
  for (Entity root : roots) {
    UpdateEntityTransform(root.GetID(), glm::mat4(1.0F));
  }
}

void Scene::UpdateEntityTransform(EntityID id,
                                  const glm::mat4 &parentTransform) {
  if (!IsEntityValid(id)) {
    return;
  }

  auto &transform = GetComponent<TransformComponent>(id);

  // Calculate world matrix
  transform.WorldMatrix =
      parentTransform * transform.LocalTransform.GetWorldMatrix();

  // Update children
  for (EntityID childId : transform.Children) {
    UpdateEntityTransform(childId, transform.WorldMatrix);
  }
}

void Scene::UpdateCameras(float aspectRatio) {
  Each<CameraComponent, TransformComponent>(
      [aspectRatio](Entity entity, CameraComponent &camera,
                    TransformComponent &transform) {
        if (!camera.FixedAspectRatio) {
          camera.AspectRatio = aspectRatio;
        }

        // Update projection matrix
        camera.ProjectionMatrix = camera.GetProjectionMatrix();

        // Update view matrix from transform
        auto position = glm::vec3(transform.WorldMatrix[3]);
        glm::vec3 forward =
            -glm::normalize(glm::vec3(transform.WorldMatrix[2]));
        glm::vec3 up = glm::normalize(glm::vec3(transform.WorldMatrix[1]));

        camera.ViewMatrix = glm::lookAt(position, position + forward, up);
        camera.ViewProjectionMatrix =
            camera.ProjectionMatrix * camera.ViewMatrix;
      });
}

Entity Scene::GetPrimaryCamera() {
  Entity primaryCamera;

  Each<CameraComponent>([&](Entity entity, CameraComponent &camera) {
    if (camera.Primary) {
      primaryCamera = entity;
    }
  });

  return primaryCamera;
}

void Scene::SetPrimaryCamera(Entity entity) {
  // Clear all primary flags
  Each<CameraComponent>(
      [](Entity e, CameraComponent &camera) { camera.Primary = false; });

  // Set the new primary
  if (entity.IsValid() && entity.HasComponent<CameraComponent>()) {
    entity.GetComponent<CameraComponent>().Primary = true;
  }
}

} // namespace BeEngine
