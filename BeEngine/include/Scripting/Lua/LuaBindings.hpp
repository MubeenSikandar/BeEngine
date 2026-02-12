// LuaBindings.hpp
#pragma once

#include <Core.hpp>
#include <Scripting/Core/ScriptTypes.hpp>
#include <Scripting/Lua/LuaTypeDefinitions.hpp>

// Forward declarations
struct lua_State;

namespace BeEngine {

// Forward declarations
class LuaEngine;
class Entity;
class Scene;
/**
 * @brief Provides all engine API bindings to Lua
 *
 * This class registers the complete BeEngine API with Lua, exposing:
 * - Entity and Transform operations
 * - Input handling
 * - Time functions
 * - Physics
 * - Audio
 * - Scene management
 * - Debug/logging
 * - Math types (Vec2, Vec3, Vec4, Quat, Color)
 *
 * Binding Strategy:
 * - Math types are value types (copied to/from Lua)
 * - Entity is a lightweight handle (uint64_t)
 * - Components are accessed through entity methods
 * - Resources are reference counted handles
 *
 * Usage:
 *   LuaBindings::RegisterAll(engine);
 */

class BE_API LuaBindings {
public:
  // ===== Registration =====
  /// Register all engine bindings
  static void RegisterAll(LuaEngine *engine);

  /// Register only math types
  static void RegisterMathTypes(lua_State *L);

  /// Register entity/component API
  static void RegisterEntityAPI(lua_State *L);

  /// Register input API
  static void RegisterInputAPI(lua_State *L);

  /// Register time API
  static void RegisterTimeAPI(lua_State *L);

  /// Register physics API
  static void RegisterPhysicsAPI(lua_State *L);

  /// Register audio API
  static void RegisterAudioAPI(lua_State *L);

  /// Register scene API
  static void RegisterSceneAPI(lua_State *L);

  /// Register debug/logging API
  static void RegisterDebugAPI(lua_State *L);

  /// Register resource API
  static void RegisterResourceAPI(lua_State *L);

  static int Entity_GetIndex(lua_State *L);
  static int Entity_GetGeneration(lua_State *L);
  static int Entity_GetUUID(lua_State *L);

  // ===== Stack Helpers =====

  // Vec2
  static void PushVec2(lua_State *L, const glm::vec2 &v);
  static void PushVec2(lua_State *L, float x, float y);
  static glm::vec2 CheckVec2(lua_State *L, int index);
  static glm::vec2 ToVec2(lua_State *L, int index);
  static bool IsVec2(lua_State *L, int index);

  // Vec3
  static void PushVec3(lua_State *L, const glm::vec3 &v);
  static void PushVec3(lua_State *L, float x, float y, float z);
  static glm::vec3 CheckVec3(lua_State *L, int index);
  static glm::vec3 ToVec3(lua_State *L, int index);
  static bool IsVec3(lua_State *L, int index);

  // Vec4
  static void PushVec4(lua_State *L, const glm::vec4 &v);
  static void PushVec4(lua_State *L, float x, float y, float z, float w);
  static glm::vec4 CheckVec4(lua_State *L, int index);
  static glm::vec4 ToVec4(lua_State *L, int index);
  static bool IsVec4(lua_State *L, int index);

  // Quat
  static void PushQuat(lua_State *L, const glm::quat &q);
  static glm::quat CheckQuat(lua_State *L, int index);
  static glm::quat ToQuat(lua_State *L, int index);
  static bool IsQuat(lua_State *L, int index);

  // Color
  static void PushColor(lua_State *L, const Color &c);
  static void PushColor(lua_State *L, float r, float g, float b,
                        float a = 1.0F);
  static Color CheckColor(lua_State *L, int index);
  static Color ToColor(lua_State *L, int index);
  static bool IsColor(lua_State *L, int index);

  // Entity
  static void PushEntity(lua_State *L, uint64_t entityId);
  static void PushEntity(lua_State *L, Entity entity);
  static uint64_t CheckEntity(lua_State *L, int index);
  static uint64_t ToEntity(lua_State *L, int index);
  static bool IsEntity(lua_State *L, int index);

  // Generic converters
  static void PushScriptFieldValue(lua_State *L, const ScriptFieldValue &value);
  static ScriptFieldValue ToScriptFieldValue(lua_State *L, int index);

  // ===== Utility =====

  /// Set active scene for bindings
  static void SetActiveScene(Scene *scene);

  /// Get active scene
  static Scene *GetActiveScene();

  /// Create standard metatable with __tostring, etc.
  static void CreateMetatable(lua_State *L, const char *name,
                              const std::vector<LuaMethodDesc> &methods);

  /// Register metamethods
  static void
  RegisterMetamethods(lua_State *L, const char *name,
                      const std::vector<LuaMethodDesc> &metamethods);

private:
  // ===== Math Type Bindings =====

  // Vec2 methods
  static int Vec2_New(lua_State *L);
  static int Vec2_Add(lua_State *L);
  static int Vec2_Sub(lua_State *L);
  static int Vec2_Mul(lua_State *L);
  static int Vec2_Div(lua_State *L);
  static int Vec2_Unm(lua_State *L);
  static int Vec2_Eq(lua_State *L);
  static int Vec2_ToString(lua_State *L);
  static int Vec2_Index(lua_State *L);
  static int Vec2_NewIndex(lua_State *L);
  static int Vec2_Length(lua_State *L);
  static int Vec2_LengthSquared(lua_State *L);
  static int Vec2_Normalize(lua_State *L);
  static int Vec2_Normalized(lua_State *L);
  static int Vec2_Dot(lua_State *L);
  static int Vec2_Distance(lua_State *L);
  static int Vec2_Lerp(lua_State *L);
  static int Vec2_Angle(lua_State *L);

  // Vec3 methods
  static int Vec3_New(lua_State *L);
  static int Vec3_Add(lua_State *L);
  static int Vec3_Sub(lua_State *L);
  static int Vec3_Mul(lua_State *L);
  static int Vec3_Div(lua_State *L);
  static int Vec3_Unm(lua_State *L);
  static int Vec3_Eq(lua_State *L);
  static int Vec3_ToString(lua_State *L);
  static int Vec3_Index(lua_State *L);
  static int Vec3_NewIndex(lua_State *L);
  static int Vec3_Length(lua_State *L);
  static int Vec3_LengthSquared(lua_State *L);
  static int Vec3_Normalize(lua_State *L);
  static int Vec3_Normalized(lua_State *L);
  static int Vec3_Dot(lua_State *L);
  static int Vec3_Cross(lua_State *L);
  static int Vec3_Distance(lua_State *L);
  static int Vec3_Lerp(lua_State *L);
  static int Vec3_Slerp(lua_State *L);
  static int Vec3_Project(lua_State *L);
  static int Vec3_Reflect(lua_State *L);

  // Vec4 methods
  static int Vec4_New(lua_State *L);
  static int Vec4_Add(lua_State *L);
  static int Vec4_Sub(lua_State *L);
  static int Vec4_Mul(lua_State *L);
  static int Vec4_Div(lua_State *L);
  static int Vec4_Unm(lua_State *L);
  static int Vec4_Eq(lua_State *L);
  static int Vec4_ToString(lua_State *L);
  static int Vec4_Index(lua_State *L);
  static int Vec4_NewIndex(lua_State *L);
  static int Vec4_Length(lua_State *L);
  static int Vec4_Normalize(lua_State *L);
  static int Vec4_Dot(lua_State *L);
  static int Vec4_Lerp(lua_State *L);

  // Quat methods
  static int Quat_New(lua_State *L);
  static int Quat_Identity(lua_State *L);
  static int Quat_FromEuler(lua_State *L);
  static int Quat_FromAxisAngle(lua_State *L);
  static int Quat_LookRotation(lua_State *L);
  static int Quat_Mul(lua_State *L);
  static int Quat_Eq(lua_State *L);
  static int Quat_ToString(lua_State *L);
  static int Quat_Index(lua_State *L);
  static int Quat_NewIndex(lua_State *L);
  static int Quat_ToEuler(lua_State *L);
  static int Quat_Inverse(lua_State *L);
  static int Quat_Conjugate(lua_State *L);
  static int Quat_Normalize(lua_State *L);
  static int Quat_Dot(lua_State *L);
  static int Quat_Slerp(lua_State *L);
  static int Quat_Lerp(lua_State *L);
  static int Quat_Angle(lua_State *L);
  static int Quat_RotateVector(lua_State *L);

  // Color methods
  static int Color_New(lua_State *L);
  static int Color_FromHex(lua_State *L);
  static int Color_FromHSV(lua_State *L);
  static int Color_Add(lua_State *L);
  static int Color_Sub(lua_State *L);
  static int Color_Mul(lua_State *L);
  static int Color_Eq(lua_State *L);
  static int Color_ToString(lua_State *L);
  static int Color_Index(lua_State *L);
  static int Color_NewIndex(lua_State *L);
  static int Color_ToHex(lua_State *L);
  static int Color_ToHSV(lua_State *L);
  static int Color_Lerp(lua_State *L);
  // Static color presets
  static int Color_White(lua_State *L);
  static int Color_Black(lua_State *L);
  static int Color_Red(lua_State *L);
  static int Color_Green(lua_State *L);
  static int Color_Blue(lua_State *L);
  static int Color_Yellow(lua_State *L);
  static int Color_Cyan(lua_State *L);
  static int Color_Magenta(lua_State *L);
  static int Color_Clear(lua_State *L);

  // ===== Entity Bindings =====

  static int Entity_Create(lua_State *L);
  static int Entity_CreateWithUUID(lua_State *L);
  static int Entity_Destroy(lua_State *L);
  static int Entity_Duplicate(lua_State *L);
  static int Entity_IsValid(lua_State *L);
  static int Entity_GetName(lua_State *L);
  static int Entity_SetName(lua_State *L);
  static int Entity_IsEnabled(lua_State *L);
  static int Entity_SetEnabled(lua_State *L);
  static int Entity_GetParent(lua_State *L);
  static int Entity_SetParent(lua_State *L);
  static int Entity_RemoveParent(lua_State *L);
  static int Entity_GetChildCount(lua_State *L);
  static int Entity_GetChild(lua_State *L);
  static int Entity_GetChildren(lua_State *L);
  static int Entity_FindByName(lua_State *L);
  static int Entity_FindByTag(lua_State *L);
  static int Entity_GetComponent(lua_State *L);
  static int Entity_HasComponent(lua_State *L);
  static int Entity_AddComponent(lua_State *L);
  static int Entity_RemoveComponent(lua_State *L);
  static int Entity_Eq(lua_State *L);
  static int Entity_ToString(lua_State *L);

  // ===== Transform Bindings =====

  static int Transform_GetPosition(lua_State *L);
  static int Transform_SetPosition(lua_State *L);
  static int Transform_GetRotation(lua_State *L);
  static int Transform_SetRotation(lua_State *L);
  static int Transform_GetEulerAngles(lua_State *L);
  static int Transform_SetEulerAngles(lua_State *L);
  static int Transform_GetScale(lua_State *L);
  static int Transform_SetScale(lua_State *L);
  static int Transform_GetWorldPosition(lua_State *L);
  static int Transform_GetForward(lua_State *L);
  static int Transform_GetRight(lua_State *L);
  static int Transform_GetUp(lua_State *L);
  static int Transform_Translate(lua_State *L);
  static int Transform_Rotate(lua_State *L);
  static int Transform_LookAt(lua_State *L);
  static int Transform_TransformPoint(lua_State *L);
  static int Transform_TransformDirection(lua_State *L);
  static int Transform_InverseTransformPoint(lua_State *L);
  static int Transform_InverseTransformDirection(lua_State *L);

  // ===== Input Bindings =====

  // Keyboard
  static int Input_GetKey(lua_State *L);
  static int Input_GetKeyDown(lua_State *L);
  static int Input_GetKeyUp(lua_State *L);
  static int Input_GetKeyName(lua_State *L);

  // Mouse
  static int Input_GetMouseButton(lua_State *L);
  static int Input_GetMouseButtonDown(lua_State *L);
  static int Input_GetMouseButtonUp(lua_State *L);
  static int Input_GetMousePosition(lua_State *L);
  static int Input_GetMouseDelta(lua_State *L);
  static int Input_GetMouseScroll(lua_State *L);
  static int Input_SetCursorVisible(lua_State *L);
  static int Input_SetCursorLocked(lua_State *L);
  static int Input_IsCursorVisible(lua_State *L);
  static int Input_IsCursorLocked(lua_State *L);

  // Gamepad
  static int Input_IsGamepadConnected(lua_State *L);
  static int Input_GetGamepadButton(lua_State *L);
  static int Input_GetGamepadButtonDown(lua_State *L);
  static int Input_GetGamepadAxis(lua_State *L);
  static int Input_GetGamepadStick(lua_State *L);
  static int Input_GetGamepadName(lua_State *L);
  static int Input_SetGamepadVibration(lua_State *L);

  // Actions
  static int Input_IsActionPressed(lua_State *L);
  static int Input_IsActionDown(lua_State *L);
  static int Input_IsActionReleased(lua_State *L);
  static int Input_GetAxis(lua_State *L);
  static int Input_GetAxisRaw(lua_State *L);

  // ===== Time Bindings =====

  static int Time_GetDelta(lua_State *L);
  static int Time_GetUnscaledDelta(lua_State *L);
  static int Time_GetFixedDelta(lua_State *L);
  static int Time_GetElapsed(lua_State *L);
  static int Time_GetUnscaledElapsed(lua_State *L);
  static int Time_GetScale(lua_State *L);
  static int Time_SetScale(lua_State *L);
  static int Time_GetFrameCount(lua_State *L);
  static int Time_GetFPS(lua_State *L);
  static int Time_IsPaused(lua_State *L);
  static int Time_SetPaused(lua_State *L);

  // ===== Physics Bindings =====

  static int Physics_Raycast(lua_State *L);
  static int Physics_RaycastAll(lua_State *L);
  static int Physics_SphereCast(lua_State *L);
  static int Physics_BoxCast(lua_State *L);
  static int Physics_OverlapSphere(lua_State *L);
  static int Physics_OverlapBox(lua_State *L);
  static int Physics_GetGravity(lua_State *L);
  static int Physics_SetGravity(lua_State *L);

  // Rigidbody
  static int Rigidbody_Exists(lua_State *L);
  static int Rigidbody_AddForce(lua_State *L);
  static int Rigidbody_AddImpulse(lua_State *L);
  static int Rigidbody_AddTorque(lua_State *L);
  static int Rigidbody_GetVelocity(lua_State *L);
  static int Rigidbody_SetVelocity(lua_State *L);
  static int Rigidbody_GetAngularVelocity(lua_State *L);
  static int Rigidbody_SetAngularVelocity(lua_State *L);
  static int Rigidbody_GetMass(lua_State *L);
  static int Rigidbody_SetMass(lua_State *L);
  static int Rigidbody_IsKinematic(lua_State *L);
  static int Rigidbody_SetKinematic(lua_State *L);
  static int Rigidbody_UseGravity(lua_State *L);
  static int Rigidbody_SetUseGravity(lua_State *L);

  // ===== Audio Bindings =====

  static int Audio_Play(lua_State *L);
  static int Audio_PlayAtPosition(lua_State *L);
  static int Audio_PlayOneShot(lua_State *L);
  static int Audio_Stop(lua_State *L);
  static int Audio_StopAll(lua_State *L);
  static int Audio_Pause(lua_State *L);
  static int Audio_Resume(lua_State *L);
  static int Audio_SetVolume(lua_State *L);
  static int Audio_GetVolume(lua_State *L);
  static int Audio_SetPitch(lua_State *L);
  static int Audio_SetLoop(lua_State *L);
  static int Audio_IsPlaying(lua_State *L);
  static int Audio_SetMasterVolume(lua_State *L);
  static int Audio_GetMasterVolume(lua_State *L);
  static int Audio_SetListenerPosition(lua_State *L);

  // ===== Scene Bindings =====

  static int Scene_GetActive(lua_State *L);
  static int Scene_GetName(lua_State *L);
  static int Scene_Load(lua_State *L);
  static int Scene_LoadAdditive(lua_State *L);
  static int Scene_Unload(lua_State *L);
  static int Scene_GetEntityCount(lua_State *L);
  static int Scene_GetAllEntities(lua_State *L);
  static int Scene_FindEntitiesWithTag(lua_State *L);
  static int Scene_Instantiate(lua_State *L);

  // ===== Debug Bindings =====

  static int Debug_Log(lua_State *L);
  static int Debug_LogTrace(lua_State *L);
  static int Debug_LogDebug(lua_State *L);
  static int Debug_LogInfo(lua_State *L);
  static int Debug_LogWarn(lua_State *L);
  static int Debug_LogError(lua_State *L);
  static int Debug_DrawLine(lua_State *L);
  static int Debug_DrawRay(lua_State *L);
  static int Debug_DrawSphere(lua_State *L);
  static int Debug_DrawBox(lua_State *L);
  static int Debug_DrawWireSphere(lua_State *L);
  static int Debug_DrawWireBox(lua_State *L);
  static int Debug_Break(lua_State *L);
  static int Debug_Assert(lua_State *L);

  // ===== Resource Bindings =====

  static int Resource_Load(lua_State *L);
  static int Resource_LoadAsync(lua_State *L);
  static int Resource_Unload(lua_State *L);
  static int Resource_IsLoaded(lua_State *L);
  static int Resource_GetPath(lua_State *L);

  // ===== Math Utility Functions =====

  static int Math_Lerp(lua_State *L);
  static int Math_InverseLerp(lua_State *L);
  static int Math_Clamp(lua_State *L);
  static int Math_Clamp01(lua_State *L);
  static int Math_Repeat(lua_State *L);
  static int Math_PingPong(lua_State *L);
  static int Math_MoveTowards(lua_State *L);
  static int Math_SmoothStep(lua_State *L);
  static int Math_SmoothDamp(lua_State *L);
  static int Math_DeltaAngle(lua_State *L);
  static int Math_LerpAngle(lua_State *L);
  static int Math_Sign(lua_State *L);
  static int Math_Approximately(lua_State *L);
  static int Math_PerlinNoise(lua_State *L);

  // ===== Key Code Constants =====

  static void RegisterKeyConstants(lua_State *L);
  static void RegisterMouseButtonConstants(lua_State *L);
  static void RegisterGamepadConstants(lua_State *L);

  static Scene *s_ActiveScene;
};

// ============================================================================
// Binding Registration Helpers
// ============================================================================

/// Helper macro to create Lua bindings with error checking
#define LUA_BINDING_BEGIN(name)                                                \
  static int name(lua_State *L) {                                              \
    try {

#define LUA_BINDING_END                                                        \
  }                                                                            \
  catch (const std::exception &e) {                                            \
    return luaL_error(L, "%s", e.what());                                      \
  }                                                                            \
  catch (...) {                                                                \
    return luaL_error(L, "Unknown error");                                     \
  }                                                                            \
  }

/// Helper to register a module of functions
void LuaRegisterModule(lua_State *L, const char *moduleName,
                       const std::vector<LuaMethodDesc> &functions);

/// Helper to register a class
void LuaRegisterClass(lua_State *L, const char *className,
                      const std::vector<LuaMethodDesc> &methods,
                      const std::vector<LuaMethodDesc> &metamethods,
                      LuaCFunction constructor = nullptr);

} // namespace BeEngine
