#pragma once

#include <Core.hpp>

namespace BeEngine {
// Forward declarations
class Scene;
class EntityRegistry;

/**
 * @brief Unique identifier for entities
 *
 * Uses a 64-bit ID split into:
 * - 32 bits: Index (which slot in the entity array)
 * - 32 bits: Generation (version to detect stale handles)
 */
class EntityID {
public:
  using IndexType = uint32_t;
  using GenerationType = uint32_t;

  static constexpr IndexType INVALID_INDEX =
      std::numeric_limits<IndexType>::max();
  static constexpr GenerationType INVALID_GENERATION = 0;

  constexpr EntityID()
      : m_Index(INVALID_INDEX), m_Generation(INVALID_GENERATION) {}

  constexpr EntityID(IndexType index, GenerationType generation)
      : m_Index(index), m_Generation(generation) {}

  constexpr EntityID(const EntityID &) = default;
  constexpr EntityID &operator=(const EntityID &) = default;
  constexpr EntityID(EntityID &&) noexcept = default;
  constexpr EntityID &operator=(EntityID &&) noexcept = default;

  NODISCARD constexpr IndexType GetIndex() const { return m_Index; }
  NODISCARD constexpr GenerationType GetGeneration() const {
    return m_Generation;
  }

  NODISCARD constexpr bool IsValid() const {
    return m_Index != INVALID_INDEX && m_Generation != INVALID_GENERATION;
  }

  NODISCARD constexpr uint64_t GetID() const {
    return (static_cast<uint64_t>(m_Generation) << 32) |
           static_cast<uint64_t>(m_Index);
  }

  static constexpr EntityID FromID(uint64_t id) {
    return {static_cast<IndexType>(id & 0xFFFFFFFF),
            static_cast<GenerationType>(id >> 32)};
  }

  constexpr bool operator==(const EntityID &other) const {
    return m_Index == other.m_Index && m_Generation == other.m_Generation;
  }

  constexpr bool operator!=(const EntityID &other) const {
    return !(*this == other);
  }

  constexpr bool operator<(const EntityID &other) const {
    return GetID() < other.GetID();
  }

  // For use as hash key
  struct Hash {
    size_t operator()(const EntityID &id) const {
      return std::hash<uint64_t>{}(id.GetID());
    }
  };

private:
  IndexType m_Index;
  GenerationType m_Generation;
};

/**
 * @brief Lightweight entity handle that provides a convenient API
 *
 * Entity is a thin wrapper around EntityID that holds a reference to the scene.
 * It provides methods to add/remove/get components.
 */
class Entity {
public:
  Entity() = default;
  Entity(EntityID id, Scene *scene) : m_ID(id), m_Scene(scene) {}

  Entity(const Entity &other) = default;
  Entity &operator=(const Entity &other) = default;
  Entity(Entity &&other) noexcept = default;
  Entity &operator=(Entity &&other) noexcept = default;

  // Check if entity is valid
  NODISCARD bool IsValid() const;
  NODISCARD explicit operator bool() const { return IsValid(); }

  // Get the raw ID
  NODISCARD EntityID GetID() const { return m_ID; }
  NODISCARD uint64_t GetUUID() const { return m_ID.GetID(); }

  // Component operations (implemented in Entity.cpp after Scene is defined)
  template <typename T, typename... Args> T &AddComponent(Args &&...args);

  template <typename T> void RemoveComponent();

  template <typename T> NODISCARD T &GetComponent();

  template <typename T> NODISCARD const T &GetComponent() const;

  template <typename T> NODISCARD T *TryGetComponent();

  template <typename T> NODISCARD const T *TryGetComponent() const;

  template <typename T> NODISCARD bool HasComponent() const;

  // Comparison
  bool operator==(const Entity &other) const {
    return m_ID == other.m_ID && m_Scene == other.m_Scene;
  }

  bool operator!=(const Entity &other) const { return !(*this == other); }

  // For use as hash key
  struct Hash {
    size_t operator()(const Entity &entity) const {
      return EntityID::Hash{}(entity.m_ID);
    }
  };

private:
  EntityID m_ID;
  Scene *m_Scene = nullptr;

  friend class Scene;
};

// Null entity constant
inline constexpr EntityID NullEntityID{};
} // namespace BeEngine
