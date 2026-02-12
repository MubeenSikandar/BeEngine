// LuaCoroutine.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Lua/LuaTypeDefinitions.hpp>
#include <functional>

// Forward declarations
struct lua_State;

namespace BeEngine {
// Forward declarations
class LuaEngine;
class LuaInstance;

// ============================================================================
// Yield Instructions
// ============================================================================

/**
 * @brief Base class for coroutine yield instructions
 *
 * Yield instructions tell the coroutine system when to resume a coroutine.
 */
class BE_API YieldInstruction {
public:
  virtual ~YieldInstruction() = default;

  /// Check if the coroutine should resume
  NODISCARD virtual bool IsComplete() const = 0;

  /// Update the instruction (called each frame)
  virtual void Update(float deltaTime) {}

  /// Get debug name
  NODISCARD virtual const char *GetName() const = 0;
};

/**
 * @brief Wait for a number of seconds
 *
 * Lua usage: yield(WaitForSeconds(1.5))
 */
class BE_API WaitForSeconds : public YieldInstruction {
public:
  explicit WaitForSeconds(float seconds, bool useUnscaledTime = false);

  NODISCARD bool IsComplete() const override { return m_Elapsed >= m_Duration; }
  void Update(float deltaTime) override;
  NODISCARD const char *GetName() const override { return "WaitForSeconds"; }

  NODISCARD float GetRemaining() const { return m_Duration - m_Elapsed; }

private:
  float m_Duration;
  float m_Elapsed{0.0F};
  bool m_UseUnscaledTime;
};

/**
 * @brief Wait until the end of the frame
 *
 * Lua usage: yield(WaitForEndOfFrame())
 */
class BE_API WaitForEndOfFrame : public YieldInstruction {
public:
  WaitForEndOfFrame() = default;

  NODISCARD bool IsComplete() const override { return m_FramePassed; }
  void Update(float deltaTime) override { m_FramePassed = true; }
  NODISCARD const char *GetName() const override { return "WaitForEndOfFrame"; }

private:
  bool m_FramePassed{false};
};

/**
 * @brief Wait for the next fixed update
 *
 * Lua usage: yield(WaitForFixedUpdate())
 */
class BE_API WaitForFixedUpdate : public YieldInstruction {
public:
  WaitForFixedUpdate() = default;

  NODISCARD bool IsComplete() const override { return m_Complete; }
  void SetComplete() { m_Complete = true; }
  NODISCARD const char *GetName() const override {
    return "WaitForFixedUpdate";
  }

private:
  bool m_Complete{false};
};

/**
 * @brief Wait until a condition is true
 *
 * Lua usage: yield(WaitUntil(function() return someCondition end))
 */
class BE_API WaitUntil : public YieldInstruction {
public:
  using Predicate = std::function<bool()>;

  explicit WaitUntil(Predicate predicate);

  NODISCARD bool IsComplete() const override;
  NODISCARD const char *GetName() const override { return "WaitUntil"; }

private:
  Predicate m_Predicate;
};

/**
 * @brief Wait while a condition is true
 *
 * Lua usage: yield(WaitWhile(function() return someCondition end))
 */
class BE_API WaitWhile : public YieldInstruction {
public:
  using Predicate = std::function<bool()>;

  explicit WaitWhile(Predicate predicate);

  NODISCARD bool IsComplete() const override;
  NODISCARD const char *GetName() const override { return "WaitWhile"; }

private:
  Predicate m_Predicate;
};

/**
 * @brief Wait for another coroutine to complete
 *
 * Lua usage: yield(coroutine)
 */
class BE_API WaitForCoroutine : public YieldInstruction {
public:
  explicit WaitForCoroutine(const LuaRef &coroutine);

  NODISCARD bool IsComplete() const override;
  NODISCARD const char *GetName() const override { return "WaitForCoroutine"; }

  NODISCARD const LuaRef &GetCoroutine() const { return m_Coroutine; }

private:
  LuaRef m_Coroutine;
};

/**
 * @brief Wait for an async operation
 *
 * Used for async resource loading, network requests, etc.
 */
class BE_API WaitForAsync : public YieldInstruction {
public:
  explicit WaitForAsync(std::function<bool()> isDone,
                        std::function<void()> onComplete = nullptr);

  NODISCARD bool IsComplete() const override;
  NODISCARD const char *GetName() const override { return "WaitForAsync"; }

private:
  std::function<bool()> m_IsDone;
  std::function<void()> m_OnComplete;
  mutable bool m_Completed{false};
};

// ============================================================================
// Coroutine Handle
// ============================================================================

/**
 * @brief Handle to a running coroutine
 *
 * Provides methods to control and query coroutine state.
 */
class BE_API CoroutineHandle {
public:
  using Id = uint64_t;

  CoroutineHandle() = default;
  explicit CoroutineHandle(Id id) : m_Id(id) {}

  NODISCARD Id GetId() const { return m_Id; }
  NODISCARD bool IsValid() const { return m_Id != 0; }

  bool operator==(const CoroutineHandle &other) const {
    return m_Id == other.m_Id;
  }
  bool operator!=(const CoroutineHandle &other) const {
    return m_Id != other.m_Id;
  }

  // Hash support
  struct Hash {
    size_t operator()(const CoroutineHandle &h) const {
      return std::hash<Id>{}(h.m_Id);
    }
  };

private:
  Id m_Id{0};
};

// ============================================================================
// Coroutine State
// ============================================================================

enum class CoroutineState : uint8_t {
  Created,   // Created but not started
  Running,   // Currently executing
  Suspended, // Yielded, waiting
  Dead,      // Completed or errored
  Cancelled  // Manually cancelled
};

constexpr const char *CoroutineStateToString(CoroutineState state) {
  switch (state) {
  case CoroutineState::Created:
    return "Created";
  case CoroutineState::Running:
    return "Running";
  case CoroutineState::Suspended:
    return "Suspended";
  case CoroutineState::Dead:
    return "Dead";
  case CoroutineState::Cancelled:
    return "Cancelled";
  default:
    return "Unknown";
  }
}

// ============================================================================
// Coroutine Manager
// ============================================================================

/**
 * @brief Manages all Lua coroutines in the engine
 *
 * Features:
 * - Coroutine lifecycle management
 * - Yield instruction processing
 * - Automatic cleanup on script destruction
 * - Debug and profiling support
 *
 * Usage from Lua:
 * ```lua
 * function PlayerController:OnCreate()
 *     -- Start a coroutine
 *     self:StartCoroutine(self.MoveSequence, self)
 * end
 *
 * function PlayerController:MoveSequence()
 *     -- Wait for 2 seconds
 *     yield(WaitForSeconds(2))
 *     print("Waited 2 seconds!")
 *
 *     -- Wait for condition
 *     yield(WaitUntil(function() return self.isReady end))
 *     print("Ready!")
 *
 *     -- Wait one frame
 *     yield()
 * end
 * ```
 */
class BE_API LuaCoroutineManager {
public:
  // ===== Lifecycle =====

  explicit LuaCoroutineManager(LuaEngine *engine);
  ~LuaCoroutineManager();

  // Non-copyable
  LuaCoroutineManager(const LuaCoroutineManager &) = delete;
  LuaCoroutineManager &operator=(const LuaCoroutineManager &) = delete;

  /// Initialize the coroutine system
  void Initialize();

  /// Shutdown and cleanup
  void Shutdown();

  // ===== Coroutine Creation =====

  /// Start a coroutine from a function
  /// @param func Lua function to run
  /// @param owner Optional owning instance (for automatic cleanup)
  /// @return Handle to the coroutine
  NODISCARD CoroutineHandle StartCoroutine(const LuaRef &func,
                                           LuaInstance *owner = nullptr);

  /// Start a coroutine from a method on an instance
  NODISCARD CoroutineHandle StartCoroutine(LuaInstance *instance,
                                           const std::string &methodName);

  /// Create a coroutine from a function (but don't start it)
  NODISCARD CoroutineHandle CreateCoroutine(const LuaRef &func);

  // ===== Coroutine Control =====

  /// Stop a specific coroutine
  void StopCoroutine(CoroutineHandle handle);

  /// Stop all coroutines owned by an instance
  void StopCoroutines(LuaInstance *owner);

  /// Stop all coroutines
  void StopAllCoroutines();

  /// Pause a coroutine
  void PauseCoroutine(CoroutineHandle handle);

  /// Resume a paused coroutine
  void ResumeCoroutine(CoroutineHandle handle);

  // ===== State Query =====

  /// Check if coroutine is running/suspended
  NODISCARD bool IsRunning(CoroutineHandle handle) const;

  /// Get coroutine state
  NODISCARD CoroutineState GetState(CoroutineHandle handle) const;

  /// Get the Lua thread for a coroutine
  NODISCARD lua_State *GetThread(CoroutineHandle handle) const;

  /// Get coroutine count
  NODISCARD size_t GetCoroutineCount() const { return m_Coroutines.size(); }

  /// Get running coroutine count
  NODISCARD size_t GetRunningCount() const;

  // ===== Update =====

  /// Update all coroutines (call each frame)
  void Update(float deltaTime);

  /// Fixed update (processes WaitForFixedUpdate)
  void FixedUpdate(float fixedDelta);

  /// Late update (processes WaitForEndOfFrame from last frame)
  void LateUpdate(float deltaTime);

  // ===== Yield Instruction Registration =====

  /// Register a custom yield instruction type
  template <typename T> void RegisterYieldInstruction(const std::string &name);

  /// Create a yield instruction from Lua
  NODISCARD Scope<YieldInstruction> CreateYieldInstruction(lua_State *L,
                                                           int index);

  // ===== Debug =====

  /// Get debug info for all coroutines
  struct CoroutineDebugInfo {
    CoroutineHandle handle;
    CoroutineState state;
    std::string ownerScript;
    std::string currentYield;
    uint64_t startFrame;
    double totalTime;
    int resumeCount;
  };

  NODISCARD std::vector<CoroutineDebugInfo> GetDebugInfo() const;

  /// Dump coroutine state to log
  void DumpState() const;

  // ===== Statistics =====

  struct Stats {
    size_t totalCreated{0};
    size_t totalCompleted{0};
    size_t totalCancelled{0};
    size_t totalErrors{0};
    size_t peakConcurrent{0};
    double totalResumeTime{0.0};
    uint64_t totalResumes{0};
  };

  NODISCARD Stats GetStats() const { return m_Stats; }
  void ResetStats() { m_Stats = Stats{}; }

private:
  // ===== Internal Types =====

  struct Coroutine {
    CoroutineHandle::Id id{0};
    LuaRef threadRef; // Reference to the Lua thread
    lua_State *thread{nullptr};
    LuaInstance *owner{nullptr};
    CoroutineState state{CoroutineState::Created};
    Scope<YieldInstruction> yieldInstruction;
    uint64_t startFrame{0};
    double totalTime{0.0};
    int resumeCount{0};
    std::string errorMessage;
  };

  // ===== Internal Methods =====

  NODISCARD Coroutine *GetCoroutine(CoroutineHandle handle);
  NODISCARD const Coroutine *GetCoroutine(CoroutineHandle handle) const;

  void ResumeCoroutineInternal(Coroutine &coro);
  void ProcessYield(Coroutine &coro, int nresults);
  void HandleError(Coroutine &coro, const std::string &error);
  void CleanupCoroutine(Coroutine &coro);

  NODISCARD CoroutineHandle::Id GenerateId();

  // ===== Lua Interface =====

  static int Lua_StartCoroutine(lua_State *L);
  static int Lua_StopCoroutine(lua_State *L);
  static int Lua_WaitForSeconds(lua_State *L);
  static int Lua_WaitForEndOfFrame(lua_State *L);
  static int Lua_WaitForFixedUpdate(lua_State *L);
  static int Lua_WaitUntil(lua_State *L);
  static int Lua_WaitWhile(lua_State *L);
  static int Lua_Yield(lua_State *L);

  void RegisterLuaFunctions();

  LuaEngine *m_Engine{nullptr};
  lua_State *m_State{nullptr};

  // All coroutines
  std::unordered_map<CoroutineHandle::Id, Coroutine> m_Coroutines;

  // Coroutines waiting for fixed update
  std::vector<CoroutineHandle::Id> m_WaitingForFixedUpdate;

  // Coroutines waiting for end of frame
  std::vector<CoroutineHandle::Id> m_WaitingForEndOfFrame;

  // ID generation
  CoroutineHandle::Id m_NextId{1};

  // Statistics
  Stats m_Stats;
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename T>
void LuaCoroutineManager::RegisterYieldInstruction(const std::string &name) {
  static_assert(std::is_base_of_v<YieldInstruction, T>,
                "T must derive from YieldInstruction");
  // Registration implementation in cpp file
}

// ============================================================================
// Async Utilities
// ============================================================================

/**
 * @brief Helper for creating async operations in Lua
 *
 * Usage:
 * ```cpp
 * // In C++:
 * LuaAsync::Create(L, [](auto resolve, auto reject) {
 *     // Do async work...
 *     resolve(result);
 * });
 *
 * // In Lua:
 * local result = yield(AsyncOperation())
 * ```
 */
class BE_API LuaAsync {
public:
  using ResolveFunc = std::function<void(const LuaValue &)>;
  using RejectFunc = std::function<void(const std::string &)>;
  using AsyncFunc = std::function<void(ResolveFunc, RejectFunc)>;

  /// Create an async operation
  static LuaRef Create(lua_State *L, AsyncFunc func);

  /// Check if a value is an async operation
  static bool IsAsync(lua_State *L, int index);

  /// Create a completed async
  static LuaRef Resolve(lua_State *L, const LuaValue &value);

  /// Create a rejected async
  static LuaRef Reject(lua_State *L, const std::string &error);

  /// Combine multiple asyncs (wait for all)
  static LuaRef All(lua_State *L, const std::vector<LuaRef> &asyncs);

  /// Race multiple asyncs (first to complete)
  static LuaRef Race(lua_State *L, const std::vector<LuaRef> &asyncs);
};

} // namespace BeEngine
