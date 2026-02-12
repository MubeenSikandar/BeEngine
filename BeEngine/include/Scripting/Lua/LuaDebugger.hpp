// LuaDebugger.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Lua/LuaTypeDefinitions.hpp>
#include <atomic>
#include <cstdint>
#include <set>
#include <string>
#include <thread>

// Forward declarations
struct lua_State;
struct lua_Debug;

namespace BeEngine {

// Forward declarations
class LuaEngine;

// ============================================================================
// Debugger Types
// ============================================================================

/**
 * @brief Represents a breakpoint in Lua code
 */
struct LuaBreakpoint {
  uint32_t id{0};
  std::string file; // Source file path
  int line{0};      // Line number (1-based)
  bool enabled{true};
  bool temporary{false};  // One-shot breakpoint
  int hitCount{0};        // Number of times hit
  int hitCondition{0};    // Only break after N hits (0 = always)
  std::string condition;  // Lua expression (break only if true)
  std::string logMessage; // Log instead of break (logpoint)

  NODISCARD bool IsLogpoint() const { return !logMessage.empty(); }
};

/**
 * @brief Represents a watch expression
 */
struct LuaWatch {
  uint32_t id{0};
  std::string expression;
  LuaValue value;
  std::string valueStr; // Stringified value
  bool hasError{false};
  std::string error;
};

/**
 * @brief Variable information for the debugger
 */
struct LuaVariable {
  std::string name;
  LuaValue value;
  std::string valueStr;
  LuaType type{LuaType::Nil};
  std::string typeName;

  // For expandable types (tables, userdata)
  bool expandable{false};
  std::vector<LuaVariable> children;

  // Metadata
  bool isLocal{false};
  bool isUpvalue{false};
  bool isGlobal{false};
  int scopeIndex{0}; // Which scope level (for locals)
};

/**
 * @brief Debugger execution state
 */
enum class DebuggerState : uint8_t {
  Running,  // Script executing normally
  Paused,   // Paused at breakpoint or step
  Stepping, // Single-stepping
  Detached  // Debugger not attached
};

constexpr const char *DebuggerStateToString(DebuggerState state) {
  switch (state) {
  case DebuggerState::Running:
    return "Running";
  case DebuggerState::Paused:
    return "Paused";
  case DebuggerState::Stepping:
    return "Stepping";
  case DebuggerState::Detached:
    return "Detached";
  default:
    return "Unknown";
  }
}

/**
 * @brief Step mode for debugger
 */
enum class StepMode : uint8_t {
  None,       // Not stepping
  Into,       // Step into function calls
  Over,       // Step over function calls
  Out,        // Step out of current function
  Instruction // Step single instruction (rare)
};

// ============================================================================
// Debugger Events
// ============================================================================

/**
 * @brief Events sent from debugger to listeners (e.g., IDE)
 */
struct DebugEvent {
  enum class Type : uint8_t {
    Stopped,       // Execution stopped (breakpoint, step, pause)
    Continued,     // Execution resumed
    Breakpoint,    // Breakpoint added/removed/modified
    Output,        // Script output (print, etc.)
    Exception,     // Error/exception occurred
    ThreadCreated, // New coroutine created
    ThreadExited,  // Coroutine finished
    ModuleLoaded   // Script loaded
  };

  Type type;
  std::string reason; // Why stopped (breakpoint, step, pause, exception)
  std::string file;
  int line{0};
  std::string message;
  LuaBreakpoint breakpoint; // For breakpoint events
};

using DebugEventCallback = std::function<void(const DebugEvent &)>;

// ============================================================================
// Debug Adapter Protocol Support
// ============================================================================

/**
 * @brief Configuration for DAP (Debug Adapter Protocol) server
 *
 * Enables integration with VS Code and other DAP-compatible editors.
 */
struct DAPConfig {
  bool enabled{false};
  uint16_t port{9966};
  std::string host{"127.0.0.1"};
  bool waitForConnection{false}; // Pause until debugger connects
  bool allowRemote{false};       // Allow non-localhost connections
};

// ============================================================================
// Lua Debugger
// ============================================================================

/**
 * @brief Full-featured debugger for Lua scripts
 *
 * Features:
 * - Breakpoints (conditional, logpoints, hit counts)
 * - Stepping (into, over, out)
 * - Variable inspection (locals, upvalues, globals)
 * - Call stack examination
 * - Watch expressions
 * - Exception/error catching
 * - Debug Adapter Protocol (DAP) support for IDE integration
 *
 * Usage:
 * ```cpp
 * LuaDebugger debugger(engine);
 * debugger.SetEventCallback([](const DebugEvent& e) { ... });
 *
 * // Add breakpoint
 * debugger.AddBreakpoint("Scripts/Player.lua", 42);
 *
 * // Start debugging
 * debugger.Attach();
 * ```
 */
class BE_API LuaDebugger {
public:
  // ===== Lifecycle =====

  explicit LuaDebugger(LuaEngine *engine);
  ~LuaDebugger();

  // Non-copyable
  LuaDebugger(const LuaDebugger &) = delete;
  LuaDebugger &operator=(const LuaDebugger &) = delete;

  /// Initialize the debugger
  void Initialize();

  /// Shutdown the debugger
  void Shutdown();

  // ===== Attach/Detach =====

  /// Attach debugger to Lua state
  void Attach();

  /// Detach debugger
  void Detach();

  /// Check if attached
  NODISCARD bool IsAttached() const {
    return m_DebuggerState != DebuggerState::Detached;
  }

  /// Get current debugger state
  NODISCARD DebuggerState GetState() const { return m_DebuggerState; }

  // ===== Execution Control =====

  /// Continue execution
  void Continue();

  /// Pause execution
  void Pause();

  /// Step into next line/function
  void StepInto();

  /// Step over current line
  void StepOver();

  /// Step out of current function
  void StepOut();

  /// Stop execution with error
  void Terminate(const std::string &reason = "");

  /// Restart debugging session
  void Restart();

  // ===== Breakpoints =====

  /// Add a breakpoint
  /// @return Breakpoint ID
  NODISCARD uint32_t AddBreakpoint(const std::string &file, int line);

  /// Add a conditional breakpoint
  NODISCARD uint32_t AddBreakpoint(const std::string &file, int line,
                                   const std::string &condition);

  /// Add a logpoint (logs message instead of breaking)
  NODISCARD uint32_t AddLogpoint(const std::string &file, int line,
                                 const std::string &message);

  /// Remove a breakpoint by ID
  void RemoveBreakpoint(uint32_t id);

  /// Remove all breakpoints in a file
  void RemoveBreakpoints(const std::string &file);

  /// Remove all breakpoints
  void RemoveAllBreakpoints();

  /// Enable/disable a breakpoint
  void EnableBreakpoint(uint32_t id, bool enabled);

  /// Get breakpoint by ID
  NODISCARD const LuaBreakpoint *GetBreakpoint(uint32_t id) const;

  /// Get all breakpoints
  NODISCARD std::vector<LuaBreakpoint> GetBreakpoints() const;

  /// Get breakpoints in a file
  NODISCARD std::vector<LuaBreakpoint>
  GetBreakpoints(const std::string &file) const;

  /// Set breakpoint hit condition
  void SetBreakpointHitCondition(uint32_t id, int hitCount);

  /// Set breakpoint condition
  void SetBreakpointCondition(uint32_t id, const std::string &condition);

  // ===== Watch Expressions =====

  /// Add a watch expression
  NODISCARD uint32_t AddWatch(const std::string &expression);

  /// Remove a watch
  void RemoveWatch(uint32_t id);

  /// Remove all watches
  void RemoveAllWatches();

  /// Get watch by ID
  NODISCARD const LuaWatch *GetWatch(uint32_t id) const;

  /// Get all watches
  NODISCARD std::vector<LuaWatch> GetWatches() const;

  /// Evaluate a watch expression
  NODISCARD LuaWatch EvaluateExpression(const std::string &expression);

  /// Update all watch values
  void UpdateWatches();

  // ===== Stack Inspection =====

  /// Get current call stack
  NODISCARD std::vector<LuaStackFrame> GetCallStack() const;

  /// Get call stack depth
  NODISCARD int GetStackDepth() const;

  /// Get frame at index (0 = top)
  NODISCARD std::optional<LuaStackFrame> GetFrame(int index) const;

  /// Get current execution position
  NODISCARD std::pair<std::string, int> GetCurrentPosition() const;

  // ===== Variable Inspection =====

  /// Get local variables at stack level
  NODISCARD std::vector<LuaVariable> GetLocals(int stackLevel = 0) const;

  /// Get upvalues at stack level
  NODISCARD std::vector<LuaVariable> GetUpvalues(int stackLevel = 0) const;

  /// Get global variables
  NODISCARD std::vector<LuaVariable> GetGlobals() const;

  /// Get a specific variable by name
  NODISCARD std::optional<LuaVariable> GetVariable(const std::string &name,
                                                   int stackLevel = 0) const;

  /// Set a variable value
  NODISCARD bool SetVariable(const std::string &name, const LuaValue &value,
                             int stackLevel = 0);

  /// Expand a table/userdata variable
  NODISCARD std::vector<LuaVariable>
  ExpandVariable(const LuaVariable &var) const;

  // ===== Evaluation =====

  /// Evaluate Lua code in current context
  NODISCARD std::pair<bool, std::string> Evaluate(const std::string &code,
                                                  int stackLevel = 0);

  /// Execute code in current context (allows side effects)
  NODISCARD bool Execute(const std::string &code, int stackLevel = 0);

  // ===== Source Mapping =====

  /// Add source file mapping (for minified/compiled code)
  void AddSourceMapping(const std::string &generatedFile,
                        const std::string &originalFile);

  /// Get original source position
  NODISCARD std::pair<std::string, int>
  GetOriginalPosition(const std::string &file, int line) const;

  /// Load source file content
  NODISCARD std::string GetSourceContent(const std::string &file) const;

  // ===== Exception Handling =====

  /// Break on all exceptions
  void SetBreakOnException(bool enabled);

  /// Break on uncaught exceptions only
  void SetBreakOnUncaughtException(bool enabled);

  /// Get exception settings
  NODISCARD bool GetBreakOnException() const { return m_BreakOnException; }
  NODISCARD bool GetBreakOnUncaughtException() const {
    return m_BreakOnUncaught;
  }

  // ===== Event Handling =====

  /// Set event callback
  void SetEventCallback(DebugEventCallback callback) {
    m_EventCallback = std::move(callback);
  }

  // ===== DAP Server =====

  /// Configure DAP server
  void ConfigureDAP(const DAPConfig &config);

  /// Start DAP server
  NODISCARD bool StartDAPServer();

  /// Stop DAP server
  void StopDAPServer();

  /// Check if DAP server is running
  NODISCARD bool IsDAPRunning() const { return m_DAPRunning; }

  // ===== Thread/Coroutine Support =====

  /// Get all threads (coroutines)
  NODISCARD std::vector<std::pair<int, std::string>> GetThreads() const;

  /// Switch to thread
  void SwitchThread(int threadId);

  /// Get current thread ID
  NODISCARD int GetCurrentThread() const { return m_CurrentThread; }

private:
  // ===== Lua Hooks =====

  static void DebugHook(lua_State *L, lua_Debug *ar);
  void HandleHook(lua_State *L, lua_Debug *ar);
  void HandleLine(lua_State *L, lua_Debug *ar);
  void HandleCall(lua_State *L, lua_Debug *ar);
  void HandleReturn(lua_State *L, lua_Debug *ar);

  // ===== Internal Methods =====

  void SetupHook();
  void RemoveHook();

  NODISCARD bool CheckBreakpoint(const std::string &file, int line);
  NODISCARD bool EvaluateCondition(const std::string &condition);
  void ProcessLogpoint(const LuaBreakpoint &bp);

  void PauseExecution(const std::string &reason);
  void ResumeExecution();
  void WaitForResume();

  void SendEvent(const DebugEvent &event);

  NODISCARD LuaVariable CreateVariable(lua_State *L, int index,
                                       const std::string &name) const;
  NODISCARD std::string ValueToString(const LuaValue &value) const;

  NODISCARD uint32_t GenerateBreakpointId();
  NODISCARD uint32_t GenerateWatchId();

  static LuaDebugger *GetFromRegistry(lua_State *L);
  void StoreInRegistry();

  // ===== DAP Implementation =====

  void DAPThread();
  void HandleDAPRequest(const std::string &request);

  LuaEngine *m_Engine{nullptr};
  lua_State *m_State{nullptr};

  // State
  std::atomic<DebuggerState> m_DebugState{DebuggerState::Detached};
  DebuggerState m_DebuggerState{DebuggerState::Detached};
  StepMode m_StepMode{StepMode::None};
  int m_StepDepth{0}; // Stack depth when step started

  // Breakpoints
  std::unordered_map<uint32_t, LuaBreakpoint> m_Breakpoints;
  std::unordered_map<std::string, std::set<int>>
      m_BreakpointLines; // Quick lookup
  uint32_t m_NextBreakpointId{1};
  mutable std::mutex m_BreakpointMutex;

  // Watches
  std::unordered_map<uint32_t, LuaWatch> m_Watches;
  uint32_t m_NextWatchId{1};

  // Source mappings
  std::unordered_map<std::string, std::string> m_SourceMappings;

  // Exception handling
  bool m_BreakOnException{false};
  bool m_BreakOnUncaught{true};

  // Pause synchronization
  std::mutex m_PauseMutex;
  std::condition_variable m_PauseCondition;
  std::atomic<bool> m_Paused{false};

  // Event callback
  DebugEventCallback m_EventCallback;

  // Thread tracking
  int m_CurrentThread{0};
  std::unordered_map<int, lua_State *> m_Threads;

  // DAP server
  DAPConfig m_DAPConfig;
  std::atomic<bool> m_DAPRunning{false};
  std::thread m_DAPThread;

  // Registry key
  static constexpr const char *REGISTRY_KEY = "BeEngine.LuaDebugger";
};

} // namespace BeEngine
