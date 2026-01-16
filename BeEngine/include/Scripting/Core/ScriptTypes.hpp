// ScriptTypes.hpp
#pragma once

#include <Core.hpp>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <spdlog/fmt/bundled/format.h>
#include <string>
#include <variant>

namespace BeEngine {

/// Unique identifier for entities and assets
using UUID = uint64_t;
constexpr UUID InvalidUUID = 0;

// ============================================================================
// Forward Declarations
// ============================================================================

class ScriptEngine;
class ScriptInstance;
class ScriptComponent;
class Scene;

// ============================================================================
// Script Language Types
// ============================================================================

enum class ScriptLanguage : uint8_t {
  None = 0,
  Cpp,    // Native C++ scripts
  Lua,    // Lua/LuaJIT
  Python, // Python 3.x
  CSharp, // C# (Mono/.NET)
  Rust,   // Rust (dylib)
  WASM,   // WebAssembly

  Count
};

constexpr const char *ScriptLanguageToString(ScriptLanguage lang) {
  switch (lang) {
  case ScriptLanguage::None:
    return "None";
  case ScriptLanguage::Cpp:
    return "C++";
  case ScriptLanguage::Lua:
    return "Lua";
  case ScriptLanguage::Python:
    return "Python";
  case ScriptLanguage::CSharp:
    return "C#";
  case ScriptLanguage::Rust:
    return "Rust";
  case ScriptLanguage::WASM:
    return "WASM";
  default:
    return "Unknown";
  }
}

constexpr const char *ScriptLanguageExtension(ScriptLanguage lang) {
  switch (lang) {
  case ScriptLanguage::Cpp:
    return ".cpp";
  case ScriptLanguage::Lua:
    return ".lua";
  case ScriptLanguage::Python:
    return ".py";
  case ScriptLanguage::CSharp:
    return ".cs";
  case ScriptLanguage::Rust:
    return ".rs";
  case ScriptLanguage::WASM:
    return ".wasm";
  default:
    return "";
  }
}

// ============================================================================
// Script Field Types (for serialization & inspector)
// ============================================================================

struct Color {
  float r{0.0F}, g{0.0F}, b{0.0F}, a{1.0F};

  constexpr Color() = default;
  constexpr Color(float r, float g, float b, float a = 1.0f)
      : r(r), g(g), b(b), a(a) {}
  explicit Color(const glm::vec4 &v) : r(v.r), g(v.g), b(v.b), a(v.a) {}

  operator glm::vec4() const { return {r, g, b, a}; }

  // Useful presets
  static constexpr Color White() { return {1, 1, 1, 1}; }
  static constexpr Color Black() { return {0, 0, 0, 1}; }
  static constexpr Color Red() { return {1, 0, 0, 1}; }
  static constexpr Color Green() { return {0, 1, 0, 1}; }
  static constexpr Color Blue() { return {0, 0, 1, 1}; }
  static constexpr Color Clear() { return {0, 0, 0, 0}; }

  bool operator==(const Color &other) const {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }
};

enum class ScriptFieldType : uint8_t {
  None = 0,

  // Primitives
  Bool,
  Int8,
  Int16,
  Int32,
  Int64,
  UInt8,
  UInt16,
  UInt32,
  UInt64,
  Float,
  Double,
  String,

  // Math types
  Vec2,
  Vec3,
  Vec4,
  Color,
  Quat,

  // Engine types
  Entity,
  Asset, // Resource handle
  Prefab,

  // Collections
  Array,

  Count
};

constexpr const char *ScriptFieldTypeToString(ScriptFieldType type) {
  switch (type) {
  case ScriptFieldType::None:
    return "None";
  case ScriptFieldType::Bool:
    return "Bool";
  case ScriptFieldType::Int32:
    return "Int32";
  case ScriptFieldType::Int64:
    return "Int64";
  case ScriptFieldType::Float:
    return "Float";
  case ScriptFieldType::Double:
    return "Double";
  case ScriptFieldType::String:
    return "String";
  case ScriptFieldType::Vec2:
    return "Vec2";
  case ScriptFieldType::Vec3:
    return "Vec3";
  case ScriptFieldType::Vec4:
    return "Vec4";
  case ScriptFieldType::Color:
    return "Color";
  case ScriptFieldType::Quat:
    return "Quaternion";
  case ScriptFieldType::Entity:
    return "Entity";
  case ScriptFieldType::Asset:
    return "Asset";
  case ScriptFieldType::Int8:
    return "Int8";
  case ScriptFieldType::Int16:
    return "Int16";
  case ScriptFieldType::UInt8:
    return "UInt8";
  case ScriptFieldType::UInt16:
    return "UInt16";
  case ScriptFieldType::UInt32:
    return "UInt32";
  case ScriptFieldType::UInt64:
    return "UInt64";
  case ScriptFieldType::Prefab:
    return "Prefab";
  case ScriptFieldType::Array:
    return "Array";
  case ScriptFieldType::Count:
    return "Count"; // or handle differently
  default:
    return "Unknown";
  }
}

// ============================================================================
// Script Field Value
// ============================================================================

struct EntityRef {
  UUID id{InvalidUUID};

  bool operator==(const EntityRef &other) const { return id == other.id; }
  bool operator!=(const EntityRef &other) const { return id != other.id; }
  explicit operator bool() const { return id != InvalidUUID; }
};

struct AssetRef {
  UUID id{InvalidUUID};

  bool operator==(const AssetRef &other) const { return id == other.id; }
  bool operator!=(const AssetRef &other) const { return id != other.id; }
  explicit operator bool() const { return id != InvalidUUID; }
};

struct PrefabRef {
  UUID id{InvalidUUID};

  bool operator==(const PrefabRef &other) const { return id == other.id; }
  bool operator!=(const PrefabRef &other) const { return id != other.id; }
  explicit operator bool() const { return id != InvalidUUID; }
};

using ScriptFieldValue =
    std::variant<std::monostate, // None
                 bool,           // Bool
                 int8_t, int16_t, int32_t, int64_t, uint8_t, uint16_t, uint32_t,
                 uint64_t, float, double, std::string, glm::vec2, glm::vec3,
                 glm::vec4, glm::quat, EntityRef, AssetRef, PrefabRef, Color

                 >;

// ============================================================================
// Script State
// ============================================================================

enum class ScriptState : uint8_t {
  Unloaded = 0,
  Loading,
  Loaded,
  Running,
  Paused,
  Error,
  Reloading
};

constexpr const char *ScriptStateToString(ScriptState state) {
  switch (state) {
  case ScriptState::Unloaded:
    return "Unloaded";
  case ScriptState::Loading:
    return "Loading";
  case ScriptState::Loaded:
    return "Loaded";
  case ScriptState::Running:
    return "Running";
  case ScriptState::Paused:
    return "Paused";
  case ScriptState::Error:
    return "Error";
  case ScriptState::Reloading:
    return "Reloading";
  default:
    return "Unknown";
  }
}

// ============================================================================
// Script Error
// ============================================================================

struct ScriptError {
  std::string message;
  std::string file;
  int line{-1};
  int column{-1};
  ScriptLanguage language{ScriptLanguage::None};

  NODISCARD std::string ToString() const {
    if (file.empty()) {
      return message;
    }
    if (line < 0) {
      return fmt::format("{}: {}", file, message);
    }
    if (column < 0) {
      return fmt::format("{}:{}: {}", file, line, message);
    }
    return fmt::format("{}:{}:{}: {}", file, line, column, message);
  }
};

// ============================================================================
// Script Metadata
// ============================================================================

struct ScriptMetadata {
  std::string name;      // Display name
  std::string className; // Class/module name
  std::string filePath;  // Source file path
  ScriptLanguage language{ScriptLanguage::None};
  std::string description;
  std::string author;
  std::string version;
  std::vector<std::string> tags;

  // Dependencies
  std::vector<std::string> dependencies;

  // Timestamps
  uint64_t lastModified{0};
  uint64_t loadedAt{0};
};

// ============================================================================
// Script Profiling
// ============================================================================

struct ScriptProfileData {
  std::string scriptName;

  // Timing (microseconds)
  double createTime{0.0};
  double updateTime{0.0};
  double fixedUpdateTime{0.0};
  double lateUpdateTime{0.0};
  double totalTime{0.0};

  // Call counts
  uint64_t updateCalls{0};
  uint64_t fixedUpdateCalls{0};
  uint64_t lateUpdateCalls{0};

  // Memory (bytes)
  size_t memoryUsage{0};

  // Errors
  uint32_t errorCount{0};

  void Reset() {
    createTime = updateTime = fixedUpdateTime = lateUpdateTime = totalTime =
        0.0;
    updateCalls = fixedUpdateCalls = lateUpdateCalls = 0;
    memoryUsage = 0;
    errorCount = 0;
  }

  NODISCARD double GetAverageUpdateTime() const {
    return updateCalls > 0 ? updateTime / static_cast<double>(updateCalls)
                           : 0.0;
  }
};

// ============================================================================
// Callbacks
// ============================================================================

using ScriptErrorCallback = std::function<void(const ScriptError &)>;
using ScriptReloadCallback = std::function<void(const std::string &scriptPath)>;
using ScriptCompileCallback =
    std::function<void(const std::string &scriptPath, bool success)>;

template <typename T> constexpr ScriptFieldType GetScriptFieldType() {
  if constexpr (std::is_same_v<T, bool>) {
    return ScriptFieldType::Bool;
  } else if constexpr (std::is_same_v<T, int8_t>) {
    return ScriptFieldType::Int8;
  } else if constexpr (std::is_same_v<T, int16_t>) {
    return ScriptFieldType::Int16;
  } else if constexpr (std::is_same_v<T, int32_t>) {
    return ScriptFieldType::Int32;
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return ScriptFieldType::Int64;
  } else if constexpr (std::is_same_v<T, uint8_t>) {
    return ScriptFieldType::UInt8;
  } else if constexpr (std::is_same_v<T, uint16_t>) {
    return ScriptFieldType::UInt16;
  } else if constexpr (std::is_same_v<T, uint32_t>) {
    return ScriptFieldType::UInt32;
  } else if constexpr (std::is_same_v<T, uint64_t>) {
    return ScriptFieldType::UInt64;
  } else if constexpr (std::is_same_v<T, float>) {
    return ScriptFieldType::Float;
  } else if constexpr (std::is_same_v<T, double>) {
    return ScriptFieldType::Double;
  } else if constexpr (std::is_same_v<T, std::string>) {
    return ScriptFieldType::String;
  } else if constexpr (std::is_same_v<T, glm::vec2>) {
    return ScriptFieldType::Vec2;
  } else if constexpr (std::is_same_v<T, glm::vec3>) {
    return ScriptFieldType::Vec3;
  } else if constexpr (std::is_same_v<T, glm::vec4>) {
    return ScriptFieldType::Vec4;
  } else if constexpr (std::is_same_v<T, Color>) {
    return ScriptFieldType::Color;
  } else if constexpr (std::is_same_v<T, glm::quat>) {
    return ScriptFieldType::Quat;
  } else if constexpr (std::is_same_v<T, EntityRef>) {
    return ScriptFieldType::Entity;
  } else if constexpr (std::is_same_v<T, AssetRef>) {
    return ScriptFieldType::Asset;
  } else if constexpr (std::is_same_v<T, PrefabRef>) {
    return ScriptFieldType::Prefab;
  } else {
    return ScriptFieldType::None;
  }
}

} // namespace BeEngine
