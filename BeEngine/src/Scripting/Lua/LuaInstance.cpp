#include <PCH/BeEnginePCH.hpp>

extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

namespace BeEngine {
// ============================================================================
// Static Members
// ============================================================================

const LuaInstance::MethodCache LuaInstance::s_MethodNames;

// ============================================================================
// Constructor / Destructor
// ============================================================================

LuaInstance::LuaInstance() : ScriptInstance() {}

LuaInstance::LuaInstance(LuaEngine *engine)
    : ScriptInstance(), m_Engine(engine) {}

LuaInstance::~LuaInstance() {
  if (m_Initialized) {
    // Call OnDestroy if it exists and hasn't been called
    if (m_CreateCalled && HasEventCallback(LuaScriptEvent::OnDestroy)) {
      CallLifecycleEvent(LuaScriptEvent::OnDestroy);
    }

    // Stop all coroutines
    StopAllCoroutines();

    // Destroy the Lua environment
    DestroyEnvironment();
  }
}

LuaInstance::LuaInstance(LuaInstance &&other) noexcept
    : ScriptInstance(std::move(other)), m_Engine(other.m_Engine),
      m_EnvironmentRef(other.m_EnvironmentRef), m_SelfRef(other.m_SelfRef),
      m_ClassRef(other.m_ClassRef), m_Initialized(other.m_Initialized),
      m_StartCalled(other.m_StartCalled), m_CreateCalled(other.m_CreateCalled),
      m_HasCallback(other.m_HasCallback),
      m_CallbacksCached(other.m_CallbacksCached),
      m_Coroutines(std::move(other.m_Coroutines)),
      m_MethodCallCount(other.m_MethodCallCount),
      m_TotalMethodTime(other.m_TotalMethodTime) {
  other.m_Engine = nullptr;
  other.m_EnvironmentRef = LuaRef{};
  other.m_SelfRef = LuaRef{};
  other.m_ClassRef = LuaRef{};
  other.m_Initialized = false;
}

LuaInstance &LuaInstance::operator=(LuaInstance &&other) noexcept {
  if (this != &other) {
    // Clean up current state
    if (m_Initialized) {
      StopAllCoroutines();
      DestroyEnvironment();
    }

    ScriptInstance::operator=(std::move(other));

    m_Engine = other.m_Engine;
    m_EnvironmentRef = other.m_EnvironmentRef;
    m_SelfRef = other.m_SelfRef;
    m_ClassRef = other.m_ClassRef;
    m_Initialized = other.m_Initialized;
    m_StartCalled = other.m_StartCalled;
    m_CreateCalled = other.m_CreateCalled;
    m_HasCallback = other.m_HasCallback;
    m_CallbacksCached = other.m_CallbacksCached;
    m_Coroutines = std::move(other.m_Coroutines);
    m_MethodCallCount = other.m_MethodCallCount;
    m_TotalMethodTime = other.m_TotalMethodTime;

    other.m_Engine = nullptr;
    other.m_EnvironmentRef = LuaRef{};
    other.m_SelfRef = LuaRef{};
    other.m_ClassRef = LuaRef{};
    other.m_Initialized = false;
  }
  return *this;
}

// ============================================================================
// Initialization
// ============================================================================

bool LuaInstance::Initialize(const std::string &scriptName) {
  if (m_Initialized) {
    BE_CORE_WARN("[LuaInstance] Already initialized");
    return true;
  }

  if (m_Engine == nullptr) {
    BE_CORE_ERROR("[LuaInstance] No engine assigned");
    return false;
  }

  lua_State *L = m_Engine->GetState();
  if (L == nullptr) {
    BE_CORE_ERROR("[LuaInstance] Engine has no Lua state");
    return false;
  }

  // Get the class system
  LuaClassSystem *classSystem = m_Engine->GetClassSystem();
  if (classSystem == nullptr) {
    BE_CORE_ERROR("[LuaInstance] No class system available");
    return false;
  }

  // Check if class is registered
  if (!classSystem->IsClassRegistered(scriptName)) {
    BE_CORE_ERROR("[LuaInstance] Script class '{}' not registered", scriptName);
    return false;
  }

  // Store script name
  SetScriptName(scriptName);

  // Create the instance through the class system
  uint64_t entityId = GetEntity().IsValid() ? GetEntity().GetUUID() : 0;
  m_SelfRef = classSystem->CreateInstance(scriptName, entityId);

  if (!m_SelfRef.IsValid()) {
    BE_CORE_ERROR("[LuaInstance] Failed to create instance of '{}'",
                  scriptName);
    return false;
  }

  // Get class reference
  m_ClassRef = classSystem->GetClass(scriptName);

  // Create environment
  if (!CreateEnvironment()) {
    BE_CORE_ERROR("[LuaInstance] Failed to create environment for '{}'",
                  scriptName);
    m_SelfRef.Release();
    return false;
  }

  // Cache which callbacks exist
  CacheCallbacks();

  // Sync initial field values
  SyncFieldsToLua();

  m_Initialized = true;
  BE_CORE_TRACE("[LuaInstance] Initialized instance of '{}'", scriptName);

  return true;
}

bool LuaInstance::Reinitialize() {
  if (m_Engine == nullptr) {
    return false;
  }

  std::string scriptName = GetScriptName();

  // Save current state
  ScriptFieldMap savedState;
  SaveState(savedState);

  // Destroy old environment
  DestroyEnvironment();
  m_Initialized = false;
  m_CallbacksCached = false;
  m_StartCalled = false;
  m_CreateCalled = false;

  // Re-initialize
  if (!Initialize(scriptName)) {
    return false;
  }

  // Restore state
  RestoreState(savedState);

  return true;
}

bool LuaInstance::CreateEnvironment() {
  lua_State *L = m_Engine->GetState();
  if (L == nullptr) {
    return false;
  }

  // Create environment table
  lua_newtable(L);

  // Set __index to fall back to _G for global access
  lua_newtable(L); // metatable
  lua_getglobal(L, "_G");
  lua_setfield(L, -2, "__index");
  lua_setmetatable(L, -2);

  // Store 'self' in environment
  m_Engine->PushRef(m_SelfRef);
  lua_setfield(L, -2, "self");

  // Store entity reference
  if (GetEntity().IsValid()) {
    LuaBindings::PushEntity(L, GetEntity());
    lua_setfield(L, -2, "entity");
  }

  // Create environment reference
  m_EnvironmentRef = m_Engine->CreateRef();

  return m_EnvironmentRef.IsValid();
}

void LuaInstance::DestroyEnvironment() {
  // Release Lua references
  if (m_EnvironmentRef.IsValid()) {
    m_EnvironmentRef.Release();
  }

  if (m_SelfRef.IsValid()) {
    m_SelfRef.Release();
  }

  // Don't release m_ClassRef - it's owned by the class system
  m_ClassRef = LuaRef{};
}

void LuaInstance::CacheCallbacks() {
  m_HasCallback.reset();

  if ((m_Engine == nullptr) || !m_SelfRef.IsValid()) {
    m_CallbacksCached = true;
    return;
  }

  lua_State *L = m_Engine->GetState();

  // Check each lifecycle event
  auto checkCallback = [this, L](LuaScriptEvent event, const char *name) {
    m_Engine->PushRef(m_SelfRef);
    lua_getfield(L, -1, name);
    bool hasMethod = lua_isfunction(L, -1);
    lua_pop(L, 2);

    if (hasMethod) {
      m_HasCallback.set(static_cast<size_t>(event));
    }
  };

  checkCallback(LuaScriptEvent::OnCreate, s_MethodNames.onCreate.c_str());
  checkCallback(LuaScriptEvent::OnDestroy, s_MethodNames.onDestroy.c_str());
  checkCallback(LuaScriptEvent::OnUpdate, s_MethodNames.onUpdate.c_str());
  checkCallback(LuaScriptEvent::OnFixedUpdate,
                s_MethodNames.onFixedUpdate.c_str());
  checkCallback(LuaScriptEvent::OnLateUpdate,
                s_MethodNames.onLateUpdate.c_str());
  checkCallback(LuaScriptEvent::OnEnable, s_MethodNames.onEnable.c_str());
  checkCallback(LuaScriptEvent::OnDisable, s_MethodNames.onDisable.c_str());
  checkCallback(LuaScriptEvent::OnStart, s_MethodNames.onStart.c_str());
  checkCallback(LuaScriptEvent::OnCollisionEnter,
                s_MethodNames.onCollisionEnter.c_str());
  checkCallback(LuaScriptEvent::OnCollisionStay,
                s_MethodNames.onCollisionStay.c_str());
  checkCallback(LuaScriptEvent::OnCollisionExit,
                s_MethodNames.onCollisionExit.c_str());
  checkCallback(LuaScriptEvent::OnTriggerEnter,
                s_MethodNames.onTriggerEnter.c_str());
  checkCallback(LuaScriptEvent::OnTriggerStay,
                s_MethodNames.onTriggerStay.c_str());
  checkCallback(LuaScriptEvent::OnTriggerExit,
                s_MethodNames.onTriggerExit.c_str());

  m_CallbacksCached = true;
}

// ============================================================================
// Lifecycle Events
// ============================================================================

void LuaInstance::OnCreate() {
  if (!m_Initialized || m_CreateCalled) {
    return;
  }

  m_CreateCalled = true;

  if (HasEventCallback(LuaScriptEvent::OnCreate)) {
    CallLifecycleEvent(LuaScriptEvent::OnCreate);
  }
}

void LuaInstance::OnDestroy() {
  if (!m_Initialized) {
    return;
  }

  // Stop all coroutines first
  StopAllCoroutines();

  if (HasEventCallback(LuaScriptEvent::OnDestroy)) {
    CallLifecycleEvent(LuaScriptEvent::OnDestroy);
  }
}

void LuaInstance::OnUpdate(float deltaTime) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  // Call OnStart once if not called
  if (!m_StartCalled) {
    m_StartCalled = true;
    if (HasEventCallback(LuaScriptEvent::OnStart)) {
      CallLifecycleEvent(LuaScriptEvent::OnStart);
    }
  }

  if (HasEventCallback(LuaScriptEvent::OnUpdate)) {
    lua_State *L = m_Engine->GetState();
    m_Engine->PushNumber(static_cast<double>(deltaTime));
    DispatchEvent(LuaScriptEvent::OnUpdate, 1);
  }
}

void LuaInstance::OnFixedUpdate(float fixedDelta) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnFixedUpdate)) {
    lua_State *L = m_Engine->GetState();
    m_Engine->PushNumber(static_cast<double>(fixedDelta));
    DispatchEvent(LuaScriptEvent::OnFixedUpdate, 1);
  }
}

void LuaInstance::OnLateUpdate(float deltaTime) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnLateUpdate)) {
    lua_State *L = m_Engine->GetState();
    m_Engine->PushNumber(static_cast<double>(deltaTime));
    DispatchEvent(LuaScriptEvent::OnLateUpdate, 1);
  }
}

void LuaInstance::OnEnable() {
  if (!m_Initialized) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnEnable)) {
    CallLifecycleEvent(LuaScriptEvent::OnEnable);
  }
}

void LuaInstance::OnDisable() {
  if (!m_Initialized) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnDisable)) {
    CallLifecycleEvent(LuaScriptEvent::OnDisable);
  }
}

void LuaInstance::OnStart() {
  if (!m_Initialized || m_StartCalled) {
    return;
  }

  m_StartCalled = true;

  if (HasEventCallback(LuaScriptEvent::OnStart)) {
    CallLifecycleEvent(LuaScriptEvent::OnStart);
  }
}

// ============================================================================
// Physics Callbacks
// ============================================================================

void LuaInstance::OnCollisionEnter(const CollisionInfo &collision) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnCollisionEnter)) {
    PushCollisionInfo(collision);
    DispatchEvent(LuaScriptEvent::OnCollisionEnter, 1);
  }
}

void LuaInstance::OnCollisionStay(Entity other) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnCollisionStay)) {
    lua_State *L = m_Engine->GetState();
    LuaBindings::PushEntity(L, other);
    DispatchEvent(LuaScriptEvent::OnCollisionStay, 1);
  }
}

void LuaInstance::OnCollisionExit(Entity other) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnCollisionExit)) {
    lua_State *L = m_Engine->GetState();
    LuaBindings::PushEntity(L, other);
    DispatchEvent(LuaScriptEvent::OnCollisionExit, 1);
  }
}

void LuaInstance::OnTriggerEnter(Entity other) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnTriggerEnter)) {
    lua_State *L = m_Engine->GetState();
    LuaBindings::PushEntity(L, other);
    DispatchEvent(LuaScriptEvent::OnTriggerEnter, 1);
  }
}

void LuaInstance::OnTriggerStay(Entity other) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnTriggerStay)) {
    lua_State *L = m_Engine->GetState();
    LuaBindings::PushEntity(L, other);
    DispatchEvent(LuaScriptEvent::OnTriggerStay, 1);
  }
}

void LuaInstance::OnTriggerExit(Entity other) {
  if (!m_Initialized || !IsEnabled()) {
    return;
  }

  if (HasEventCallback(LuaScriptEvent::OnTriggerExit)) {
    lua_State *L = m_Engine->GetState();
    LuaBindings::PushEntity(L, other);
    DispatchEvent(LuaScriptEvent::OnTriggerExit, 1);
  }
}

void LuaInstance::PushCollisionInfo(const CollisionInfo &collision) {
  lua_State *L = m_Engine->GetState();

  // Create collision info table
  lua_newtable(L);

  // other entity
  LuaBindings::PushEntity(L, collision.other);
  lua_setfield(L, -2, "other");

  // contact point
  LuaBindings::PushVec3(L, collision.contactPoint);
  lua_setfield(L, -2, "point");

  // normal
  LuaBindings::PushVec3(L, collision.normal);
  lua_setfield(L, -2, "normal");

  // impulse
  lua_pushnumber(L, collision.impulse);
  lua_setfield(L, -2, "impulse");
}

// ============================================================================
// Hot Reload
// ============================================================================

void LuaInstance::SaveState(ScriptFieldMap &outState) {
  if (!m_Initialized || (m_Engine == nullptr)) {
    return;
  }

  SyncFieldsFromLua();
  outState = GetFields();
}

void LuaInstance::RestoreState(const ScriptFieldMap &inState) {
  if (!m_Initialized || (m_Engine == nullptr)) {
    return;
  }

  // Update internal fields
  for (const auto &[name, field] : inState) {
    SetField(name, field.value);
  }

  // Sync to Lua
  SyncFieldsToLua();
}

void LuaInstance::OnBeforeReload() {
  // Stop coroutines before reload
  StopAllCoroutines();
}

void LuaInstance::OnAfterReload() {
  // Re-cache callbacks as methods may have changed
  m_CallbacksCached = false;
  CacheCallbacks();
}

// ============================================================================
// Lua Access
// ============================================================================

lua_State *LuaInstance::GetState() const {
  return (m_Engine != nullptr) ? m_Engine->GetState() : nullptr;
}

void LuaInstance::PushEnvironment() const {
  if ((m_Engine != nullptr) && m_EnvironmentRef.IsValid()) {
    m_Engine->PushRef(m_EnvironmentRef);
  }
}

void LuaInstance::PushSelf() const {
  if ((m_Engine != nullptr) && m_SelfRef.IsValid()) {
    m_Engine->PushRef(m_SelfRef);
  }
}

// ============================================================================
// Method Invocation
// ============================================================================

LuaErrorCode LuaInstance::CallMethod(const std::string &methodName) {
  return CallMethod(methodName, 0, 0);
}

LuaErrorCode LuaInstance::CallMethod(const std::string &methodName, int numArgs,
                                     int numResults) {
  if (!m_Initialized || (m_Engine == nullptr)) {
    return LuaErrorCode::InvalidReference;
  }

  lua_State *L = m_Engine->GetState();
  if (L == nullptr) {
    return LuaErrorCode::InvalidReference;
  }

  // Get method from self table
  m_Engine->PushRef(m_SelfRef);
  lua_getfield(L, -1, methodName.c_str());

  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2 + numArgs); // Pop nil, self, and any pushed args
    return LuaErrorCode::InvalidReference;
  }

  // Reorder stack: function, self, args...
  // Current: [args...] self function
  // Needed:  function self [args...]

  // Insert self before args
  lua_insert(L, -(numArgs + 2)); // Move function to bottom
  lua_insert(L, -(numArgs + 1)); // Move self after function

  // Call with self as first argument
  int errHandler = m_Engine->PushErrorHandler();

  // Move error handler below function
  lua_insert(L, -(numArgs + 3));

  auto startTime = std::chrono::high_resolution_clock::now();

  LuaErrorCode result =
      m_Engine->ProtectedCall(numArgs + 1, numResults, errHandler);

  auto endTime = std::chrono::high_resolution_clock::now();
  double elapsed =
      std::chrono::duration<double, std::micro>(endTime - startTime).count();

  // Update stats
  ++m_MethodCallCount;
  m_TotalMethodTime += elapsed;

  // Pop error handler
  lua_remove(L, -(numResults + 1));

  if (result != LuaErrorCode::Ok) {
    HandleMethodError(methodName, result);
  }

  return result;
}

bool LuaInstance::HasMethod(const std::string &methodName) const {
  if (!m_Initialized || (m_Engine == nullptr) || !m_SelfRef.IsValid()) {
    return false;
  }

  lua_State *L = m_Engine->GetState();

  m_Engine->PushRef(m_SelfRef);
  lua_getfield(L, -1, methodName.c_str());
  bool hasMethod = lua_isfunction(L, -1);
  lua_pop(L, 2);

  return hasMethod;
}

void LuaInstance::HandleMethodError(const std::string &methodName,
                                    LuaErrorCode error) {
  BE_CORE_ERROR("[LuaInstance] Error calling '{}' on '{}': {}", methodName,
                GetScriptName(), m_Engine->GetLastError());
}

// ============================================================================
// Field Access
// ============================================================================

void LuaInstance::SetInstanceField(const std::string &name,
                                   const LuaValue &value) {
  if (!m_Initialized || (m_Engine == nullptr) || !m_SelfRef.IsValid()) {
    return;
  }

  lua_State *L = m_Engine->GetState();

  m_Engine->PushRef(m_SelfRef);
  m_Engine->PushValue(value);
  lua_setfield(L, -2, name.c_str());
  lua_pop(L, 1);
}

LuaValue LuaInstance::GetInstanceField(const std::string &name) const {
  if (!m_Initialized || (m_Engine == nullptr) || !m_SelfRef.IsValid()) {
    return std::monostate{};
  }

  lua_State *L = m_Engine->GetState();

  m_Engine->PushRef(m_SelfRef);
  lua_getfield(L, -1, name.c_str());
  LuaValue value = m_Engine->GetValue(-1);
  lua_pop(L, 2);

  return value;
}

bool LuaInstance::HasInstanceField(const std::string &name) const {
  if (!m_Initialized || (m_Engine == nullptr) || !m_SelfRef.IsValid()) {
    return false;
  }

  lua_State *L = m_Engine->GetState();

  m_Engine->PushRef(m_SelfRef);
  lua_getfield(L, -1, name.c_str());
  bool exists = !lua_isnil(L, -1);
  lua_pop(L, 2);

  return exists;
}

void LuaInstance::SyncFieldsToLua() {
  if (!m_Initialized || (m_Engine == nullptr)) {
    return;
  }

  const auto &fields = GetFields();
  for (const auto &[name, field] : fields) {
    LuaValue luaValue;

    // Convert ScriptFieldValue to LuaValue
    std::visit(
        [&luaValue](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            luaValue = std::monostate{};
          } else if constexpr (std::is_same_v<T, bool>) {
            luaValue = arg;
          } else if constexpr (std::is_same_v<T, int32_t> ||
                               std::is_same_v<T, int64_t> ||
                               std::is_same_v<T, float> ||
                               std::is_same_v<T, double>) {
            luaValue = static_cast<double>(arg);
          } else if constexpr (std::is_same_v<T, std::string>) {
            luaValue = arg;
          }
          // Other types (Vec3, etc.) need special handling
        },
        field.value);

    SetInstanceField(name, luaValue);
  }
}

void LuaInstance::SyncFieldsFromLua() {
  if (!m_Initialized || (m_Engine == nullptr)) {
    return;
  }

  auto &fields = GetFields();
  for (auto &[name, field] : fields) {
    LuaValue luaValue = GetInstanceField(name);

    // Convert LuaValue to ScriptFieldValue
    std::visit(
        [&field](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, std::monostate>) {
            // Keep existing value for nil
          } else if constexpr (std::is_same_v<T, bool>) {
            field.value = arg;
          } else if constexpr (std::is_same_v<T, double>) {
            // Convert based on field type
            if (std::holds_alternative<int32_t>(field.value)) {
              field.value = static_cast<int32_t>(arg);
            } else if (std::holds_alternative<int64_t>(field.value)) {
              field.value = static_cast<int64_t>(arg);
            } else if (std::holds_alternative<float>(field.value)) {
              field.value = static_cast<float>(arg);
            } else {
              field.value = arg;
            }
          } else if constexpr (std::is_same_v<T, std::string>) {
            field.value = arg;
          }
        },
        luaValue);
  }
}

// ============================================================================
// Event Dispatch
// ============================================================================

bool LuaInstance::HasEventCallback(LuaScriptEvent event) const {
  if (!m_CallbacksCached) {
    // Shouldn't happen, but handle it
    return false;
  }
  return m_HasCallback.test(static_cast<size_t>(event));
}

LuaErrorCode LuaInstance::DispatchEvent(LuaScriptEvent event) {
  return DispatchEvent(event, 0);
}

LuaErrorCode LuaInstance::DispatchEvent(LuaScriptEvent event, int numArgs) {
  const char *methodName = LuaScriptEventToString(event);
  return CallMethod(methodName, numArgs, 0);
}

LuaErrorCode LuaInstance::CallLifecycleEvent(LuaScriptEvent event) {
  return DispatchEvent(event, 0);
}

LuaErrorCode LuaInstance::CallLifecycleEventTimed(LuaScriptEvent event,
                                                  double &outTime) {
  auto start = std::chrono::high_resolution_clock::now();
  LuaErrorCode result = CallLifecycleEvent(event);
  auto end = std::chrono::high_resolution_clock::now();

  outTime = std::chrono::duration<double, std::micro>(end - start).count();
  return result;
}

// ============================================================================
// Custom Events
// ============================================================================

LuaErrorCode LuaInstance::DispatchCustomEvent(const std::string &eventName) {
  return CallMethod(eventName, 0, 0);
}

LuaErrorCode LuaInstance::DispatchCustomEvent(const std::string &eventName,
                                              const LuaRef &eventData) {
  if (!m_Engine) {
    return LuaErrorCode::InvalidReference;
  }

  m_Engine->PushRef(eventData);
  return CallMethod(eventName, 1, 0);
}

LuaErrorCode LuaInstance::SendMessage(const std::string &methodName,
                                      const LuaValue &arg) {
  if (!HasMethod(methodName)) {
    return LuaErrorCode::Ok; // Not an error if method doesn't exist
  }

  if (!std::holds_alternative<std::monostate>(arg)) {
    m_Engine->PushValue(arg);
    return CallMethod(methodName, 1, 0);
  }

  return CallMethod(methodName, 0, 0);
}

void LuaInstance::BroadcastMessage(const std::string &methodName,
                                   const LuaValue &arg) {
  // This would need access to all scripts on the entity
  // For now, just call on this instance
  SendMessage(methodName, arg);
}

// ============================================================================
// Coroutines
// ============================================================================

LuaRef LuaInstance::StartCoroutine(const std::string &methodName) {
  if (!m_Initialized || !m_Engine) {
    return LuaRef{};
  }

  LuaCoroutineManager *coroMgr = m_Engine->GetCoroutineManager();
  if (coroMgr == nullptr) {
    BE_CORE_WARN("[LuaInstance] Coroutine manager not available");
    return LuaRef{};
  }

  // Get the method
  lua_State *L = m_Engine->GetState();
  m_Engine->PushRef(m_SelfRef);
  lua_getfield(L, -1, methodName.c_str());

  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    BE_CORE_WARN("[LuaInstance] Method '{}' not found for coroutine",
                 methodName);
    return LuaRef{};
  }

  LuaRef funcRef = m_Engine->CreateRef();
  lua_pop(L, 1); // Pop self

  CoroutineHandle handle = coroMgr->StartCoroutine(funcRef, this);

  // Store coroutine reference for cleanup
  // Note: We'd need to track the handle->ref mapping
  // For now, return the function ref

  return funcRef;
}

void LuaInstance::StopCoroutine(const LuaRef &coroutine) {
  if (m_Engine == nullptr) {
    return;
  }

  LuaCoroutineManager *coroMgr = m_Engine->GetCoroutineManager();
  if (coroMgr != nullptr) {
    // Would need handle lookup from ref
    // coroMgr->StopCoroutine(handle);
  }
}

void LuaInstance::StopAllCoroutines() {
  if (m_Engine == nullptr) {
    return;
  }

  LuaCoroutineManager *coroMgr = m_Engine->GetCoroutineManager();
  if (coroMgr != nullptr) {
    coroMgr->StopCoroutines(this);
  }

  m_Coroutines.clear();
}

bool LuaInstance::HasRunningCoroutines() const { return !m_Coroutines.empty(); }

// ============================================================================
// Debugging
// ============================================================================

std::string LuaInstance::GetDebugString() const {
  std::stringstream ss;
  ss << "LuaInstance[" << GetScriptName() << "]";
  ss << " Entity="
     << (GetEntity().IsValid() ? std::to_string(GetEntity().GetUUID())
                               : "invalid");
  ss << " Initialized=" << (m_Initialized ? "true" : "false");
  ss << " Enabled=" << (IsEnabled() ? "true" : "false");
  ss << " MethodCalls=" << m_MethodCallCount;
  ss << " TotalTime=" << m_TotalMethodTime << "us";
  return ss.str();
}

void LuaInstance::DumpState() const {
  BE_CORE_INFO("[LuaInstance] State dump for '{}':", GetScriptName());
  BE_CORE_INFO("  Entity: {}", GetEntity().IsValid()
                                   ? std::to_string(GetEntity().GetUUID())
                                   : "invalid");
  BE_CORE_INFO("  Initialized: {}", m_Initialized);
  BE_CORE_INFO("  Enabled: {}", IsEnabled());
  BE_CORE_INFO("  StartCalled: {}", m_StartCalled);
  BE_CORE_INFO("  CreateCalled: {}", m_CreateCalled);

  BE_CORE_INFO("  Callbacks:");
  for (size_t i = 0; i < static_cast<size_t>(LuaScriptEvent::Count); ++i) {
    if (m_HasCallback.test(i)) {
      BE_CORE_INFO("    - {}",
                   LuaScriptEventToString(static_cast<LuaScriptEvent>(i)));
    }
  }

  BE_CORE_INFO("  Fields:");
  const auto &fields = GetFields();
  for (const auto &[name, field] : fields) {
    BE_CORE_INFO("    {}: {}", name, field.GetString());
  }
}

std::vector<std::string> LuaInstance::GetMethodList() const {
  std::vector<std::string> methods;

  if (!m_Initialized || (m_Engine == nullptr) || !m_SelfRef.IsValid()) {
    return methods;
  }

  lua_State *L = m_Engine->GetState();

  m_Engine->PushRef(m_SelfRef);

  // Iterate over self table
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (lua_isfunction(L, -1) && lua_isstring(L, -2)) {
      methods.emplace_back(lua_tostring(L, -2));
    }
    lua_pop(L, 1); // Pop value, keep key
  }

  lua_pop(L, 1); // Pop self

  return methods;
}

std::vector<std::string> LuaInstance::GetFieldList() const {
  std::vector<std::string> fieldNames;

  if (!m_Initialized || m_Engine == nullptr || !m_SelfRef.IsValid()) {
    return fieldNames;
  }

  lua_State *L = m_Engine->GetState();

  m_Engine->PushRef(m_SelfRef);

  // Iterate over self table
  lua_pushnil(L);
  while (lua_next(L, -2) != 0) {
    if (!lua_isfunction(L, -1) && lua_isstring(L, -2)) {
      fieldNames.emplace_back(lua_tostring(L, -2));
    }
    lua_pop(L, 1); // Pop value, keep key
  }

  lua_pop(L, 1); // Pop self

  return fieldNames;
}

void LuaInstance::ResetCounters() {
  m_MethodCallCount = 0;
  m_TotalMethodTime = 0.0;
}

// ============================================================================
// LuaInstanceFactory
// ============================================================================

Ref<ScriptInstance> LuaInstanceFactory::Create(const std::string &scriptName,
                                               Entity entity) {
  auto instance = CreateRef<LuaInstance>(m_Engine);
  instance->SetEntity(entity);
  instance->SetScriptName(scriptName);
  instance->SetLanguage(ScriptLanguage::Lua);

  if (!instance->Initialize(scriptName)) {
    BE_CORE_ERROR("[LuaInstanceFactory] Failed to create instance of '{}'",
                  scriptName);
    return nullptr;
  }

  return instance;
}

} // namespace BeEngine
