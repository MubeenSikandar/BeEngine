// LucaClassSystem.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Core/ScriptField.hpp>
#include <Scripting/Lua/LuaTypeDefinitions.hpp>

// Forward declarations
struct lua_State;

namespace BeEngine {
// Forward declarations
class LuaEngine;

/**
 * @brief Provides an object-oriented class system for Lua scripts
 *
 * BeEngine uses a class-based scripting model similar to Unity/Unreal.
 * This class implements that model in Lua, providing:
 *
 * - Class definition and inheritance
 * - Constructor/destructor lifecycle
 * - Method definition and overriding
 * - Property accessors with validation
 * - Multiple inheritance (mixin style)
 * - Interface checking
 *
 * Example Lua usage:
 * ```lua
 * local PlayerController = Class("PlayerController", {
 *     -- Fields with defaults
 *     speed = 5.0,
 *     jumpForce = 10.0,
 *     isGrounded = false,
 *
 *     -- Lifecycle methods
 *     OnCreate = function(self)
 *         print("PlayerController created!")
 *     end,
 *
 *     OnUpdate = function(self, dt)
 *         local input = Input.GetAxis("Horizontal")
 *         self.entity:Translate(Vec3(input * self.speed * dt, 0, 0))
 *     end
 * })
 *
 * -- Inheritance
 * local SuperPlayer = Class("SuperPlayer", PlayerController, {
 *     speed = 10.0, -- Override default
 *
 *     OnUpdate = function(self, dt)
 *         -- Call parent
 *         SuperPlayer.super.OnUpdate(self, dt)
 *         -- Additional behavior
 *     end
 * })
 * ```
 */
class BE_API LuaClassSystem {
public:
  // ===== Lifecycle =====
  explicit LuaClassSystem(LuaEngine *engine);
  ~LuaClassSystem();

  // Non-copyable
  LuaClassSystem(const LuaClassSystem &) = delete;
  LuaClassSystem &operator=(const LuaClassSystem &) = delete;

  /// Initialize the class system in Lua
  void Initialize();

  /// Shutdown and cleanup
  void Shutdown();

  // ===== Class Registration =====

  /// Register a new script class
  /// @param name Class name
  /// @param classTable Reference to the class table in Lua
  /// @return true if successful
  NODISCARD bool RegisterClass(const std::string &name,
                               const LuaRef &classTable);

  /// Unregister a class
  void UnregisterClass(const std::string &name);

  /// Check if a class is registered
  NODISCARD bool IsClassRegistered(const std::string &name) const;

  /// Get a class table by name
  NODISCARD LuaRef GetClass(const std::string &name) const;

  /// Get all registered class names
  NODISCARD std::vector<std::string> GetRegisteredClasses() const;

  // ===== Instance Management =====

  /// Create an instance of a class
  /// @param className Name of the class
  /// @param entityId Entity to attach to
  /// @return Reference to the instance (self table)
  NODISCARD LuaRef CreateInstance(const std::string &className,
                                  uint64_t entityId);

  /// Destroy an instance
  void DestroyInstance(const LuaRef &instance);

  /// Get the class name of an instance
  NODISCARD std::string GetInstanceClassName(const LuaRef &instance) const;

  /// Check if instance is of a class (including inheritance)
  NODISCARD bool IsInstanceOf(const LuaRef &instance,
                              const std::string &className) const;

  // ===== Inheritance =====

  /// Set parent class
  void SetParentClass(const std::string &childClass,
                      const std::string &parentClass);

  /// Get parent class name
  NODISCARD std::string GetParentClass(const std::string &className) const;

  /// Check if class inherits from another (direct or indirect)
  NODISCARD bool InheritsFrom(const std::string &childClass,
                              const std::string &parentClass) const;

  /// Get inheritance chain (from child to root)
  NODISCARD std::vector<std::string>
  GetInheritanceChain(const std::string &className) const;

  // ===== Method Resolution =====

  /// Find a method in the class or its parents
  /// @param className Starting class
  /// @param methodName Method to find
  /// @return Reference to the method function, or invalid ref if not found
  NODISCARD LuaRef FindMethod(const std::string &className,
                              const std::string &methodName) const;

  /// Check if class has a method (including inherited)
  NODISCARD bool HasMethod(const std::string &className,
                           const std::string &methodName) const;

  /// Get all methods of a class (including inherited)
  NODISCARD std::vector<std::string>
  GetMethods(const std::string &className) const;

  // ===== Field Extraction =====

  /// Extract exposed fields from a class definition
  NODISCARD ScriptFieldMap ExtractFields(const std::string &className) const;

  /// Get default field values
  NODISCARD std::unordered_map<std::string, LuaValue>
  GetDefaultFields(const std::string &className) const;

  // ===== Mixins =====

  /// Add a mixin to a class (copies methods/fields)
  void AddMixin(const std::string &className, const LuaRef &mixin);

  /// Add a mixin by name
  void AddMixin(const std::string &className, const std::string &mixinName);

  // ===== Validation =====

  /// Validate that a class has required lifecycle methods
  struct ValidationResult {
    bool valid{true};
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
  };

  NODISCARD ValidationResult ValidateClass(const std::string &className) const;

  /// Check if class implements an interface (set of method names)
  NODISCARD bool
  ImplementsInterface(const std::string &className,
                      const std::vector<std::string> &methods) const;

  // ===== Metadata =====

  struct ClassMetadata {
    std::string name;
    std::string displayName;
    std::string description;
    std::string category;
    std::string parentClass;
    std::vector<std::string> tags;
    std::vector<std::string> methods;
    ScriptFieldMap fields;
    uint64_t registeredAt{0};
  };

  /// Get metadata for a class
  NODISCARD const ClassMetadata *
  GetClassMetadata(const std::string &name) const;

  /// Set class metadata
  void SetClassMetadata(const std::string &name, const ClassMetadata &metadata);

  // ===== Hot Reload Support =====

  /// Prepare for hot reload (save instance states)
  void PrepareForReload(const std::string &className);

  /// Complete hot reload (restore instance states)
  void CompleteReload(const std::string &className,
                      const LuaRef &newClassTable);

private:
  // ===== Internal Lua Functions =====

  /// The "Class" function called from Lua
  static int Lua_Class(lua_State *L);

  /// Instance __index metamethod
  static int Lua_Instance_Index(lua_State *L);

  /// Instance __newindex metamethod
  static int Lua_Instance_NewIndex(lua_State *L);

  /// Instance __tostring metamethod
  static int Lua_Instance_ToString(lua_State *L);

  /// Instance __eq metamethod
  static int Lua_Instance_Eq(lua_State *L);

  /// Class __call metamethod (for creating instances)
  static int Lua_Class_Call(lua_State *L);

  /// Super method accessor
  static int Lua_Super(lua_State *L);

  // ===== Internal Helpers =====

  void SetupMetatables();
  void CreateClassMetatable();
  void CreateInstanceMetatable();

  NODISCARD LuaRef CreateBaseObjectClass();
  void RegisterBuiltinClasses();

  static LuaClassSystem *GetFromRegistry(lua_State *L);
  void StoreInRegistry();

  LuaEngine *m_Engine{nullptr};
  lua_State *m_State{nullptr};

  // Registered classes
  struct RegisteredClass {
    std::string name;
    LuaRef classTable;
    LuaRef metatable;
    std::string parentClass;
    ClassMetadata metadata;
  };
  std::unordered_map<std::string, RegisteredClass> m_Classes;

  // Hot reload state storage
  struct ReloadState {
    std::vector<std::pair<uint64_t, ScriptFieldMap>> instanceStates;
  };
  std::unordered_map<std::string, ReloadState> m_ReloadStates;

  // Registry key for this instance
  static constexpr const char *REGISTRY_KEY = "BeEngine.LuaClassSystem";
  static constexpr const char *CLASS_METATABLE = "BeEngine.ClassMeta";
  static constexpr const char *INSTANCE_METATABLE = "BeEngine.InstanceMeta";
};

// ============================================================================
// Attribute Annotations (parsed from Lua comments)
// ============================================================================

/**
 * @brief Parses Lua script comments for attribute annotations
 *
 * Supports annotations like:
 * ```lua
 * ---@field speed number The movement speed
 * ---@range 0 100
 * ---@tooltip "Controls how fast the player moves"
 * speed = 5.0,
 * ```
 */
class BE_API LuaAttributeParser {
public:
  struct FieldAnnotation {
    std::string name;
    std::string type;
    std::string description;
    std::string tooltip;
    std::string category;
    bool hasRange{false};
    double minValue{0.0};
    double maxValue{0.0};
    bool isReadOnly{false};
    bool isHidden{false};
    std::vector<std::string> enumValues;
  };

  /// Parse annotations from Lua source code
  static std::vector<FieldAnnotation>
  ParseAnnotations(const std::string &source);

  /// Extract field annotations for a specific field
  static std::optional<FieldAnnotation>
  GetFieldAnnotation(const std::string &source, const std::string &fieldName);

  /// Convert annotation to ScriptField
  static ScriptField AnnotationToField(const FieldAnnotation &annotation);

private:
  static FieldAnnotation ParseFieldComment(const std::string &comment);
};

// ============================================================================
// Script Class Builder (Fluent API for C++ defined classes)
// ============================================================================

/**
 * @brief Fluent API for defining Lua-accessible classes from C++
 *
 * Usage:
 * ```cpp
 * LuaClassBuilder::Create(engine, "NativeComponent")
 *     .Inherit("Component")
 *     .Field("value", ScriptFieldType::Float, 0.0f)
 *     .Method("DoSomething", NativeComponent_DoSomething)
 *     .Property("health", NativeComponent_GetHealth, NativeComponent_SetHealth)
 *     .Register();
 * ```
 */
class BE_API LuaClassBuilder {
public:
  static LuaClassBuilder Create(LuaEngine *engine, const std::string &name);

  LuaClassBuilder &Inherit(const std::string &parentClass);
  LuaClassBuilder &DisplayName(const std::string &name);
  LuaClassBuilder &Description(const std::string &desc);
  LuaClassBuilder &Category(const std::string &category);
  LuaClassBuilder &Tag(const std::string &tag);

  // Field definitions
  LuaClassBuilder &Field(const std::string &name, ScriptFieldType type);
  LuaClassBuilder &Field(const std::string &name, ScriptFieldType type,
                         const ScriptFieldValue &defaultValue);
  LuaClassBuilder &Field(const ScriptField &field);

  // Method definitions
  LuaClassBuilder &Method(const std::string &name, LuaCFunction func);
  LuaClassBuilder &Method(const std::string &name, LuaCFunction func,
                          const std::string &doc);

  // Property definitions (getter/setter)
  LuaClassBuilder &Property(const std::string &name, LuaCFunction getter,
                            LuaCFunction setter = nullptr);

  // Static members
  LuaClassBuilder &StaticMethod(const std::string &name, LuaCFunction func);
  LuaClassBuilder &StaticField(const std::string &name, const LuaValue &value);

  // Constructor/destructor
  LuaClassBuilder &Constructor(LuaCFunction ctor);
  LuaClassBuilder &Destructor(LuaCFunction dtor);

  /// Finalize and register the class
  bool Register();

private:
  explicit LuaClassBuilder(LuaEngine *engine, const std::string &name);

  LuaEngine *m_Engine;
  std::string m_Name;
  std::string m_ParentClass;
  LuaClassSystem::ClassMetadata m_Metadata;
  std::vector<ScriptField> m_Fields;
  std::vector<LuaMethodDesc> m_Methods;
  std::vector<LuaMethodDesc> m_StaticMethods;
  std::vector<LuaPropertyDesc> m_Properties;
  std::vector<std::pair<std::string, LuaValue>> m_StaticFields;
  LuaCFunction m_Constructor{nullptr};
  LuaCFunction m_Destructor{nullptr};
};
} // namespace BeEngine
