// LuaSandbox.hpp
#pragma once

#include "Scripting/Lua/LuaDebugger.hpp"
#include <Core.hpp>
#include <Scripting/Lua/LuaTypeDefinitions.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_set>

// Forward declarations
struct lua_State;
struct lua_Debug;

namespace BeEngine {
// Forward declarations
class LuaEngine;

// ============================================================================
// Sandbox Permission Flags
// ============================================================================

enum class SandboxPermission : uint32_t {
  None = 0,

  // File system
  ReadFiles = 1 << 0,       // Can read files
  WriteFiles = 1 << 1,      // Can write files
  DeleteFiles = 1 << 2,     // Can delete files
  ListDirectories = 1 << 3, // Can list directory contents

  // Network
  HttpGet = 1 << 4,   // Can make HTTP GET requests
  HttpPost = 1 << 5,  // Can make HTTP POST requests
  Sockets = 1 << 6,   // Can open raw sockets
  WebSocket = 1 << 7, // Can use WebSockets

  // System
  ExecuteProcess = 1 << 8, // Can spawn processes
  Environment = 1 << 9,    // Can access environment variables
  LoadLibrary = 1 << 10,   // Can load native libraries (dangerous!)

  // Lua features
  Coroutines = 1 << 11,      // Can use coroutines
  Debug = 1 << 12,           // Can use debug library
  Loadstring = 1 << 13,      // Can use loadstring/load
  Bytecode = 1 << 14,        // Can load bytecode (security risk!)
  MetaProgramming = 1 << 15, // Can modify metatables

  // Engine access
  EngineAPI = 1 << 16,  // Can use engine API
  EditorAPI = 1 << 17,  // Can use editor-only APIs
  NativeCode = 1 << 18, // Can call native C functions
  Reflection = 1 << 19, // Can inspect engine internals

  // Presets
  Untrusted = None,
  Modding = ReadFiles | Coroutines | EngineAPI,
  Trusted = ReadFiles | WriteFiles | Coroutines | Debug | Loadstring |
            MetaProgramming | EngineAPI,
  Full = 0xFFFFFFFF
};

inline SandboxPermission operator|(SandboxPermission a, SandboxPermission b) {
  return static_cast<SandboxPermission>(static_cast<uint32_t>(a) |
                                        static_cast<uint32_t>(b));
}

inline SandboxPermission operator&(SandboxPermission a, SandboxPermission b) {
  return static_cast<SandboxPermission>(static_cast<uint32_t>(a) &
                                        static_cast<uint32_t>(b));
}

inline bool HasPermission(SandboxPermission flags, SandboxPermission perm) {
  return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(perm)) != 0;
}

// ============================================================================
// Sandbox Configuration
// ============================================================================

struct SandboxLimits {
  // Memory limits
  size_t maxMemoryBytes{64 * 1024 * 1024};  // 64MB default
  size_t softMemoryLimit{48 * 1024 * 1024}; // Warning threshold

  // Execution limits
  double maxExecutionTimeMs{100.0};   // Per-call limit
  double maxTotalTimeMs{1000.0};      // Total time per frame
  uint64_t maxInstructions{10000000}; // Instruction count limit

  // Resource limits
  size_t maxStringLength{1024 * 1024}; // 1MB max string
  size_t maxTableSize{100000};         // Max table entries
  int maxStackDepth{200};              // Recursion limit
  int maxUpvalues{60};                 // Max upvalues per function
  size_t maxCoroutines{100};           // Max concurrent coroutines

  // File system limits
  size_t maxFileSize{10 * 1024 * 1024}; // 10MB max file read/write
  int maxOpenFiles{10};                 // Max concurrent open files

  // Network limits
  size_t maxRequestSize{1024 * 1024};       // 1MB max request body
  size_t maxResponseSize{10 * 1024 * 1024}; // 10MB max response
  int maxConnections{5};                    // Max concurrent connections
  double networkTimeoutMs{30000.0};         // 30 second timeout
};

// ============================================================================
// Sandbox Violation
// ============================================================================

struct SandboxViolation {
  enum class Type {
    MemoryLimit,
    TimeLimit,
    InstructionLimit,
    PermissionDenied,
    ResourceLimit,
    SecurityViolation
  };

  Type type;
  std::string message;
  std::string location; // File:line if available
  std::chrono::system_clock::time_point timestamp;
  SandboxPermission attemptedPermission{SandboxPermission::None};
};

using SandboxViolationCallback = std::function<void(const SandboxViolation &)>;

// ============================================================================
// Lua Sandbox
// ============================================================================

/**
 * @brief Provides sandboxing for Lua scripts
 *
 * The sandbox enforces:
 * - Memory limits (per-script and total)
 * - Execution time limits (per-call and per-frame)
 * - Instruction count limits
 * - Permission-based API access
 * - Restricted standard library
 * - Isolated environments per script
 *
 * Security Model:
 * - Scripts run in isolated environments
 * - Each script gets a whitelist of allowed functions
 * - Dangerous functions are replaced with safe alternatives
 * - Memory and time are strictly monitored
 *
 * Usage:
 * ```cpp
 * LuaSandbox sandbox(engine);
 * sandbox.SetPermissions(SandboxPermission::Modding);
 * sandbox.SetLimits(limits);
 *
 * // Create sandboxed environment
 * LuaRef env = sandbox.CreateEnvironment("MyScript");
 *
 * // Execute in sandbox
 * sandbox.Execute(env, "print('Hello!')");
 * ```
 */
class BE_API LuaSandbox {
public:
  // ===== Lifecycle =====

  explicit LuaSandbox(LuaEngine *engine);
  ~LuaSandbox();

  // Non-copyable
  LuaSandbox(const LuaSandbox &) = delete;
  LuaSandbox &operator=(const LuaSandbox &) = delete;

  /// Initialize the sandbox system
  void Initialize();

  /// Shutdown and cleanup
  void Shutdown();

  /// Check if sandbox is enabled
  NODISCARD bool IsEnabled() const { return m_Enabled; }

  /// Enable/disable sandboxing
  void SetEnabled(bool enabled) { m_Enabled = enabled; }

  // ===== Configuration =====

  /// Set global permissions
  void SetPermissions(SandboxPermission permissions);

  /// Get current permissions
  NODISCARD SandboxPermission GetPermissions() const { return m_Permissions; }

  /// Check if permission is granted
  NODISCARD bool HasPermission(SandboxPermission permission) const;

  /// Set resource limits
  void SetLimits(const SandboxLimits &limits);

  /// Get current limits
  NODISCARD const SandboxLimits &GetLimits() const { return m_Limits; }

  /// Set violation callback
  void SetViolationCallback(SandboxViolationCallback callback) {
    m_ViolationCallback = std::move(callback);
  }

  // ===== Path Restrictions =====

  /// Add an allowed read path
  void AddReadPath(const std::filesystem::path &path);

  /// Add an allowed write path
  void AddWritePath(const std::filesystem::path &path);

  /// Remove a path from allowed list
  void RemovePath(const std::filesystem::path &path);

  /// Clear all path restrictions
  void ClearPaths();

  /// Check if path is readable
  NODISCARD bool CanReadPath(const std::filesystem::path &path) const;

  /// Check if path is writable
  NODISCARD bool CanWritePath(const std::filesystem::path &path) const;

  // ===== Module Restrictions =====

  /// Allow a Lua module to be required
  void AllowModule(const std::string &moduleName);

  /// Block a Lua module
  void BlockModule(const std::string &moduleName);

  /// Check if module is allowed
  NODISCARD bool IsModuleAllowed(const std::string &moduleName) const;

  /// Get list of allowed modules
  NODISCARD std::vector<std::string> GetAllowedModules() const;

  // ===== Environment Management =====

  /// Create a sandboxed environment for a script
  /// @param name Name for the environment (for debugging)
  /// @return Reference to the environment table
  NODISCARD LuaRef CreateEnvironment(const std::string &name);

  /// Create environment with custom permissions
  NODISCARD LuaRef CreateEnvironment(const std::string &name,
                                     SandboxPermission permissions);

  /// Destroy an environment
  void DestroyEnvironment(const LuaRef &env);

  /// Get environment by name
  NODISCARD LuaRef GetEnvironment(const std::string &name) const;

  // ===== Execution =====

  /// Execute code in a sandboxed environment
  /// @param env Environment reference
  /// @param code Lua code to execute
  /// @return Error code
  NODISCARD LuaErrorCode Execute(const LuaRef &env, const std::string &code);

  /// Execute file in a sandboxed environment
  NODISCARD LuaErrorCode ExecuteFile(const LuaRef &env,
                                     const std::filesystem::path &path);

  /// Call a function with sandbox protection
  NODISCARD LuaErrorCode ProtectedCall(int nargs, int nresults);

  // ===== Memory Monitoring =====

  /// Get current memory usage
  NODISCARD size_t GetMemoryUsage() const;

  /// Get peak memory usage
  NODISCARD size_t GetPeakMemoryUsage() const { return m_PeakMemory; }

  /// Check if memory limit is exceeded
  NODISCARD bool IsMemoryLimitExceeded() const;

  /// Force garbage collection
  void ForceGC();

  // ===== Time Monitoring =====

  /// Start execution timer
  void StartTimer();

  /// Check execution time
  NODISCARD double GetExecutionTime() const;

  /// Check if time limit exceeded
  NODISCARD bool IsTimeLimitExceeded() const;

  /// Reset timer
  void ResetTimer();

  // ===== Instruction Counting =====

  /// Get current instruction count
  NODISCARD uint64_t GetInstructionCount() const { return m_InstructionCount; }

  /// Reset instruction count
  void ResetInstructionCount() { m_InstructionCount = 0; }

  // ===== Violations =====

  /// Get violation history
  NODISCARD const std::vector<SandboxViolation> &GetViolations() const {
    return m_Violations;
  }

  /// Clear violation history
  void ClearViolations() { m_Violations.clear(); }

  /// Get violation count
  NODISCARD size_t GetViolationCount() const { return m_Violations.size(); }

  // ===== Statistics =====

  struct Stats {
    size_t totalExecutions{0};
    size_t successfulExecutions{0};
    size_t blockedOperations{0};
    size_t memoryViolations{0};
    size_t timeViolations{0};
    size_t permissionViolations{0};
    double totalExecutionTime{0.0};
    size_t totalMemoryAllocated{0};
  };

  NODISCARD Stats GetStats() const { return m_Stats; }
  void ResetStats() { m_Stats = Stats{}; }

private:
  // ===== Internal Methods =====

  void SetupHooks();
  void SetupSafeGlobals(lua_State *L, LuaRef &env, SandboxPermission perms);
  void SetupSafeStdLib(lua_State *L, LuaRef &env, SandboxPermission perms);
  void SetupSafeIO(lua_State *L, LuaRef &env);
  void SetupSafeOS(lua_State *L, LuaRef &env);
  void SetupSafeString(lua_State *L, LuaRef &env);
  void SetupSafeTable(lua_State *L, LuaRef &env);
  void SetupSafeMath(lua_State *L, LuaRef &env);

  void ReportViolation(SandboxViolation::Type type, const std::string &message,
                       SandboxPermission attempted = SandboxPermission::None);

  NODISCARD bool CheckStringLength(size_t length) const;
  NODISCARD bool CheckTableSize(size_t size) const;

  // ===== Lua Hooks =====

  static void InstructionHook(lua_State *L, lua_Debug *ar);
  static void *SandboxedAllocator(void *ud, void *ptr, size_t osize,
                                  size_t nsize);

  // ===== Safe Replacements =====

  static int Safe_Print(lua_State *L);
  static int Safe_Require(lua_State *L);
  static int Safe_Loadstring(lua_State *L);
  static int Safe_Loadfile(lua_State *L);
  static int Safe_Dofile(lua_State *L);
  static int Safe_Pcall(lua_State *L);
  static int Safe_Xpcall(lua_State *L);
  static int Safe_Error(lua_State *L);
  static int Safe_Assert(lua_State *L);

  // File operations
  static int Safe_IO_Open(lua_State *L);
  static int Safe_IO_Read(lua_State *L);
  static int Safe_IO_Write(lua_State *L);
  static int Safe_IO_Lines(lua_State *L);

  // OS operations
  static int Safe_OS_Time(lua_State *L);
  static int Safe_OS_Date(lua_State *L);
  static int Safe_OS_Clock(lua_State *L);
  static int Safe_OS_Difftime(lua_State *L);
  // OS functions that are blocked
  static int Blocked_OS_Execute(lua_State *L);
  static int Blocked_OS_Exit(lua_State *L);
  static int Blocked_OS_Remove(lua_State *L);
  static int Blocked_OS_Rename(lua_State *L);
  static int Blocked_OS_Setenv(lua_State *L);
  static int Blocked_OS_Getenv(lua_State *L);

  static LuaSandbox *GetFromRegistry(lua_State *L);
  void StoreInRegistry();

  LuaEngine *m_Engine{nullptr};
  lua_State *m_State{nullptr};
  bool m_Enabled{true};

  // Permissions and limits
  SandboxPermission m_Permissions{SandboxPermission::Untrusted};
  SandboxLimits m_Limits;

  // Path restrictions
  std::unordered_set<std::string> m_AllowedReadPaths;
  std::unordered_set<std::string> m_AllowedWritePaths;

  // Module restrictions
  std::unordered_set<std::string> m_AllowedModules;
  std::unordered_set<std::string> m_BlockedModules;

  // Environments
  struct Environment {
    std::string name;
    LuaRef envRef;
    SandboxPermission permissions;
    size_t memoryUsage{0};
    double executionTime{0.0};
  };
  std::unordered_map<std::string, Environment> m_Environments;

  // Monitoring
  std::atomic<size_t> m_MemoryUsage{0};
  size_t m_PeakMemory{0};
  std::atomic<uint64_t> m_InstructionCount{0};
  std::chrono::steady_clock::time_point m_TimerStart;
  bool m_TimerRunning{false};

  // Violations
  std::vector<SandboxViolation> m_Violations;
  SandboxViolationCallback m_ViolationCallback;
  mutable std::mutex m_ViolationMutex;

  // Statistics
  Stats m_Stats;

  // Registry key
  static constexpr const char *REGISTRY_KEY = "BeEngine.LuaSandbox";
};

// ============================================================================
// Sandbox Presets
// ============================================================================

/**
 * @brief Factory for common sandbox configurations
 */
class BE_API SandboxPresets {
public:
  /// Create preset for untrusted user scripts (mods)
  static SandboxLimits Untrusted();

  /// Create preset for semi-trusted scripts
  static SandboxLimits Modding();

  /// Create preset for trusted engine scripts
  static SandboxLimits Trusted();

  /// Create preset for development/debug
  static SandboxLimits Development();

  /// Create preset for performance-critical code
  static SandboxLimits HighPerformance();
};

} // namespace BeEngine
