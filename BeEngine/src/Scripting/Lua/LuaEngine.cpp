// LuaEngine.cpp

#include <PCH/BeEnginePCH.hpp>
#include <algorithm>
#include <fstream>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

// LuaJIT detection
#ifdef LUAJIT_VERSION
#define BE_USING_LUAJIT 1
#include <luajit.h>
#else
#define BE_USING_LUAJIT 0
#endif

namespace BeEngine {
// ============================================================================
// Constructor / Destructor
// ============================================================================

LuaEngine::LuaEngine()
    : ScriptEngine(ScriptLanguage::Lua, "LuaEngine"), m_Config() {}

LuaEngine::LuaEngine(const Config &config)
    : ScriptEngine(ScriptLanguage::Lua, "LuaEngine"), m_Config(config) {}

LuaEngine::~LuaEngine() { Shutdown(); }

LuaEngine::LuaEngine(LuaEngine &&other) noexcept
    : ScriptEngine(std::move(other)), m_State(other.m_State),
      m_Config(std::move(other.m_Config)),
      m_LuaVersion(std::move(other.m_LuaVersion)), m_IsLuaJIT(other.m_IsLuaJIT),
      m_MemoryUsage(other.m_MemoryUsage.load()),
      m_PeakMemory(other.m_PeakMemory), m_MemoryLimit(other.m_MemoryLimit),
      m_Scripts(std::move(other.m_Scripts)),
      m_Instances(std::move(other.m_Instances)),
      m_Sandbox(std::move(other.m_Sandbox)),
      m_Debugger(std::move(other.m_Debugger)),
      m_CoroutineManager(std::move(other.m_CoroutineManager)),
      m_ClassSystem(std::move(other.m_ClassSystem)),
      m_LastError(std::move(other.m_LastError)),
      m_ErrorHandlerRef(other.m_ErrorHandlerRef),
      m_ScriptErrorCallback(std::move(other.m_ScriptErrorCallback)),
      m_Profile(std::move(other.m_Profile)),
      m_ProfileEnabled(other.m_ProfileEnabled),
      m_WorkerStates(std::move(other.m_WorkerStates)) {
  other.m_State = nullptr;
  other.m_ErrorHandlerRef = LuaRef::NoRef;
}

LuaEngine &LuaEngine::operator=(LuaEngine &&other) noexcept {
  if (this != &other) {
    Shutdown();

    ScriptEngine::operator=(std::move(other));

    m_State = other.m_State;
    m_Config = std::move(other.m_Config);
    m_LuaVersion = std::move(other.m_LuaVersion);
    m_IsLuaJIT = other.m_IsLuaJIT;
    m_MemoryUsage = other.m_MemoryUsage.load();
    m_PeakMemory = other.m_PeakMemory;
    m_MemoryLimit = other.m_MemoryLimit;
    m_Scripts = std::move(other.m_Scripts);
    m_Instances = std::move(other.m_Instances);
    m_Sandbox = std::move(other.m_Sandbox);
    m_Debugger = std::move(other.m_Debugger);
    m_CoroutineManager = std::move(other.m_CoroutineManager);
    m_ClassSystem = std::move(other.m_ClassSystem);
    m_LastError = std::move(other.m_LastError);
    m_ErrorHandlerRef = other.m_ErrorHandlerRef;
    m_ScriptErrorCallback = std::move(other.m_ScriptErrorCallback);
    m_Profile = std::move(other.m_Profile);
    m_ProfileEnabled = other.m_ProfileEnabled;
    m_WorkerStates = std::move(other.m_WorkerStates);

    other.m_State = nullptr;
    other.m_ErrorHandlerRef = LuaRef::NoRef;
  }
  return *this;
}

// ============================================================================
// Lifecycle
// ============================================================================

bool LuaEngine::Initialize() {
  if (m_Initialized) {
    BE_CORE_WARN("[LuaEngine] Already initialized");
    return true;
  }

  BE_CORE_INFO("[LuaEngine] Initializing...");

  // Create Lua state with custom allocator for memory tracking
  m_State = lua_newstate(LuaAllocator, this);
  if (m_State == nullptr) {
    BE_CORE_ERROR("[LuaEngine] Failed to create Lua state");
    return false;
  }

  // Set panic handler
  lua_atpanic(m_State, LuaPanicHandler);

  // Open standard libraries
  luaL_openlibs(m_State);

  // Detect LuaJIT
#if BE_USING_LUAJIT
  m_IsLuaJIT = true;
  m_LuaVersion = LUAJIT_VERSION;
  BE_CORE_INFO("[LuaEngine] Using LuaJIT: {}", m_LuaVersion);

  // Enable JIT if configured
  if (m_Config.enableJIT) {
    luaJIT_setmode(m_State, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
  } else {
    luaJIT_setmode(m_State, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_OFF);
  }
#else
  m_IsLuaJIT = false;
  m_LuaVersion = LUA_VERSION;
  BE_CORE_INFO("[LuaEngine] Using standard Lua: {}", m_LuaVersion);
#endif

  // Setup state
  SetupState();
  SetupPackagePaths();

  // Register engine API
  RegisterEngineAPI();
  RegisterBuiltinTypes();

  // Initialize subsystems
  if (m_Config.enableSandbox) {
    m_Sandbox = CreateScope<LuaSandbox>(this);
    m_Sandbox->Initialize();
  }

  if (m_Config.enableCoroutines) {
    m_CoroutineManager = CreateScope<LuaCoroutineManager>(this);
    m_CoroutineManager->Initialize();
  }

  m_ClassSystem = CreateScope<LuaClassSystem>(this);
  m_ClassSystem->Initialize();

  if (m_Config.enableDebugInfo) {
    m_Debugger = CreateScope<LuaDebugger>(this);
    m_Debugger->Initialize();
  }

  // Set memory limit
  if (m_Config.maxMemoryBytes > 0) {
    m_MemoryLimit = m_Config.maxMemoryBytes;
  }

  m_Initialized = true;
  BE_CORE_INFO("[LuaEngine] Initialized successfully");

  return true;
}

void LuaEngine::Shutdown() {
  if (!m_Initialized) {
    return;
  }

  BE_CORE_INFO("[LuaEngine] Shutting down...");

  // Destroy all instances first
  {
    std::lock_guard<std::mutex> lock(m_InstancesMutex);
    m_Instances.clear();
  }

  // Unload all scripts
  UnloadAllScripts();

  // Shutdown subsystems
  if (m_Debugger) {
    m_Debugger->Shutdown();
    m_Debugger.reset();
  }

  if (m_CoroutineManager) {
    m_CoroutineManager->Shutdown();
    m_CoroutineManager.reset();
  }

  if (m_ClassSystem) {
    m_ClassSystem->Shutdown();
    m_ClassSystem.reset();
  }

  if (m_Sandbox) {
    m_Sandbox->Shutdown();
    m_Sandbox.reset();
  }

  // Destroy worker states
  {
    std::lock_guard<std::mutex> lock(m_WorkerMutex);
    for (auto *worker : m_WorkerStates) {
      lua_close(worker);
    }
    m_WorkerStates.clear();
  }

  // Release error handler reference
  if ((m_State != nullptr) && m_ErrorHandlerRef != LuaRef::NoRef) {
    luaL_unref(m_State, LUA_REGISTRYINDEX, m_ErrorHandlerRef);
    m_ErrorHandlerRef = LuaRef::NoRef;
  }

  // Close main Lua state
  if (m_State != nullptr) {
    lua_close(m_State);
    m_State = nullptr;
  }

  m_Initialized = false;
  BE_CORE_INFO("[LuaEngine] Shutdown complete");
}

// ============================================================================
// Script Loading
// ============================================================================

bool LuaEngine::LoadScript(const std::filesystem::path &filePath) {
  if (!m_Initialized || (m_State == nullptr)) {
    BE_CORE_ERROR("[LuaEngine] Engine not initialized");
    return false;
  }

  // Check file exists
  if (!std::filesystem::exists(filePath)) {
    BE_CORE_ERROR("[LuaEngine] Script file not found: {}", filePath.string());
    return false;
  }

  // Read file content
  std::ifstream file(filePath);
  if (!file.is_open()) {
    BE_CORE_ERROR("[LuaEngine] Failed to open script file: {}",
                  filePath.string());
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string code = buffer.str();
  file.close();

  // Get script name from filename (without extension)
  std::string scriptName = filePath.stem().string();

  // Load the script
  if (!LoadScriptFromString(scriptName, code)) {
    return false;
  }

  // Update metadata with file info
  UpdateScriptMetadata(scriptName, filePath);

  return true;
}

bool LuaEngine::LoadScriptFromString(const std::string &name,
                                     const std::string &code) {
  if (!m_Initialized || (m_State == nullptr)) {
    BE_CORE_ERROR("[LuaEngine] Engine not initialized");
    return false;
  }

  BE_CORE_TRACE("[LuaEngine] Loading script: {}", name);

  // Check if already loaded
  {
    std::lock_guard<std::mutex> lock(m_ScriptsMutex);
    if (m_Scripts.find(name) != m_Scripts.end()) {
      BE_CORE_WARN("[LuaEngine] Script '{}' already loaded, unloading first",
                   name);
      // Unload without lock (we already have it)
      m_Scripts.erase(name);
    }
  }

  // Load the chunk
  std::string chunkName = "@" + name + ".lua";
  LuaErrorCode err = LoadChunk(code, chunkName);
  if (err != LuaErrorCode::Ok) {
    BE_CORE_ERROR("[LuaEngine] Failed to load script '{}': {}", name,
                  m_LastError);
    return false;
  }

  // Execute the chunk to register the class
  err = ProtectedCall(0, 1, PushErrorHandler());
  Pop(1); // Pop error handler

  if (err != LuaErrorCode::Ok) {
    BE_CORE_ERROR("[LuaEngine] Failed to execute script '{}': {}", name,
                  m_LastError);
    return false;
  }

  // The script should have returned a class table (or registered via Class())
  // Store reference to the result
  LuaRef classRef;
  if (!IsNil(-1)) {
    classRef = CreateRef();
  }
  Pop(1);

  // Extract fields from the script
  ScriptFieldMap fields = ExtractFields(name);

  // Create loaded script entry
  LoadedScript script;
  script.name = name;
  script.classRef = classRef;
  script.fields = fields;
  script.metadata.name = name;
  script.metadata.className = name;
  script.metadata.language = ScriptLanguage::Lua;
  script.metadata.loadedAt =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  script.isValid = true;

  // Store script
  {
    std::lock_guard<std::mutex> lock(m_ScriptsMutex);
    m_Scripts[name] = std::move(script);
  }

  // Register with class system
  if (m_ClassSystem && classRef.IsValid()) {
    m_ClassSystem->RegisterClass(name, classRef);
  }

  BE_CORE_INFO("[LuaEngine] Loaded script: {}", name);
  return true;
}

void LuaEngine::UnloadScript(const std::string &scriptName) {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  auto it = m_Scripts.find(scriptName);
  if (it == m_Scripts.end()) {
    return;
  }

  // Release class reference
  if (it->second.classRef.IsValid()) {
    LuaRef ref = it->second.classRef;
    ref.Release();
  }

  // Unregister from class system
  if (m_ClassSystem) {
    m_ClassSystem->UnregisterClass(scriptName);
  }

  m_Scripts.erase(it);
  BE_CORE_TRACE("[LuaEngine] Unloaded script: {}", scriptName);
}

void LuaEngine::UnloadAllScripts() {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  for (auto &[name, script] : m_Scripts) {
    if (script.classRef.IsValid()) {
      script.classRef.Release();
    }
  }

  if (m_ClassSystem) {
    for (auto &[name, script] : m_Scripts) {
      m_ClassSystem->UnregisterClass(name);
    }
  }

  m_Scripts.clear();
  BE_CORE_TRACE("[LuaEngine] Unloaded all scripts");
}

bool LuaEngine::IsScriptLoaded(const std::string &scriptName) const {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);
  return m_Scripts.find(scriptName) != m_Scripts.end();
}

std::vector<std::string> LuaEngine::GetLoadedScripts() const {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  std::vector<std::string> result;
  result.reserve(m_Scripts.size());

  for (const auto &[name, script] : m_Scripts) {
    result.push_back(name);
  }

  return result;
}

bool LuaEngine::ValidateScript(const std::filesystem::path &filePath,
                               std::vector<ScriptError> &outErrors) {
  if (!std::filesystem::exists(filePath)) {
    ScriptError err;
    err.message = "File not found: " + filePath.string();
    err.file = filePath.string();
    err.language = ScriptLanguage::Lua;
    outErrors.push_back(err);
    return false;
  }

  // Read file
  std::ifstream file(filePath);
  if (!file.is_open()) {
    ScriptError err;
    err.message = "Failed to open file";
    err.file = filePath.string();
    err.language = ScriptLanguage::Lua;
    outErrors.push_back(err);
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string code = buffer.str();
  file.close();

  // Try to load (but not execute) the chunk
  std::string chunkName = "@" + filePath.filename().string();
  int result =
      luaL_loadbuffer(m_State, code.c_str(), code.size(), chunkName.c_str());

  if (result != LUA_OK) {
    ScriptError err;
    err.message = lua_tostring(m_State, -1);
    err.file = filePath.string();
    err.language = ScriptLanguage::Lua;

    // Try to parse line number from error message
    // Format: "filename:line: message"
    size_t colonPos = err.message.find(':');
    if (colonPos != std::string::npos) {
      size_t secondColon = err.message.find(':', colonPos + 1);
      if (secondColon != std::string::npos) {
        std::string lineStr =
            err.message.substr(colonPos + 1, secondColon - colonPos - 1);
        try {
          err.line = std::stoi(lineStr);
        } catch (...) {
          err.line = -1;
        }
      }
    }

    outErrors.push_back(err);
    lua_pop(m_State, 1);
    return false;
  }

  // Pop the loaded chunk (we don't want to execute it)
  lua_pop(m_State, 1);
  return true;
}

bool LuaEngine::CompileScript(const std::filesystem::path &filePath) {
  // For Lua, compile is the same as validate
  std::vector<ScriptError> errors;
  return ValidateScript(filePath, errors);
}

// ============================================================================
// Instance Management
// ============================================================================

Ref<ScriptInstance> LuaEngine::CreateInstance(const std::string &scriptName,
                                              Entity entity) {
  if (!m_Initialized) {
    BE_CORE_ERROR("[LuaEngine] Engine not initialized");
    return nullptr;
  }

  // Check script is loaded
  if (!IsScriptLoaded(scriptName)) {
    BE_CORE_ERROR("[LuaEngine] Script '{}' not loaded", scriptName);
    return nullptr;
  }

  // Create instance
  auto instance = CreateRef<LuaInstance>(this);
  instance->SetEntity(entity);
  instance->SetScriptName(scriptName);
  instance->SetLanguage(ScriptLanguage::Lua);

  // Initialize the instance
  if (!instance->Initialize(scriptName)) {
    BE_CORE_ERROR("[LuaEngine] Failed to initialize instance of '{}'",
                  scriptName);
    return nullptr;
  }

  // Track instance
  {
    std::lock_guard<std::mutex> lock(m_InstancesMutex);
    m_Instances.insert(instance.get());
  }

  return instance;
}

void LuaEngine::DestroyInstance(const Ref<ScriptInstance> &instance) {
  if (!instance) {
    return;
  }

  auto *luaInstance = dynamic_cast<LuaInstance *>(instance.get());
  if (luaInstance == nullptr) {
    return;
  }

  // Remove from tracking
  {
    std::lock_guard<std::mutex> lock(m_InstancesMutex);
    m_Instances.erase(luaInstance);
  }

  // The shared_ptr will handle cleanup
}

// ============================================================================
// Metadata
// ============================================================================

ScriptMetadata
LuaEngine::GetScriptMetadata(const std::string &scriptName) const {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  auto it = m_Scripts.find(scriptName);
  if (it != m_Scripts.end()) {
    return it->second.metadata;
  }

  return ScriptMetadata{};
}

std::vector<ScriptMetadata> LuaEngine::GetAvailableScripts() const {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  std::vector<ScriptMetadata> result;
  result.reserve(m_Scripts.size());

  for (const auto &[name, script] : m_Scripts) {
    result.push_back(script.metadata);
  }

  return result;
}

ScriptFieldMap LuaEngine::GetScriptFields(const std::string &scriptName) const {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  auto it = m_Scripts.find(scriptName);
  if (it != m_Scripts.end()) {
    return it->second.fields;
  }

  return ScriptFieldMap{};
}

// ============================================================================
// Hot Reload
// ============================================================================

bool LuaEngine::ReloadScript(const std::string &scriptName) {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  auto it = m_Scripts.find(scriptName);
  if (it == m_Scripts.end()) {
    BE_CORE_ERROR("[LuaEngine] Script '{}' not loaded", scriptName);
    return false;
  }

  std::filesystem::path filePath = it->second.filePath;
  if (filePath.empty() || !std::filesystem::exists(filePath)) {
    BE_CORE_ERROR("[LuaEngine] No file path for script '{}'", scriptName);
    return false;
  }

  BE_CORE_INFO("[LuaEngine] Reloading script: {}", scriptName);

  // Notify class system
  if (m_ClassSystem) {
    m_ClassSystem->PrepareForReload(scriptName);
  }

  // Store old class ref for cleanup
  LuaRef oldClassRef = it->second.classRef;

  // Read and load new content
  std::ifstream file(filePath);
  if (!file.is_open()) {
    BE_CORE_ERROR("[LuaEngine] Failed to open file for reload: {}",
                  filePath.string());
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string code = buffer.str();
  file.close();

  // Load new chunk
  std::string chunkName = "@" + scriptName + ".lua";
  LuaErrorCode err = LoadChunk(code, chunkName);
  if (err != LuaErrorCode::Ok) {
    BE_CORE_ERROR("[LuaEngine] Failed to reload script '{}': {}", scriptName,
                  m_LastError);
    return false;
  }

  // Execute
  err = ProtectedCall(0, 1, PushErrorHandler());
  Pop(1); // Pop error handler

  if (err != LuaErrorCode::Ok) {
    BE_CORE_ERROR("[LuaEngine] Failed to execute reloaded script '{}': {}",
                  scriptName, m_LastError);
    return false;
  }

  // Get new class ref
  LuaRef newClassRef;
  if (!IsNil(-1)) {
    newClassRef = CreateRef();
  }
  Pop(1);

  // Update script entry
  it->second.classRef = newClassRef;
  it->second.fields = ExtractFields(scriptName);
  it->second.lastModified =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();

  // Release old class ref
  if (oldClassRef.IsValid()) {
    LuaRef ref = oldClassRef;
    ref.Release();
  }

  // Complete reload in class system
  if (m_ClassSystem) {
    m_ClassSystem->CompleteReload(scriptName, newClassRef);
  }

  BE_CORE_INFO("[LuaEngine] Successfully reloaded script: {}", scriptName);
  return true;
}

void LuaEngine::ReloadModifiedScripts() {
  std::vector<std::string> toReload;

  {
    std::lock_guard<std::mutex> lock(m_ScriptsMutex);

    for (auto &[name, script] : m_Scripts) {
      if (script.filePath.empty()) {
        continue;
      }

      if (!std::filesystem::exists(script.filePath)) {
        continue;
      }

      auto lastWrite = std::filesystem::last_write_time(script.filePath);
      auto lastWriteMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                             lastWrite.time_since_epoch())
                             .count();

      if (static_cast<uint64_t>(lastWriteMs) > script.lastModified) {
        toReload.push_back(name);
      }
    }
  }

  for (const auto &name : toReload) {
    ReloadScript(name);
  }
}

// ============================================================================
// Execution
// ============================================================================

bool LuaEngine::ExecuteString(const std::string &code, std::string &output) {
  if (!m_Initialized || (m_State != nullptr)) {
    output = "Engine not initialized";
    return false;
  }

  LuaErrorCode err = ExecuteString(code);
  if (err != LuaErrorCode::Ok) {
    output = m_LastError;
    return false;
  }

  // Check if there's a return value on stack
  if (GetStackSize() > 0) {
    output = LuaValueToString(GetValue(-1));
    Pop(1);
  } else {
    output = "";
  }

  return true;
}

LuaErrorCode LuaEngine::ExecuteFile(const std::filesystem::path &filePath) {
  if (!m_Initialized || (m_State != nullptr)) {
    m_LastError = "Engine not initialized";
    return LuaErrorCode::RuntimeError;
  }

  int result = luaL_loadfile(m_State, filePath.string().c_str());
  if (result != LUA_OK) {
    m_LastError = lua_tostring(m_State, -1);
    lua_pop(m_State, 1);
    return static_cast<LuaErrorCode>(result);
  }

  return ProtectedCall(0, LUA_MULTRET, PushErrorHandler());
}

LuaErrorCode LuaEngine::ExecuteString(const std::string &code) {
  if (!m_Initialized || (m_State != nullptr)) {
    m_LastError = "Engine not initialized";
    return LuaErrorCode::RuntimeError;
  }

  int result = luaL_loadstring(m_State, code.c_str());
  if (result != LUA_OK) {
    m_LastError = lua_tostring(m_State, -1);
    lua_pop(m_State, 1);
    return static_cast<LuaErrorCode>(result);
  }

  return ProtectedCall(0, LUA_MULTRET, PushErrorHandler());
}

LuaErrorCode LuaEngine::CallFunction(const LuaRef &func, int numArgs,
                                     int numResults) {
  if (!func.IsValid()) {
    m_LastError = "Invalid function reference";
    return LuaErrorCode::InvalidReference;
  }

  PushRef(func);

  // Move function below arguments
  if (numArgs > 0) {
    lua_insert(m_State, -(numArgs + 1));
  }

  return ProtectedCall(numArgs, numResults, PushErrorHandler());
}

LuaErrorCode LuaEngine::ProtectedCall(int numArgs, int numResults,
                                      int errHandler) {
  int result = lua_pcall(m_State, numArgs, numResults, errHandler);

  if (result != LUA_OK) {
    if (lua_isstring(m_State, -1) != 0) {
      m_LastError = lua_tostring(m_State, -1);
    } else {
      m_LastError = LuaErrorCodeToString(static_cast<LuaErrorCode>(result));
    }
    lua_pop(m_State, 1);

    // Notify error callback
    ScriptError error;
    error.message = m_LastError;
    error.language = ScriptLanguage::Lua;
    NotifyError(error);

    return static_cast<LuaErrorCode>(result);
  }

  // Pop error handler if it was pushed
  if (errHandler != 0) {
    // Error handler is below results, need to remove it
    // Actually, lua_pcall removes it automatically on success
  }

  return LuaErrorCode::Ok;
}

// ============================================================================
// Global Variables
// ============================================================================

void LuaEngine::SetGlobal(const std::string &name, const LuaValue &value) {
  PushValue(value);
  lua_setglobal(m_State, name.c_str());
}

LuaValue LuaEngine::GetGlobal(const std::string &name) const {
  lua_getglobal(m_State, name.c_str());
  LuaValue value = GetValue(-1);
  lua_pop(m_State, 1);
  return value;
}

bool LuaEngine::HasGlobal(const std::string &name) const {
  lua_getglobal(m_State, name.c_str());
  bool exists = !lua_isnil(m_State, -1);
  lua_pop(m_State, 1);
  return exists;
}

void LuaEngine::RemoveGlobal(const std::string &name) {
  lua_pushnil(m_State);
  lua_setglobal(m_State, name.c_str());
}

// ============================================================================
// Module System
// ============================================================================

void LuaEngine::RegisterModule(const std::string &name,
                               const std::vector<LuaMethodDesc> &functions) {
  lua_newtable(m_State);

  for (const auto &func : functions) {
    lua_pushcfunction(m_State, func.func);
    lua_setfield(m_State, -2, func.name);
  }

  lua_setglobal(m_State, name.c_str());
}

void LuaEngine::PreloadModule(const std::string &name, LuaCFunction loader) {
  lua_getglobal(m_State, "package");
  lua_getfield(m_State, -1, "preload");
  lua_pushcfunction(m_State, loader);
  lua_setfield(m_State, -2, name.c_str());
  lua_pop(m_State, 2);
}

void LuaEngine::AddPackagePath(const std::filesystem::path &path) {
  lua_getglobal(m_State, "package");
  lua_getfield(m_State, -1, "path");

  std::string currentPath = lua_tostring(m_State, -1);
  std::string newPath = currentPath + ";" + path.string() + "/?.lua;" +
                        path.string() + "/?/init.lua";

  lua_pop(m_State, 1);
  lua_pushstring(m_State, newPath.c_str());
  lua_setfield(m_State, -2, "path");
  lua_pop(m_State, 1);
}

void LuaEngine::AddCPackagePath(const std::filesystem::path &path) {
  lua_getglobal(m_State, "package");
  lua_getfield(m_State, -1, "cpath");

  std::string currentPath = lua_tostring(m_State, -1);

#ifdef _WIN32
  std::string ext = ".dll";
#elif __APPLE__
  std::string ext = ".dylib";
#else
  std::string ext = ".so";
#endif

  std::string newPath = currentPath + ";" + path.string() + "/*" + ext;

  lua_pop(m_State, 1);
  lua_pushstring(m_State, newPath.c_str());
  lua_setfield(m_State, -2, "cpath");
  lua_pop(m_State, 1);
}

LuaRef LuaEngine::RequireModule(const std::string &moduleName) {
  lua_getglobal(m_State, "require");
  lua_pushstring(m_State, moduleName.c_str());

  if (lua_pcall(m_State, 1, 1, 0) != LUA_OK) {
    m_LastError = lua_tostring(m_State, -1);
    lua_pop(m_State, 1);
    return LuaRef{};
  }

  return CreateRef();
}

// ============================================================================
// Type Registration
// ============================================================================

void LuaEngine::RegisterClass(const LuaClassDesc &desc) {
  // Create metatable
  luaL_newmetatable(m_State, desc.name);

  // Set __index to itself
  lua_pushvalue(m_State, -1);
  lua_setfield(m_State, -2, "__index");

  // Register methods
  for (const auto &method : desc.methods) {
    lua_pushcfunction(m_State, method.func);
    lua_setfield(m_State, -2, method.name);
  }

  // Register destructor
  if (desc.destructor != nullptr) {
    lua_pushcfunction(m_State, desc.destructor);
    lua_setfield(m_State, -2, "__gc");
  }

  // Pop metatable
  lua_pop(m_State, 1);

  // Create constructor table
  if (desc.constructor != nullptr) {
    lua_newtable(m_State);

    // Set metatable with __call
    lua_newtable(m_State);
    lua_pushcfunction(m_State, desc.constructor);
    lua_setfield(m_State, -2, "__call");
    lua_setmetatable(m_State, -2);

    // Add static methods
    for (const auto &method : desc.staticMethods) {
      lua_pushcfunction(m_State, method.func);
      lua_setfield(m_State, -2, method.name);
    }

    lua_setglobal(m_State, desc.name);
  }
}

// ============================================================================
// Memory Management
// ============================================================================

size_t LuaEngine::GetMemoryUsage() const { return m_MemoryUsage.load(); }

void LuaEngine::CollectGarbage() {
  if (m_State != nullptr) {
    lua_gc(m_State, LUA_GCCOLLECT, 0);
  }
}

void LuaEngine::StepGarbage(int stepSize) {
  if (m_State != nullptr) {
    lua_gc(m_State, LUA_GCSTEP, stepSize);
  }
}

// ============================================================================
// Error Handling
// ============================================================================

std::string LuaEngine::GetTraceback(int level) const {
  if (m_State == nullptr) {
    return "";
  }

  luaL_traceback(m_State, m_State, nullptr, level);
  std::string traceback = lua_tostring(m_State, -1);
  lua_pop(m_State, 1);
  return traceback;
}

int LuaEngine::PushErrorHandler() {
  lua_pushcfunction(m_State, LuaErrorHandlerFunc);
  return lua_gettop(m_State);
}

// ============================================================================
// Stack Operations
// ============================================================================

int LuaEngine::GetStackSize() const {
  return (m_State != nullptr) ? lua_gettop(m_State) : 0;
}

bool LuaEngine::CheckStack(int extra) const {
  return m_State ? lua_checkstack(m_State, extra) : false;
}

void LuaEngine::Pop(int n) {
  if (m_State != nullptr) {
    lua_pop(m_State, n);
  }
}

void LuaEngine::PushNil() {
  if (m_State != nullptr) {
    lua_pushnil(m_State);
  }
}

void LuaEngine::PushBool(bool value) {
  if (m_State != nullptr) {
    lua_pushboolean(m_State, value ? 1 : 0);
  }
}

void LuaEngine::PushNumber(double value) {
  if (m_State != nullptr) {
    lua_pushnumber(m_State, value);
  }
}

void LuaEngine::PushInteger(int64_t value) {
  if (m_State != nullptr) {
    lua_pushinteger(m_State, static_cast<lua_Integer>(value));
  }
}

void LuaEngine::PushString(const std::string &value) {
  if (m_State != nullptr) {
    lua_pushlstring(m_State, value.c_str(), value.size());
  }
}

void LuaEngine::PushLightUserdata(void *ptr) {
  if (m_State != nullptr) {
    lua_pushlightuserdata(m_State, ptr);
  }
}

void LuaEngine::PushValue(const LuaValue &value) {
  std::visit(
      [this](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          PushNil();
        } else if constexpr (std::is_same_v<T, bool>) {
          PushBool(arg);
        } else if constexpr (std::is_same_v<T, double>) {
          PushNumber(arg);
        } else if constexpr (std::is_same_v<T, std::string>) {
          PushString(arg);
        } else if constexpr (std::is_same_v<T, LuaRef>) {
          PushRef(arg);
        } else if constexpr (std::is_same_v<T, void *>) {
          PushLightUserdata(arg);
        }
      },
      value);
}

bool LuaEngine::GetBool(int index) const {
  return (m_State != nullptr) ? lua_toboolean(m_State, index) != 0 : false;
}

double LuaEngine::GetNumber(int index) const {
  return m_State != nullptr ? lua_tonumber(m_State, index) : 0.0;
}

int64_t LuaEngine::GetInteger(int index) const {
  return m_State != nullptr
             ? static_cast<int64_t>(lua_tointeger(m_State, index))
             : 0;
}

std::string LuaEngine::GetString(int index) const {
  if (m_State == nullptr) {
    return "";
  }

  size_t len = 0;
  const char *str = lua_tolstring(m_State, index, &len);
  return str ? std::string(str, len) : "";
}

void *LuaEngine::GetLightUserdata(int index) const {
  return m_State != nullptr ? lua_touserdata(m_State, index) : nullptr;
}

LuaValue LuaEngine::GetValue(int index) const {
  if (m_State == nullptr) {
    return std::monostate{};
  }

  switch (lua_type(m_State, index)) {
  case LUA_TNIL:
    return std::monostate{};
  case LUA_TBOOLEAN:
    return lua_toboolean(m_State, index) != 0;
  case LUA_TNUMBER:
    return lua_tonumber(m_State, index);
  case LUA_TSTRING: {
    size_t len = 0;
    const char *str = lua_tolstring(m_State, index, &len);
    return std::string(str, len);
  }
  case LUA_TLIGHTUSERDATA:
    return lua_touserdata(m_State, index);
  case LUA_TTABLE:
  case LUA_TFUNCTION:
  case LUA_TUSERDATA:
  case LUA_TTHREAD: {
    // Create a reference
    lua_pushvalue(m_State, index);
    int ref = luaL_ref(m_State, LUA_REGISTRYINDEX);
    return LuaRef{m_State, ref};
  }
  default:
    return std::monostate{};
  }
}

LuaType LuaEngine::GetType(int index) const {
  if (m_State == nullptr) {
    return LuaType::None;
  }
  return static_cast<LuaType>(lua_type(m_State, index));
}

bool LuaEngine::IsNil(int index) const {
  return m_State != nullptr ? lua_isnil(m_State, index) : true;
}

bool LuaEngine::IsBool(int index) const {
  return m_State != nullptr ? lua_isboolean(m_State, index) : false;
}

bool LuaEngine::IsNumber(int index) const {
  return m_State != nullptr ? lua_isnumber(m_State, index) != 0 : false;
}

bool LuaEngine::IsString(int index) const {
  return m_State != nullptr ? lua_isstring(m_State, index) != 0 : false;
}

bool LuaEngine::IsTable(int index) const {
  return m_State != nullptr ? lua_istable(m_State, index) : false;
}

bool LuaEngine::IsFunction(int index) const {
  return m_State != nullptr ? lua_isfunction(m_State, index) : false;
}

bool LuaEngine::IsUserdata(int index) const {
  return m_State != nullptr ? lua_isuserdata(m_State, index) != 0 : false;
}

// ============================================================================
// Table Operations
// ============================================================================

void LuaEngine::NewTable() {
  if (m_State != nullptr) {
    lua_newtable(m_State);
  }
}

void LuaEngine::CreateTable(int narr, int nrec) {
  if (m_State != nullptr) {
    lua_createtable(m_State, narr, nrec);
  }
}

void LuaEngine::SetField(int tableIndex, const std::string &key) {
  if (m_State != nullptr) {
    lua_setfield(m_State, tableIndex, key.c_str());
  }
}

void LuaEngine::GetField(int tableIndex, const std::string &key) {
  if (m_State != nullptr) {
    lua_getfield(m_State, tableIndex, key.c_str());
  }
}

void LuaEngine::RawSet(int tableIndex) {
  if (m_State != nullptr) {
    lua_rawset(m_State, tableIndex);
  }
}

void LuaEngine::RawGet(int tableIndex) {
  if (m_State != nullptr) {
    lua_rawget(m_State, tableIndex);
  }
}

void LuaEngine::RawSetI(int tableIndex, int64_t n) {
  if (m_State != nullptr) {
    lua_rawseti(m_State, tableIndex, static_cast<int>(n));
  }
}

void LuaEngine::RawGetI(int tableIndex, int64_t n) {
  if (m_State != nullptr) {
    lua_rawgeti(m_State, tableIndex, static_cast<int>(n));
  }
}

size_t LuaEngine::GetTableLength(int index) const {
  if (m_State == nullptr) {
    return 0;
  }

#if LUA_VERSION_NUM >= 502
  return lua_rawlen(m_State, index);
#else
  return lua_objlen(m_State, index);
#endif
}

// ============================================================================
// References
// ============================================================================

LuaRef LuaEngine::CreateRef() {
  if (m_State == nullptr) {
    return LuaRef{};
  }

  int ref = luaL_ref(m_State, LUA_REGISTRYINDEX);
  return LuaRef{m_State, ref};
}

void LuaEngine::PushRef(const LuaRef &ref) {
  if (m_State != nullptr && ref.IsValid()) {
    lua_rawgeti(m_State, LUA_REGISTRYINDEX, ref.ref);
  } else if (m_State != nullptr) {
    lua_pushnil(m_State);
  }
}

void LuaEngine::ReleaseRef(LuaRef &ref) { ref.Release(); }

// ============================================================================
// Profiling
// ============================================================================

void LuaEngine::SetProfilingEnabled(bool enabled) {
  m_ProfileEnabled = enabled;

  if (enabled) {
    // Setup profiling hooks
    // This would be implemented with lua_sethook
  } else {
    // Remove profiling hooks
  }
}

LuaStateProfile LuaEngine::GetProfile() const { return m_Profile; }

void LuaEngine::ResetProfile() { m_Profile = LuaStateProfile{}; }

// ============================================================================
// Worker States
// ============================================================================

lua_State *LuaEngine::CreateWorkerState() {
  lua_State *worker = lua_newstate(LuaAllocator, this);
  if (worker == nullptr) {
    return nullptr;
  }

  luaL_openlibs(worker);

  std::lock_guard<std::mutex> lock(m_WorkerMutex);
  m_WorkerStates.push_back(worker);

  return worker;
}

void LuaEngine::DestroyWorkerState(lua_State *worker) {
  if (worker == nullptr) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(m_WorkerMutex);
    auto it =
        std::ranges::find(m_WorkerStates.begin(), m_WorkerStates.end(), worker);
    if (it != m_WorkerStates.end()) {
      m_WorkerStates.erase(it);
    }
  }

  lua_close(worker);
}

// ============================================================================
// Debugging
// ============================================================================

std::vector<LuaStackFrame> LuaEngine::GetCallStack(int maxDepth) const {
  std::vector<LuaStackFrame> frames;

  if (m_State == nullptr) {
    return frames;
  }

  lua_Debug ar;
  for (int level = 0; level < maxDepth; ++level) {
    if (lua_getstack(m_State, level, &ar) == 0) {
      break;
    }

    lua_getinfo(m_State, "nSltu", &ar);

    LuaStackFrame frame;
    frame.info.name = ar.name ? ar.name : "";
    frame.info.source = ar.source ? ar.source : "";
    frame.info.currentLine = ar.currentline;
    frame.info.lineDefined = ar.linedefined;
    frame.info.lastLineDefined = ar.lastlinedefined;
    frame.info.what = ar.what ? ar.what : "";
    frame.info.numUpvalues = ar.nups;
#if LUA_VERSION_NUM >= 502
    frame.info.numParams = ar.nparams;
    frame.info.isVararg = ar.isvararg != 0;
#endif

    // Get locals
    frame.locals = GetLocals(level);
    frame.upvalues = GetUpvalues(level);

    frames.push_back(std::move(frame));
  }

  return frames;
}

std::vector<std::pair<std::string, LuaValue>>
LuaEngine::GetLocals(int level) const {
  std::vector<std::pair<std::string, LuaValue>> locals;

  if (m_State == nullptr) {
    return locals;
  }

  lua_Debug ar;
  if (lua_getstack(m_State, level, &ar) == 0) {
    return locals;
  }

  int n = 1;
  const char *name;
  while ((name = lua_getlocal(m_State, &ar, n)) != nullptr) {
    // Skip internal variables (starting with '(')
    if (name[0] != '(') {
      LuaValue value = GetValue(-1);
      locals.emplace_back(name, value);
    }
    lua_pop(m_State, 1);
    ++n;
  }

  return locals;
}

std::vector<std::pair<std::string, LuaValue>>
LuaEngine::GetUpvalues(int level) const {
  std::vector<std::pair<std::string, LuaValue>> upvalues;

  if (m_State == nullptr) {
    return upvalues;
  }

  lua_Debug ar;
  if (lua_getstack(m_State, level, &ar) == 0) {
    return upvalues;
  }

  // Get the function at this level
  lua_getinfo(m_State, "f", &ar);

  int n = 1;
  const char *name;
  while ((name = lua_getupvalue(m_State, -1, n)) != nullptr) {
    LuaValue value = GetValue(-1);
    upvalues.emplace_back(name, value);
    lua_pop(m_State, 1);
    ++n;
  }

  lua_pop(m_State, 1); // Pop the function

  return upvalues;
}

// ============================================================================
// Internal Methods
// ============================================================================

void LuaEngine::SetupState() {
  // Store engine pointer in registry for callbacks
  lua_pushlightuserdata(m_State, this);
  lua_setfield(m_State, LUA_REGISTRYINDEX, "BeEngine.LuaEngine");

  // Override print function
  lua_pushcfunction(m_State, LuaPrintOverride);
  lua_setglobal(m_State, "print");

  // Create error handler and store reference
  lua_pushcfunction(m_State, LuaErrorHandlerFunc);
  m_ErrorHandlerRef = luaL_ref(m_State, LUA_REGISTRYINDEX);
}

void LuaEngine::SetupPackagePaths() {
  // Add configured paths
  for (const auto &path : m_Config.packagePaths) {
    AddPackagePath(path);
  }

  for (const auto &path : m_Config.cPackagePaths) {
    AddCPackagePath(path);
  }
}

void LuaEngine::RegisterEngineAPI() {
  // This will be implemented by LuaBindings
  LuaBindings::RegisterAll(this);
}

void LuaEngine::RegisterBuiltinTypes() {
  // Register math types, entity wrapper, etc.
  // This is handled by LuaBindings::RegisterAll
}

LuaErrorCode LuaEngine::LoadChunk(const std::string &chunk,
                                  const std::string &chunkName) {
  int result =
      luaL_loadbuffer(m_State, chunk.c_str(), chunk.size(), chunkName.c_str());

  if (result != LUA_OK) {
    m_LastError = lua_tostring(m_State, -1);
    lua_pop(m_State, 1);
    return static_cast<LuaErrorCode>(result);
  }

  return LuaErrorCode::Ok;
}

ScriptFieldMap LuaEngine::ExtractFields(const std::string &scriptName) {
  ScriptFieldMap fields;

  // This would use the class system to extract fields from the script's class
  // table For now, return empty - will be properly implemented with
  // LuaClassSystem
  if (m_ClassSystem) {
    fields = m_ClassSystem->ExtractFields(scriptName);
  }

  return fields;
}

void LuaEngine::UpdateScriptMetadata(const std::string &scriptName,
                                     const std::filesystem::path &filePath) {
  std::lock_guard<std::mutex> lock(m_ScriptsMutex);

  auto it = m_Scripts.find(scriptName);
  if (it != m_Scripts.end()) {
    it->second.filePath = filePath;
    it->second.metadata.filePath = filePath.string();

    if (std::filesystem::exists(filePath)) {
      auto lastWrite = std::filesystem::last_write_time(filePath);
      it->second.lastModified =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              lastWrite.time_since_epoch())
              .count();
      it->second.metadata.lastModified = it->second.lastModified;
    }
  }
}

void LuaEngine::NotifyError(const ScriptError &error) {
  if (m_ScriptErrorCallback) {
    m_ScriptErrorCallback(error);
  }

  // Also call base class error reporting
  ReportError(error);
}

// ============================================================================
// Static Callbacks
// ============================================================================

void *LuaEngine::LuaAllocator(void *ud, void *ptr, size_t osize, size_t nsize) {
  auto *engine = static_cast<LuaEngine *>(ud);

  if (nsize == 0) {
    // Free
    if (ptr != nullptr) {
      engine->m_MemoryUsage -= osize;
      std::free(ptr);
    }
    return nullptr;
  }

  // Check memory limit
  if (engine->m_MemoryLimit > 0) {
    size_t newUsage = engine->m_MemoryUsage - osize + nsize;
    if (newUsage > engine->m_MemoryLimit) {
      // Memory limit exceeded
      return nullptr;
    }
  }

  void *newPtr = std::realloc(ptr, nsize);
  if (newPtr != nullptr) {
    engine->m_MemoryUsage = engine->m_MemoryUsage - osize + nsize;

    // Track peak memory
    size_t current = engine->m_MemoryUsage.load();
    if (current > engine->m_PeakMemory) {
      engine->m_PeakMemory = current;
    }
  }

  return newPtr;
}

int LuaEngine::LuaPanicHandler(lua_State *L) {
  const char *msg = lua_tostring(L, -1);
  BE_CORE_CRITICAL("[LuaEngine] PANIC: {}", msg ? msg : "unknown error");

  // This will cause the program to abort
  // In production, you might want to handle this differently
  return 0;
}

int LuaEngine::LuaPrintOverride(lua_State *L) {
  // Get engine from registry
  lua_getfield(L, LUA_REGISTRYINDEX, "BeEngine.LuaEngine");
  auto *engine = static_cast<LuaEngine *>(lua_touserdata(L, -1));
  lua_pop(L, 1);

  // Build message from all arguments
  std::string message;
  int n = lua_gettop(L);

  for (int i = 1; i <= n; ++i) {
    if (i > 1) {
      message += "\t";
    }

    const char *str = lua_tolstring(L, i, nullptr);
    if (str) {
      message += str;
    }
    lua_pop(L, 1); // Pop the string from luaL_tolstring
  }

  // Use custom handler or default logging
  if ((engine != nullptr) && engine->m_Config.printHandler) {
    engine->m_Config.printHandler(message);
  } else {
    BE_CORE_INFO("[Lua] {}", message);
  }

  return 0;
}

int LuaEngine::LuaErrorHandlerFunc(lua_State *L) {
  const char *msg = lua_tostring(L, 1);

  // Add traceback
  luaL_traceback(L, L, msg, 1);

  return 1;
}

// ============================================================================
// Template Specializations (defined in header, implemented here)
// ============================================================================

// Note: The template functions CreateUserdata, CheckUserdata, ToUserdata
// have placeholder implementations in the header. Full implementations
// would require knowing the specific types at compile time.

} // namespace BeEngine
