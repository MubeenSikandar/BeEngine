// LuaInstance.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Core/ScriptInstance.hpp>
#include <Scripting/Lua/LuaTypeDefinitions.hpp>
#include <bitset>

// Forward declarations
struct lua_State;

namespace BeEngine {

// Forward declarations
class LuaEngine;

/**
 * @brief Script instance for Lua scripts
 *
 * Represents a single Lua script attached to an entity.
 * Manages the Lua environment (table) for the instance and
 * dispatches lifecycle events to the appropriate Lua functions.
 *
 * Memory Model:
 * - Each instance has its own environment table in Lua
 * - The environment table inherits from the script class table
 * - Instance data (self) is stored in the environment
 *
 * Thread Safety:
 * - Instances should only be accessed from the main thread
 * - The owning LuaEngine must outlive all instances
 */

class BE_API LuaInstance : public ScriptInstance {
public:
  // ===== Lifecycle =====

  LuaInstance();
  explicit LuaInstance(LuaEngine *engine);
  ~LuaInstance() override;

  // Non-copyable
  LuaInstance(const LuaInstance &) = delete;
  LuaInstance &operator=(const LuaInstance &) = delete;

  // Moveable
  LuaInstance(LuaInstance &&other) noexcept;
  LuaInstance &operator=(LuaInstance &&other) noexcept;

  // ===== ScriptInstance Interface =====

  void OnCreate() override;
  void OnDestroy() override;
  void OnUpdate(float deltaTime) override;
  void OnFixedUpdate(float fixedDelta) override;
  void OnLateUpdate(float deltaTime) override;
  void OnEnable() override;
  void OnDisable() override;
  void OnStart() override;

  // Physics callbacks
  void OnCollisionEnter(const CollisionInfo &collision) override;
  void OnCollisionStay(Entity other) override;
  void OnCollisionExit(Entity other) override;
  void OnTriggerEnter(Entity other) override;
  void OnTriggerStay(Entity other) override;
  void OnTriggerExit(Entity other) override;

  // Hot reload
  void SaveState(ScriptFieldMap &outState) override;
  void RestoreState(const ScriptFieldMap &inState) override;
  void OnBeforeReload() override;
  void OnAfterReload() override;

  // ===== Initialization =====

  /// Initialize the instance with a script class
  /// @param scriptName Name of the registered script class
  /// @return true if successful
  NODISCARD bool Initialize(const std::string &scriptName);

  /// Re-initialize after hot reload
  NODISCARD bool Reinitialize();

  /// Check if properly initialized
  NODISCARD bool IsInitialized() const { return m_Initialized; }

  // ===== Lua Access =====

  /// Get the owning engine
  NODISCARD LuaEngine *GetEngine() const { return m_Engine; }

  /// Get the Lua state
  NODISCARD lua_State *GetState() const;

  /// Get reference to this instance's environment table
  NODISCARD const LuaRef &GetEnvironmentRef() const { return m_EnvironmentRef; }

  /// Get reference to the 'self' table
  NODISCARD const LuaRef &GetSelfRef() const { return m_SelfRef; }

  /// Push the instance environment onto stack
  void PushEnvironment() const;

  /// Push 'self' table onto stack
  void PushSelf() const;

  // ===== Method Invocation =====

  /// Call a method on this instance
  /// @param methodName Name of the method to call
  /// @return Error code
  NODISCARD LuaErrorCode CallMethod(const std::string &methodName);

  /// Call a method with arguments
  /// @param methodName Name of the method
  /// @param numArgs Number of arguments already pushed
  /// @param numResults Number of results to keep on stack
  NODISCARD LuaErrorCode CallMethod(const std::string &methodName, int numArgs,
                                    int numResults);

  /// Call a method and get return value
  template <typename T>
  NODISCARD std::optional<T>
  CallMethodWithResult(const std::string &methodName);

  /// Check if instance has a method
  NODISCARD bool HasMethod(const std::string &methodName) const;

  // ===== Field Access =====

  /// Set a field value on the instance
  void SetInstanceField(const std::string &name, const LuaValue &value);

  /// Get a field value from the instance
  NODISCARD LuaValue GetInstanceField(const std::string &name) const;

  /// Check if instance has a field
  NODISCARD bool HasInstanceField(const std::string &name) const;

  /// Sync C++ fields to Lua
  void SyncFieldsToLua();

  /// Sync Lua fields to C++
  void SyncFieldsFromLua();

  // ===== Event Dispatch =====

  /// Check if event callback exists
  NODISCARD bool HasEventCallback(LuaScriptEvent event) const;

  /// Dispatch an event
  NODISCARD LuaErrorCode DispatchEvent(LuaScriptEvent event);

  /// Dispatch an event with arguments (arguments already on stack)
  NODISCARD LuaErrorCode DispatchEvent(LuaScriptEvent event, int numArgs);

  // ===== Custom Events =====

  /// Call a custom event handler
  NODISCARD LuaErrorCode DispatchCustomEvent(const std::string &eventName);

  /// Call a custom event with data table
  NODISCARD LuaErrorCode DispatchCustomEvent(const std::string &eventName,
                                             const LuaRef &eventData);

  /// Send message to this instance (like Unity's SendMessage)
  NODISCARD LuaErrorCode SendMessage(const std::string &methodName,
                                     const LuaValue &arg = {});

  /// Broadcast message to all scripts on entity
  void BroadcastMessage(const std::string &methodName,
                        const LuaValue &arg = {});

  // ===== Coroutines =====

  /// Start a coroutine from a method
  NODISCARD LuaRef StartCoroutine(const std::string &methodName);

  /// Stop a coroutine
  void StopCoroutine(const LuaRef &coroutine);

  /// Stop all coroutines for this instance
  void StopAllCoroutines();

  /// Check if instance has running coroutines
  NODISCARD bool HasRunningCoroutines() const;

  // ===== Debugging =====

  /// Get debug information
  NODISCARD std::string GetDebugString() const;

  /// Dump instance state to log
  void DumpState() const;

  /// Get method list
  NODISCARD std::vector<std::string> GetMethodList() const;

  /// Get field list
  NODISCARD std::vector<std::string> GetFieldList() const;

  // ===== Performance =====

  /// Get number of method calls
  NODISCARD uint64_t GetMethodCallCount() const { return m_MethodCallCount; }

  /// Get total time spent in methods (microseconds)
  NODISCARD double GetTotalMethodTime() const { return m_TotalMethodTime; }

  /// Reset performance counters
  void ResetCounters();

protected:
  // ===== Internal Methods =====

  /// Create the instance environment
  NODISCARD bool CreateEnvironment();

  /// Destroy the instance environment
  void DestroyEnvironment();

  /// Cache which callbacks exist
  void CacheCallbacks();

  /// Call a lifecycle event
  NODISCARD LuaErrorCode CallLifecycleEvent(LuaScriptEvent event);

  /// Call a lifecycle event with timing
  NODISCARD LuaErrorCode CallLifecycleEventTimed(LuaScriptEvent event,
                                                 double &outTime);

  /// Push collision info to Lua
  void PushCollisionInfo(const CollisionInfo &collision);

  /// Handle method call error
  void HandleMethodError(const std::string &methodName, LuaErrorCode error);

private:
  // Engine reference
  LuaEngine *m_Engine{nullptr};

  // Lua references
  LuaRef m_EnvironmentRef; // Instance's environment table
  LuaRef m_SelfRef;        // The 'self' table for this instance
  LuaRef m_ClassRef;       // Reference to the script class

  // State tracking
  bool m_Initialized{false};
  bool m_StartCalled{false};
  bool m_CreateCalled{false};

  // Callback caching (for performance)
  std::bitset<static_cast<size_t>(LuaScriptEvent::Count)> m_HasCallback;
  bool m_CallbacksCached{false};

  // Method name cache (avoid string construction)
  struct MethodCache {
    std::string onCreate{"OnCreate"};
    std::string onDestroy{"OnDestroy"};
    std::string onUpdate{"OnUpdate"};
    std::string onFixedUpdate{"OnFixedUpdate"};
    std::string onLateUpdate{"OnLateUpdate"};
    std::string onEnable{"OnEnable"};
    std::string onDisable{"OnDisable"};
    std::string onStart{"OnStart"};
    std::string onCollisionEnter{"OnCollisionEnter"};
    std::string onCollisionStay{"OnCollisionStay"};
    std::string onCollisionExit{"OnCollisionExit"};
    std::string onTriggerEnter{"OnTriggerEnter"};
    std::string onTriggerStay{"OnTriggerStay"};
    std::string onTriggerExit{"OnTriggerExit"};
  };
  static const MethodCache s_MethodNames;

  // Coroutines owned by this instance
  std::vector<LuaRef> m_Coroutines;

  // Performance tracking
  uint64_t m_MethodCallCount{0};
  double m_TotalMethodTime{0.0};
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename T>
std::optional<T>
LuaInstance::CallMethodWithResult(const std::string &methodName) {
  auto error = CallMethod(methodName, 0, 1);
  if (error != LuaErrorCode::Ok) {
    return std::nullopt;
  }

  // Get result from stack
  auto *L = GetState();
  if (!L) {
    return std::nullopt;
  }

  // Implementation depends on type T
  // This is a simplified version - full implementation in cpp
  return std::nullopt;
}

// ============================================================================
// Factory for LuaInstance
// ============================================================================

/**
 * @brief Creates LuaInstance objects for the ScriptRegistry
 */
class BE_API LuaInstanceFactory {
public:
  explicit LuaInstanceFactory(LuaEngine *engine) : m_Engine(engine) {}

  /// Create an instance for the given script
  NODISCARD Ref<ScriptInstance> Create(const std::string &scriptName,
                                       Entity entity);

private:
  LuaEngine *m_Engine;
};

} // namespace BeEngine
