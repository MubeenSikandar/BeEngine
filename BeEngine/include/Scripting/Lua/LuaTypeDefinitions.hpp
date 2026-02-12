// LuaTypeDefinitions.hpp
#pragma once

#include <Core.hpp>
#include <cstdint>
#include <functional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

// Forward declare Lua types to avoid including lua.h in headers
struct lua_State;

namespace BeEngine {

// ============================================================================
// Lua State Handle
// ============================================================================

/// Wrapper for lua_State* that provides RAII semantics when needed
struct LuaStateHandle {
  lua_State *L{nullptr};

  LuaStateHandle() = default;
  explicit LuaStateHandle(lua_State *state) : L(state) {}

  operator lua_State *() const { return L; }
  NODISCARD lua_State *Get() const { return L; }
  NODISCARD bool IsValid() const { return L != nullptr; }
};
// ============================================================================
// Lua Reference
// ============================================================================

/// Reference to a Lua object stored in the registry
struct LuaRef {

  static constexpr int NoRef = -2;
  static constexpr int RefNil = -1;

  int ref{NoRef};
  lua_State *L{nullptr};

  LuaRef() = default;
  LuaRef(lua_State *state, int reference) : ref(reference), L(state) {}

  NODISCARD bool IsValid() const { return L != nullptr && ref != NoRef; }
  NODISCARD bool IsNil() const { return ref == RefNil; }

  void Release();
  void Push() const;

  bool operator==(const LuaRef &other) const {
    return L == other.L && ref == other.ref;
  }
  bool operator!=(const LuaRef &other) const { return !(*this == other); }
};

/// RAII wrapper for LuaRef that automatically releases on destruction
class LuaRefGuard {
public:
  LuaRefGuard() = default;
  explicit LuaRefGuard(LuaRef ref) : m_Ref(ref) {}
  ~LuaRefGuard();

  LuaRefGuard(const LuaRefGuard &) = delete;
  LuaRefGuard &operator=(const LuaRefGuard &) = delete;

  LuaRefGuard(LuaRefGuard &&other) noexcept;
  LuaRefGuard &operator=(LuaRefGuard &&other) noexcept;

  NODISCARD LuaRef Get() const { return m_Ref; }
  NODISCARD bool IsValid() const { return m_Ref.IsValid(); }

  LuaRef Release();

private:
  LuaRef m_Ref;
};

// ============================================================================
// Lua Type Enumeration
// ============================================================================

enum class LuaType : int8_t {
  None = -1,
  Nil = 0,
  Boolean = 1,
  LightUserdata = 2,
  Number = 3,
  String = 4,
  Table = 5,
  Function = 6,
  Userdata = 7,
  Thread = 8
};

constexpr const char *LuaTypeToString(LuaType type) {
  switch (type) {
  case LuaType::None:
    return "none";
  case LuaType::Nil:
    return "nil";
  case LuaType::Boolean:
    return "boolean";
  case LuaType::LightUserdata:
    return "lightuserdata";
  case LuaType::Number:
    return "number";
  case LuaType::String:
    return "string";
  case LuaType::Table:
    return "table";
  case LuaType::Function:
    return "function";
  case LuaType::Userdata:
    return "userdata";
  case LuaType::Thread:
    return "thread";
  default:
    return "unknown";
  }
}

// ============================================================================
// Lua Value Variant
// ============================================================================

/// Represents a Lua value that can be passed between C++ and Lua
using LuaValue = std::variant<std::monostate, // nil
                              bool,           // boolean
                              double, // number (Lua uses doubles internally)
                              std::string, // string
                              LuaRef,      // table/function/userdata reference
                              void *       // light userdata
                              >;

/// Convert LuaValue to string representation for debugging
std::string LuaValueToString(const LuaValue &value);

/// Get the LuaType of a LuaValue
NODISCARD LuaType GetLuaValueType(const LuaValue &value);

// ============================================================================
// Function Types
// ============================================================================

/// C function signature for Lua (returns number of results)
using LuaCFunction = int (*)(lua_State *);

/// Modern C++ callable wrapper for Lua functions
using LuaCallback = std::function<int(lua_State *)>;

/// Error handler function type
using LuaErrorHandler = std::function<void(const std::string &message,
                                           const std::string &traceback)>;

/// Print/log function type
using LuaPrintHandler = std::function<void(const std::string &message)>;

// ============================================================================
// Method Registration Structures
// ============================================================================

/// Describes a method that can be bound to Lua
struct LuaMethodDesc {
  const char *name;
  LuaCFunction func;
  const char *doc; // Optional documentation string
};

/// Describes a property (getter/setter pair)
struct LuaPropertyDesc {
  const char *name;
  LuaCFunction getter;
  LuaCFunction setter; // Can be nullptr for read-only
  const char *doc;
};

/// Describes a Lua class/type to be registered
struct LuaClassDesc {
  const char *name;
  const char *baseClass;    // Parent class name (or nullptr)
  LuaCFunction constructor; // __call metamethod
  LuaCFunction destructor;  // __gc metamethod
  std::vector<LuaMethodDesc> methods;
  std::vector<LuaPropertyDesc> properties;
  std::vector<LuaMethodDesc> staticMethods;
  const char *doc;
};

// ============================================================================
// Userdata Types
// ============================================================================

/// Header for all engine userdata objects
struct LuaUserdataHeader {
  uint32_t typeId;      // Unique type identifier
  uint32_t flags;       // Various flags (owned, weak ref, etc.)
  const char *typeName; // For error messages
};

/// Flags for userdata management
enum class LuaUserdataFlags : uint32_t {
  None = 0,
  Owned = 1 << 0,         // C++ side owns the object
  WeakRef = 1 << 1,       // Weak reference (check validity before use)
  Immutable = 1 << 2,     // Cannot be modified from Lua
  NoGC = 1 << 3,          // Don't call destructor on GC
  EngineManaged = 1 << 4, // Managed by engine (entities, components)
};

inline LuaUserdataFlags operator|(LuaUserdataFlags a, LuaUserdataFlags b) {
  return static_cast<LuaUserdataFlags>(static_cast<uint32_t>(a) |
                                       static_cast<uint32_t>(b));
}

inline LuaUserdataFlags operator&(LuaUserdataFlags a, LuaUserdataFlags b) {
  return static_cast<LuaUserdataFlags>(static_cast<uint32_t>(a) &
                                       static_cast<uint32_t>(b));
}

inline bool HasFlag(LuaUserdataFlags flags, LuaUserdataFlags flag) {
  return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
}

// ============================================================================
// Type Registration IDs
// ============================================================================

/// Unique IDs for engine types exposed to Lua
enum class LuaEngineTypeId : uint32_t {
  Unknown = 0,

  // Math types
  Vec2 = 100,
  Vec3,
  Vec4,
  Quat,
  Color,
  Mat4,

  // Engine types
  Entity = 200,
  Transform,
  Scene,
  Resource,

  // Component types
  ScriptComponent = 300,
  MeshRenderer,
  Camera,
  Light,
  Rigidbody,
  Collider,
  AudioSource,

  // Custom user types start here
  UserTypeStart = 10000
};

// ============================================================================
// Stack Operations Configuration
// ============================================================================

/// Configuration for automatic type conversion
struct LuaTypeConfig {
  bool strictNumberTypes{false};    // Disallow float<->int conversion
  bool allowImplicitToString{true}; // Allow __tostring metamethod
  bool checkUserdataTypes{true};    // Verify userdata metatables
  bool enableNilCoalescing{false};  // Treat nil as default value
};

// ============================================================================
// Metatable Names
// ============================================================================

namespace LuaMetatable {
constexpr const char *Vec2 = "BeEngine.Vec2";
constexpr const char *Vec3 = "BeEngine.Vec3";
constexpr const char *Vec4 = "BeEngine.Vec4";
constexpr const char *Quat = "BeEngine.Quat";
constexpr const char *Color = "BeEngine.Color";
constexpr const char *Entity = "BeEngine.Entity";
constexpr const char *Transform = "BeEngine.Transform";
constexpr const char *Scene = "BeEngine.Scene";
constexpr const char *Rigidbody = "BeEngine.Rigidbody";
constexpr const char *AudioSource = "BeEngine.AudioSource";
constexpr const char *Resource = "BeEngine.Resource";
} // namespace LuaMetatable

// ============================================================================
// Error Codes
// ============================================================================

enum class LuaErrorCode : int32_t {
  Ok = 0,
  RuntimeError = 2, // LUA_ERRRUN
  SyntaxError = 3,  // LUA_ERRSYNTAX
  MemoryError = 4,  // LUA_ERRMEM
  ErrorHandler = 5, // LUA_ERRERR (error in error handler)
  FileError = 6,    // LUA_ERRFILE
  GCError = 7,      // LUA_ERRGCMM (error in __gc)

  // Custom engine errors
  TypeMismatch = 100,
  InvalidReference,
  EntityNotFound,
  ComponentNotFound,
  SandboxViolation,
  TimeoutExceeded,
  MemoryLimitExceeded,
  CoroutineError
};

constexpr const char *LuaErrorCodeToString(LuaErrorCode code) {
  switch (code) {
  case LuaErrorCode::Ok:
    return "OK";
  case LuaErrorCode::RuntimeError:
    return "Runtime Error";
  case LuaErrorCode::SyntaxError:
    return "Syntax Error";
  case LuaErrorCode::MemoryError:
    return "Memory Error";
  case LuaErrorCode::ErrorHandler:
    return "Error Handler Error";
  case LuaErrorCode::FileError:
    return "File Error";
  case LuaErrorCode::GCError:
    return "GC Metamethod Error";
  case LuaErrorCode::TypeMismatch:
    return "Type Mismatch";
  case LuaErrorCode::InvalidReference:
    return "Invalid Reference";
  case LuaErrorCode::EntityNotFound:
    return "Entity Not Found";
  case LuaErrorCode::ComponentNotFound:
    return "Component Not Found";
  case LuaErrorCode::SandboxViolation:
    return "Sandbox Violation";
  case LuaErrorCode::TimeoutExceeded:
    return "Execution Timeout";
  case LuaErrorCode::MemoryLimitExceeded:
    return "Memory Limit Exceeded";
  case LuaErrorCode::CoroutineError:
    return "Coroutine Error";
  default:
    return "Unknown Error";
  }
}

// ============================================================================
// Script Lifecycle Events
// ============================================================================

/// Events that can trigger Lua callbacks
enum class LuaScriptEvent : uint8_t {
  OnCreate,
  OnDestroy,
  OnUpdate,
  OnFixedUpdate,
  OnLateUpdate,
  OnEnable,
  OnDisable,
  OnStart,

  // Physics events
  OnCollisionEnter,
  OnCollisionStay,
  OnCollisionExit,
  OnTriggerEnter,
  OnTriggerStay,
  OnTriggerExit,

  // Custom events
  OnCustomEvent,

  Count
};

constexpr const char *LuaScriptEventToString(LuaScriptEvent event) {
  switch (event) {
  case LuaScriptEvent::OnCreate:
    return "OnCreate";
  case LuaScriptEvent::OnDestroy:
    return "OnDestroy";
  case LuaScriptEvent::OnUpdate:
    return "OnUpdate";
  case LuaScriptEvent::OnFixedUpdate:
    return "OnFixedUpdate";
  case LuaScriptEvent::OnLateUpdate:
    return "OnLateUpdate";
  case LuaScriptEvent::OnEnable:
    return "OnEnable";
  case LuaScriptEvent::OnDisable:
    return "OnDisable";
  case LuaScriptEvent::OnStart:
    return "OnStart";
  case LuaScriptEvent::OnCollisionEnter:
    return "OnCollisionEnter";
  case LuaScriptEvent::OnCollisionStay:
    return "OnCollisionStay";
  case LuaScriptEvent::OnCollisionExit:
    return "OnCollisionExit";
  case LuaScriptEvent::OnTriggerEnter:
    return "OnTriggerEnter";
  case LuaScriptEvent::OnTriggerStay:
    return "OnTriggerStay";
  case LuaScriptEvent::OnTriggerExit:
    return "OnTriggerExit";
  case LuaScriptEvent::OnCustomEvent:
    return "OnCustomEvent";
  case LuaScriptEvent::Count:
    return "Count";
  default:
    return "Unknown";
  }
}

// ============================================================================
// Debug Information
// ============================================================================

/// Debug info for a Lua function
struct LuaDebugInfo {
  std::string name;    // Function name (if available)
  std::string source;  // Source file or chunk name
  int currentLine{-1}; // Current line being executed
  int lineDefined{-1}; // Line where function is defined
  int lastLineDefined{-1};
  std::string what; // "Lua", "C", "main", "tail"
  int numUpvalues{0};
  int numParams{0};
  bool isVararg{false};
  bool isTailCall{false};
};

/// Stack frame for debugging
struct LuaStackFrame {
  LuaDebugInfo info;
  std::vector<std::pair<std::string, LuaValue>> locals;
  std::vector<std::pair<std::string, LuaValue>> upvalues;
};

// ============================================================================
// Profiling Data
// ============================================================================

/// Performance data for a single Lua function
struct LuaFunctionProfile {
  std::string name;
  std::string source;
  int lineDefined{0};
  uint64_t callCount{0};
  double totalTime{0.0}; // Microseconds
  double selfTime{0.0};  // Excluding child calls
  double avgTime{0.0};
  double maxTime{0.0};
  size_t memoryAllocated{0};
};

/// Overall Lua state profile
struct LuaStateProfile {
  double totalTime{0.0};
  size_t totalMemory{0};
  size_t peakMemory{0};
  uint64_t gcCount{0};
  double gcTime{0.0};
  std::vector<LuaFunctionProfile> hotFunctions;
};

} // namespace BeEngine
