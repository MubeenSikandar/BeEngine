// LuaEngine.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Core/ScriptEngine.hpp>
#include <Scripting/Lua/LuaTypeDefinitions.hpp>
#include <unordered_set>

// Forward declarations
struct lua_State;

namespace BeEngine {

// Forward declarations
class LuaInstance;
class LuaSandbox;
class LuaDebugger;
class LuaCoroutineManager;
class LuaClassSystem;

/**
 * @brief Lua script engine implementation
 *
 * Provides a complete Lua scripting environment for BeEngine:
 * - LuaJIT support for high performance
 * - Sandboxing for safe mod/user script execution
 * - Hot reload support
 * - Coroutine/async support
 * - Full engine API bindings
 * - Debugger integration
 *
 * Thread Safety:
 * - Each LuaEngine owns its own lua_State
 * - Script instances should only be accessed from the main thread
 * - Use CreateWorkerState() for background tasks
 */
class BE_API LuaEngine : public ScriptEngine {
public:
  // ===== Configuration =====

  struct Config {
    // Memory
    size_t maxMemoryBytes{128 * 1024 * 1024}; // 128MB default
    size_t initialStackSize{1024};
    size_t maxStackSize{65536};

    // Execution
    double maxExecutionTimeMs{100.0}; // Per-call limit
    bool enableJIT{true};             // Use LuaJIT if available
    bool enableDebugInfo{true};       // Include debug symbols

    // Features
    bool enableSandbox{true};
    bool enableCoroutines{true};
    bool enableHotReload{true};
    bool enableProfiling{false};

    // Paths
    std::vector<std::filesystem::path> packagePaths;
    std::vector<std::filesystem::path> cPackagePaths;

    // Callbacks
    LuaPrintHandler printHandler;
    LuaErrorHandler errorHandler;
  };

  // ===== Lifecycle =====

  LuaEngine();
  explicit LuaEngine(const Config &config);
  ~LuaEngine() override;

  // Non-copyable
  LuaEngine(const LuaEngine &) = delete;
  LuaEngine &operator=(const LuaEngine &) = delete;

  // Moveable
  LuaEngine(LuaEngine &&other) noexcept;
  LuaEngine &operator=(LuaEngine &&other) noexcept;

  // ===== ScriptEngine Interface =====

  bool Initialize() override;
  void Shutdown() override;
  NODISCARD bool IsInitialized() const override { return m_Initialized; }

  // Script loading
  bool LoadScript(const std::filesystem::path &filePath) override;
  bool LoadScriptFromString(const std::string &name,
                            const std::string &code) override;
  void UnloadScript(const std::string &scriptName) override;
  void UnloadAllScripts() override;
  NODISCARD bool IsScriptLoaded(const std::string &scriptName) const override;
  NODISCARD std::vector<std::string> GetLoadedScripts() const override;

  NODISCARD bool ValidateScript(const std::filesystem::path &filePath,
                                std::vector<ScriptError> &outErrors) override;
  bool CompileScript(const std::filesystem::path &filePath) override;

  // Instance management
  Ref<ScriptInstance> CreateInstance(const std::string &scriptName,
                                     Entity entity) override;
  void DestroyInstance(const Ref<ScriptInstance> &instance) override;

  // Metadata
  NODISCARD ScriptMetadata
  GetScriptMetadata(const std::string &scriptName) const override;
  NODISCARD std::vector<ScriptMetadata> GetAvailableScripts() const override;
  NODISCARD ScriptFieldMap
  GetScriptFields(const std::string &scriptName) const override;

  // Hot reload
  NODISCARD bool SupportsHotReload() const override { return true; }
  bool ReloadScript(const std::string &scriptName) override;
  void ReloadModifiedScripts() override;

  // Execution
  bool ExecuteString(const std::string &code, std::string &output) override;

  // ===== Lua-Specific API =====

  /// Get the main Lua state
  NODISCARD lua_State *GetState() const { return m_State; }

  /// Get the Lua version string
  NODISCARD const std::string &GetLuaVersion() const { return m_LuaVersion; }

  /// Check if using LuaJIT
  NODISCARD bool IsLuaJIT() const { return m_IsLuaJIT; }

  // ===== Code Execution =====

  /// Execute a Lua file
  NODISCARD LuaErrorCode ExecuteFile(const std::filesystem::path &filePath);

  /// Execute a Lua string with error handling
  NODISCARD LuaErrorCode ExecuteString(const std::string &code);

  /// Call a global function by name
  template <typename... Args>
  NODISCARD LuaErrorCode CallGlobal(const std::string &funcName,
                                    Args &&...args);

  /// Call a function reference
  NODISCARD LuaErrorCode CallFunction(const LuaRef &func, int numArgs = 0,
                                      int numResults = 0);

  /// Protected call with custom error handler
  NODISCARD LuaErrorCode ProtectedCall(int numArgs, int numResults,
                                       int errorHandler = 0);

  // ===== Global Variables =====

  /// Set a global variable
  void SetGlobal(const std::string &name, const LuaValue &value);

  /// Get a global variable
  NODISCARD LuaValue GetGlobal(const std::string &name) const;

  /// Check if a global exists
  NODISCARD bool HasGlobal(const std::string &name) const;

  /// Remove a global variable
  void RemoveGlobal(const std::string &name);

  // ===== Module System =====

  /// Register a module (table of functions)
  void RegisterModule(const std::string &name,
                      const std::vector<LuaMethodDesc> &functions);

  /// Pre-load a module (lazy loading)
  void PreloadModule(const std::string &name, LuaCFunction loader);

  /// Add to package.path
  void AddPackagePath(const std::filesystem::path &path);

  /// Add to package.cpath
  void AddCPackagePath(const std::filesystem::path &path);

  /// Require a module (like Lua's require())
  NODISCARD LuaRef RequireModule(const std::string &moduleName);

  // ===== Type Registration =====

  /// Register a C++ class for use in Lua
  void RegisterClass(const LuaClassDesc &desc);

  /// Create userdata with metatable
  template <typename T> T *CreateUserdata(const char *metatableName);

  /// Check and get userdata
  template <typename T> T *CheckUserdata(int index, const char *metatableName);

  /// Get userdata or nullptr
  template <typename T> T *ToUserdata(int index, const char *metatableName);

  // ===== Subsystems =====

  /// Get the sandbox manager
  NODISCARD LuaSandbox *GetSandbox() const { return m_Sandbox.get(); }

  /// Get the debugger
  NODISCARD LuaDebugger *GetDebugger() const { return m_Debugger.get(); }

  /// Get the coroutine manager
  NODISCARD LuaCoroutineManager *GetCoroutineManager() const {
    return m_CoroutineManager.get();
  }

  /// Get the class system
  NODISCARD LuaClassSystem *GetClassSystem() const {
    return m_ClassSystem.get();
  }

  // ===== Memory Management =====

  /// Get current memory usage (bytes)
  NODISCARD size_t GetMemoryUsage() const;

  /// Get peak memory usage (bytes)
  NODISCARD size_t GetPeakMemoryUsage() const { return m_PeakMemory; }

  /// Force garbage collection
  void CollectGarbage();

  /// Step garbage collection
  void StepGarbage(int stepSize = 100);

  /// Set memory limit (0 = unlimited)
  void SetMemoryLimit(size_t bytes) { m_MemoryLimit = bytes; }

  /// Get memory limit
  NODISCARD size_t GetMemoryLimit() const { return m_MemoryLimit; }

  // ===== Error Handling =====

  /// Get the last error message
  NODISCARD const std::string &GetLastError() const { return m_LastError; }

  /// Get stack traceback
  NODISCARD std::string GetTraceback(int level = 1) const;

  /// Push error handler function
  int PushErrorHandler();

  /// Set custom error handler
  void SetErrorHandler(LuaErrorHandler handler) {
    m_Config.errorHandler = std::move(handler);
  }

  /// Set print handler (for Lua's print function)
  void SetPrintHandler(LuaPrintHandler handler) {
    m_Config.printHandler = std::move(handler);
  }

  // ===== Stack Operations =====

  /// Get stack size
  NODISCARD int GetStackSize() const;

  /// Check stack space
  NODISCARD bool CheckStack(int extra) const;

  /// Pop values from stack
  void Pop(int n = 1);

  /// Push values
  void PushNil();
  void PushBool(bool value);
  void PushNumber(double value);
  void PushInteger(int64_t value);
  void PushString(const std::string &value);
  void PushLightUserdata(void *ptr);
  void PushValue(const LuaValue &value);

  /// Get values
  NODISCARD bool GetBool(int index) const;
  NODISCARD double GetNumber(int index) const;
  NODISCARD int64_t GetInteger(int index) const;
  NODISCARD std::string GetString(int index) const;
  NODISCARD void *GetLightUserdata(int index) const;
  NODISCARD LuaValue GetValue(int index) const;
  NODISCARD LuaType GetType(int index) const;

  /// Check types
  NODISCARD bool IsNil(int index) const;
  NODISCARD bool IsBool(int index) const;
  NODISCARD bool IsNumber(int index) const;
  NODISCARD bool IsString(int index) const;
  NODISCARD bool IsTable(int index) const;
  NODISCARD bool IsFunction(int index) const;
  NODISCARD bool IsUserdata(int index) const;

  // ===== Table Operations =====

  /// Create a new table
  void NewTable();

  /// Create table with pre-allocated space
  void CreateTable(int narr, int nrec);

  /// Set table field
  void SetField(int tableIndex, const std::string &key);

  /// Get table field
  void GetField(int tableIndex, const std::string &key);

  /// Raw table access
  void RawSet(int tableIndex);
  void RawGet(int tableIndex);
  void RawSetI(int tableIndex, int64_t n);
  void RawGetI(int tableIndex, int64_t n);

  /// Get table length
  NODISCARD size_t GetTableLength(int index) const;

  // ===== References =====

  /// Create a reference to stack top
  NODISCARD LuaRef CreateRef();

  /// Push referenced value
  void PushRef(const LuaRef &ref);

  /// Release a reference
  void ReleaseRef(LuaRef &ref);

  // ===== Profiling =====

  /// Enable/disable profiling
  void SetProfilingEnabled(bool enabled);

  /// Get profile data
  NODISCARD LuaStateProfile GetProfile() const;

  /// Reset profile data
  void ResetProfile();

  // ===== Worker States =====

  /// Create a new Lua state for background work
  NODISCARD lua_State *CreateWorkerState();

  /// Destroy a worker state
  void DestroyWorkerState(lua_State *worker);

  // ===== Debugging =====

  /// Get call stack
  NODISCARD std::vector<LuaStackFrame> GetCallStack(int maxDepth = 10) const;

  /// Get local variables at stack level
  NODISCARD std::vector<std::pair<std::string, LuaValue>>
  GetLocals(int level) const;

  /// Get upvalues for function at stack level
  NODISCARD std::vector<std::pair<std::string, LuaValue>>
  GetUpvalues(int level) const;

  // ===== Callbacks =====

  /// Called when script errors occur
  using ErrorCallback = std::function<void(const ScriptError &)>;

  void SetScriptErrorCallback(ErrorCallback callback) {
    m_ScriptErrorCallback = std::move(callback);
  }

protected:
  // ===== Internal Methods =====

  void SetupState();
  void SetupPackagePaths();
  void RegisterEngineAPI();
  void RegisterBuiltinTypes();

  NODISCARD LuaErrorCode LoadChunk(const std::string &chunk,
                                   const std::string &chunkName);
  NODISCARD ScriptFieldMap ExtractFields(const std::string &scriptName);
  void UpdateScriptMetadata(const std::string &scriptName,
                            const std::filesystem::path &filePath);

  static void *LuaAllocator(void *ud, void *ptr, size_t osize, size_t nsize);
  static int LuaPanicHandler(lua_State *L);
  static int LuaPrintOverride(lua_State *L);
  static int LuaErrorHandlerFunc(lua_State *L);

  void NotifyError(const ScriptError &error);

private:
  // Lua state
  lua_State *m_State{nullptr};
  Config m_Config;

  // Version info
  std::string m_LuaVersion;
  bool m_IsLuaJIT{false};

  // Memory tracking
  std::atomic<size_t> m_MemoryUsage{0};
  size_t m_PeakMemory{0};
  size_t m_MemoryLimit{0};

  // Script tracking
  struct LoadedScript {
    std::string name;
    std::filesystem::path filePath;
    ScriptMetadata metadata;
    ScriptFieldMap fields;
    LuaRef classRef; // Reference to the script class table
    uint64_t lastModified{0};
    bool isValid{true};
  };
  std::unordered_map<std::string, LoadedScript> m_Scripts;
  mutable std::mutex m_ScriptsMutex;

  // Instance tracking
  std::unordered_set<LuaInstance *> m_Instances;
  mutable std::mutex m_InstancesMutex;

  // Subsystems
  Scope<LuaSandbox> m_Sandbox;
  Scope<LuaDebugger> m_Debugger;
  Scope<LuaCoroutineManager> m_CoroutineManager;
  Scope<LuaClassSystem> m_ClassSystem;

  // Error handling
  std::string m_LastError;
  int m_ErrorHandlerRef{-2}; // LUA_NOREF
  ErrorCallback m_ScriptErrorCallback;

  // Profiling
  LuaStateProfile m_Profile;
  bool m_ProfileEnabled{false};

  // Worker states
  std::vector<lua_State *> m_WorkerStates;
  std::mutex m_WorkerMutex;
};

// ============================================================================
// Template Implementations
// ============================================================================

template <typename T> T *LuaEngine::CreateUserdata(const char *metatableName) {
  // Implementation in cpp file - this is a forward declaration
  // Actual implementation uses lua_newuserdata and luaL_setmetatable
  return nullptr; // Placeholder
}

template <typename T>
T *LuaEngine::CheckUserdata(int index, const char *metatableName) {
  // Implementation in cpp file
  return nullptr; // Placeholder
}

template <typename T>
T *LuaEngine::ToUserdata(int index, const char *metatableName) {
  // Implementation in cpp file
  return nullptr; // Placeholder
}

template <typename... Args>
LuaErrorCode LuaEngine::CallGlobal(const std::string &funcName,
                                   Args &&...args) {
  // Push function
  GetField(-10002, funcName); // LUA_GLOBALSINDEX in LuaJIT

  if (!IsFunction(-1)) {
    Pop(1);
    m_LastError = "Global '" + funcName + "' is not a function";
    return LuaErrorCode::RuntimeError;
  }

  // Push arguments
  int numArgs = 0;
  (PushValue(std::forward<Args>(args)), ..., ++numArgs);

  // Call
  return ProtectedCall(numArgs, 0, 0);
}

} // namespace BeEngine
