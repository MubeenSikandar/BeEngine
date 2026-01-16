#include <PCH/BeEnginePCH.hpp>

namespace BeEngine::ScriptAPI {

// ============================================================================
// Internal State
// ============================================================================

static Scene *s_ActiveScene = nullptr;
static SceneManager *s_SceneManager = nullptr;

// Script callbacks storage (entity UUID -> callbacks)
static std::unordered_map<be_entity_t, be_script_callbacks_t> s_ScriptCallbacks;

// ============================================================================
// Initialization (called by engine)
// ============================================================================

void Initialize(SceneManager *sceneManager) {
  s_SceneManager = sceneManager;
  if (s_SceneManager) {
    s_ActiveScene = s_SceneManager->GetActiveScene();

    // Register for scene change callbacks
    s_SceneManager->OnSceneChange(
        [](Scene *oldScene, Scene *newScene) { s_ActiveScene = newScene; });
  }
  BE_CORE_INFO("[ScriptAPI] Initialized");
}

void Shutdown() {
  s_ScriptCallbacks.clear();
  s_ActiveScene = nullptr;
  s_SceneManager = nullptr;
  BE_CORE_INFO("[ScriptAPI] Shutdown");
}

void SetActiveScene(Scene *scene) { s_ActiveScene = scene; }

Scene *GetActiveScene() { return s_ActiveScene; }

// ============================================================================
// Script Update (called by engine each frame)
// ============================================================================

void UpdateScripts(float deltaTime) {
  for (auto &[entityId, callbacks] : s_ScriptCallbacks) {
    if (callbacks.on_update) {
      callbacks.on_update(entityId, deltaTime, callbacks.user_data);
    }
  }
}

void FixedUpdateScripts(float fixedDelta) {
  for (auto &[entityId, callbacks] : s_ScriptCallbacks) {
    if (callbacks.on_fixed_update) {
      callbacks.on_fixed_update(entityId, fixedDelta, callbacks.user_data);
    }
  }
}

void LateUpdateScripts(float deltaTime) {
  for (auto &[entityId, callbacks] : s_ScriptCallbacks) {
    if (callbacks.on_late_update) {
      callbacks.on_late_update(entityId, deltaTime, callbacks.user_data);
    }
  }
}

// ============================================================================
// Helper Functions
// ============================================================================

static Entity GetEntityFromHandle(be_entity_t handle) {
  if (!s_ActiveScene || handle == BE_INVALID_ENTITY) {
    return Entity();
  }
  return s_ActiveScene->GetEntityByUUID(handle);
}

static be_vec3_t ToBeVec3(const glm::vec3 &v) {
  be_vec3_t result = {v.x, v.y, v.z};
  return result;
}

static glm::vec3 ToGlmVec3(be_vec3_t v) { return glm::vec3(v.x, v.y, v.z); }

static be_vec2_t ToBeVec2(const glm::vec2 &v) {
  be_vec2_t result = {v.x, v.y};
  return result;
}

static be_quat_t ToBeQuat(const glm::quat &q) {
  be_quat_t result = {q.x, q.y, q.z, q.w};
  return result;
}

static glm::quat ToGlmQuat(be_quat_t q) {
  return glm::quat(q.w, q.x, q.y, q.z);
}

} // namespace BeEngine::ScriptAPI

// ============================================================================
// C API Implementation
// ============================================================================

extern "C" {

using namespace BeEngine;

// ============================================================================
// Entity API
// ============================================================================

be_entity_t be_entity_create(const char *name) {
  if (!ScriptAPI::s_ActiveScene) {
    BE_CORE_ERROR("[ScriptAPI] No active scene");
    return BE_INVALID_ENTITY;
  }

  Entity entity =
      ScriptAPI::s_ActiveScene->CreateEntity(name ? name : "Entity");
  return entity.GetUUID();
}

be_entity_t be_entity_create_with_uuid(uint64_t uuid, const char *name) {
  if (!ScriptAPI::s_ActiveScene) {
    BE_CORE_ERROR("[ScriptAPI] No active scene");
    return BE_INVALID_ENTITY;
  }

  Entity entity = ScriptAPI::s_ActiveScene->CreateEntityWithUUID(
      uuid, name ? name : "Entity");
  return entity.GetUUID();
}

void be_entity_destroy(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  // Call destroy callback if registered
  auto it = ScriptAPI::s_ScriptCallbacks.find(entity);
  if (it != ScriptAPI::s_ScriptCallbacks.end()) {
    if (it->second.on_destroy) {
      it->second.on_destroy(entity, it->second.user_data);
    }
    ScriptAPI::s_ScriptCallbacks.erase(it);
  }

  ScriptAPI::s_ActiveScene->DestroyEntity(e);
}

be_entity_t be_entity_duplicate(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return BE_INVALID_ENTITY;
  }

  Entity duplicated = ScriptAPI::s_ActiveScene->DuplicateEntity(e);
  return duplicated.IsValid() ? duplicated.GetUUID() : BE_INVALID_ENTITY;
}

bool be_entity_is_valid(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  return e.IsValid();
}

const char *be_entity_get_name(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return "";
  }

  auto *tag = e.TryGetComponent<TagComponent>();
  return tag ? tag->Tag.c_str() : "";
}

void be_entity_set_name(be_entity_t entity, const char *name) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid() || !name) {
    return;
  }

  auto *tag = e.TryGetComponent<TagComponent>();
  if (tag) {
    tag->Tag = name;
  }
}

bool be_entity_is_enabled(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return false;
  }

  auto *tag = e.TryGetComponent<TagComponent>();
  return tag ? tag->Enabled : false;
}

void be_entity_set_enabled(be_entity_t entity, bool enabled) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *tag = e.TryGetComponent<TagComponent>();
  if (tag) {
    bool wasEnabled = tag->Enabled;
    tag->Enabled = enabled;

    // Fire enable/disable callbacks
    auto it = ScriptAPI::s_ScriptCallbacks.find(entity);
    if (it != ScriptAPI::s_ScriptCallbacks.end()) {
      if (enabled && !wasEnabled && it->second.on_enable) {
        it->second.on_enable(entity, it->second.user_data);
      } else if (!enabled && wasEnabled && it->second.on_disable) {
        it->second.on_disable(entity, it->second.user_data);
      }
    }
  }
}

be_entity_t be_entity_get_parent(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return BE_INVALID_ENTITY;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (!transform || !transform->Parent.IsValid()) {
    return BE_INVALID_ENTITY;
  }

  return transform->Parent.GetID();
}

void be_entity_set_parent(be_entity_t entity, be_entity_t parent) {
  if (!ScriptAPI::s_ActiveScene) {
    return;
  }

  Entity child = ScriptAPI::GetEntityFromHandle(entity);
  Entity parentEntity = ScriptAPI::GetEntityFromHandle(parent);

  if (child.IsValid() && parentEntity.IsValid()) {
    ScriptAPI::s_ActiveScene->SetParent(child, parentEntity);
  }
}

void be_entity_remove_parent(be_entity_t entity) {
  if (!ScriptAPI::s_ActiveScene) {
    return;
  }

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (e.IsValid()) {
    ScriptAPI::s_ActiveScene->RemoveParent(e);
  }
}

uint32_t be_entity_get_child_count(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return 0;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  return transform ? static_cast<uint32_t>(transform->Children.size()) : 0;
}

be_entity_t be_entity_get_child(be_entity_t entity, uint32_t index) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return BE_INVALID_ENTITY;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (!transform || index >= transform->Children.size()) {
    return BE_INVALID_ENTITY;
  }

  return transform->Children[index].GetID();
}

be_entity_t be_entity_find_by_name(const char *name) {
  if (!ScriptAPI::s_ActiveScene || !name) {
    return BE_INVALID_ENTITY;
  }

  Entity e = ScriptAPI::s_ActiveScene->FindEntityByName(name);
  return e.IsValid() ? e.GetUUID() : BE_INVALID_ENTITY;
}

// ============================================================================
// Transform API
// ============================================================================

be_vec3_t be_transform_get_position(be_entity_t entity) {
  be_vec3_t result = {0.0F, 0.0F, 0.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    return ScriptAPI::ToBeVec3(transform->GetPosition());
  }
  return result;
}

void be_transform_set_position(be_entity_t entity, be_vec3_t position) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    transform->SetPosition(ScriptAPI::ToGlmVec3(position));
  }
}

be_quat_t be_transform_get_rotation(be_entity_t entity) {
  be_quat_t result = {0.0F, 0.0F, 0.0F, 1.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    return ScriptAPI::ToBeQuat(transform->GetRotation());
  }
  return result;
}

void be_transform_set_rotation(be_entity_t entity, be_quat_t rotation) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    transform->SetRotation(ScriptAPI::ToGlmQuat(rotation));
  }
}

be_vec3_t be_transform_get_euler_angles(be_entity_t entity) {
  be_vec3_t result = {0.0F, 0.0F, 0.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::vec3 euler = glm::degrees(glm::eulerAngles(transform->GetRotation()));
    return ScriptAPI::ToBeVec3(euler);
  }
  return result;
}

void be_transform_set_euler_angles(be_entity_t entity, be_vec3_t euler) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    transform->SetRotation(glm::radians(ScriptAPI::ToGlmVec3(euler)));
  }
}

be_vec3_t be_transform_get_scale(be_entity_t entity) {
  be_vec3_t result = {1.0F, 1.0F, 1.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    return ScriptAPI::ToBeVec3(transform->GetScale());
  }
  return result;
}

void be_transform_set_scale(be_entity_t entity, be_vec3_t scale) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    transform->SetScale(ScriptAPI::ToGlmVec3(scale));
  }
}

be_vec3_t be_transform_get_world_position(be_entity_t entity) {
  be_vec3_t result = {0.0F, 0.0F, 0.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::vec3 worldPos = glm::vec3(transform->WorldMatrix[3]);
    return ScriptAPI::ToBeVec3(worldPos);
  }
  return result;
}

be_vec3_t be_transform_get_forward(be_entity_t entity) {
  be_vec3_t result = {0.0F, 0.0F, -1.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::vec3 forward = -glm::normalize(glm::vec3(transform->WorldMatrix[2]));
    return ScriptAPI::ToBeVec3(forward);
  }
  return result;
}

be_vec3_t be_transform_get_right(be_entity_t entity) {
  be_vec3_t result = {1.0F, 0.0F, 0.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::vec3 right = glm::normalize(glm::vec3(transform->WorldMatrix[0]));
    return ScriptAPI::ToBeVec3(right);
  }
  return result;
}

be_vec3_t be_transform_get_up(be_entity_t entity) {
  be_vec3_t result = {0.0F, 1.0F, 0.0F};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::vec3 up = glm::normalize(glm::vec3(transform->WorldMatrix[1]));
    return ScriptAPI::ToBeVec3(up);
  }
  return result;
}

void be_transform_translate(be_entity_t entity, be_vec3_t delta) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::vec3 current = transform->GetPosition();
    transform->SetPosition(current + ScriptAPI::ToGlmVec3(delta));
  }
}

void be_transform_rotate(be_entity_t entity, be_vec3_t euler_delta) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::quat current = transform->GetRotation();
    glm::quat deltaRot =
        glm::quat(glm::radians(ScriptAPI::ToGlmVec3(euler_delta)));
    transform->SetRotation(deltaRot * current);
  }
}

void be_transform_look_at(be_entity_t entity, be_vec3_t target) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *transform = e.TryGetComponent<TransformComponent>();
  if (transform) {
    glm::vec3 pos = transform->GetPosition();
    glm::vec3 targetPos = ScriptAPI::ToGlmVec3(target);
    glm::vec3 direction = glm::normalize(targetPos - pos);

    // Calculate rotation to look at target
    glm::vec3 up(0.0F, 1.0F, 0.0F);
    glm::mat4 lookMat = glm::lookAt(pos, targetPos, up);
    glm::quat rotation = glm::conjugate(glm::quat_cast(lookMat));

    transform->SetRotation(rotation);
  }
}

// ============================================================================
// Input API
// ============================================================================

bool be_input_get_key(int32_t key_code) {
  return Input::IsKeyDown(static_cast<KeyCode>(key_code));
}

bool be_input_get_key_down(int32_t key_code) {
  return Input::IsKeyPressed(static_cast<KeyCode>(key_code));
}

bool be_input_get_key_up(int32_t key_code) {
  return Input::IsKeyReleased(static_cast<KeyCode>(key_code));
}

bool be_input_get_mouse_button(int32_t button) {
  return Input::IsMouseButtonDown(static_cast<MouseButton>(button));
}

bool be_input_get_mouse_button_down(int32_t button) {
  return Input::IsMouseButtonPressed(static_cast<MouseButton>(button));
}

bool be_input_get_mouse_button_up(int32_t button) {
  return Input::IsMouseButtonReleased(static_cast<MouseButton>(button));
}

be_vec2_t be_input_get_mouse_position(void) {
  glm::vec2 pos = Input::GetMousePosition();
  return ScriptAPI::ToBeVec2(pos);
}

be_vec2_t be_input_get_mouse_delta(void) {
  glm::vec2 delta = Input::GetMouseDelta();
  return ScriptAPI::ToBeVec2(delta);
}

be_vec2_t be_input_get_mouse_scroll(void) {
  glm::vec2 scroll = Input::GetMouseScroll();
  return ScriptAPI::ToBeVec2(scroll);
}

void be_input_set_cursor_visible(bool visible) {
  Input::SetCursorVisible(visible);
}

void be_input_set_cursor_locked(bool locked) { Input::SetCursorLocked(locked); }

bool be_input_is_gamepad_connected(int32_t gamepad_id) {
  return Input::IsGamepadConnected(static_cast<GamepadID>(gamepad_id));
}

bool be_input_get_gamepad_button(int32_t gamepad_id, int32_t button) {
  return Input::IsGamepadButtonDown(static_cast<GamepadButton>(button),
                                    static_cast<GamepadID>(gamepad_id));
}

bool be_input_get_gamepad_button_down(int32_t gamepad_id, int32_t button) {
  return Input::IsGamepadButtonPressed(static_cast<GamepadButton>(button),
                                       static_cast<GamepadID>(gamepad_id));
}

float be_input_get_gamepad_axis(int32_t gamepad_id, int32_t axis) {
  return Input::GetGamepadAxis(static_cast<GamepadAxis>(axis),
                               static_cast<GamepadID>(gamepad_id));
}

be_vec2_t be_input_get_gamepad_stick(int32_t gamepad_id, bool left_stick) {
  glm::vec2 stick =
      Input::GetGamepadStick(left_stick, static_cast<GamepadID>(gamepad_id));
  return ScriptAPI::ToBeVec2(stick);
}

bool be_input_is_action_pressed(const char *action_name) {
  if (!action_name) {
    return false;
  }
  return Input::IsActionPressed(action_name);
}

bool be_input_is_action_down(const char *action_name) {
  if (!action_name) {
    return false;
  }
  return Input::IsActionDown(action_name);
}

bool be_input_is_action_released(const char *action_name) {
  if (!action_name) {
    return false;
  }
  return Input::IsActionReleased(action_name);
}

float be_input_get_axis(const char *axis_name) {
  if (!axis_name) {
    return 0.0F;
  }
  return Input::GetAxis(axis_name);
}

float be_input_get_axis_raw(const char *axis_name) {
  if (!axis_name) {
    return 0.0F;
  }
  return Input::GetAxisRaw(axis_name);
}

// ============================================================================
// Time API
// ============================================================================

float be_time_get_delta(void) { return Time::GetDeltaTime(); }

float be_time_get_unscaled_delta(void) { return Time::GetUnscaledDeltaTime(); }

float be_time_get_fixed_delta(void) { return Time::GetFixedDeltaTime(); }

float be_time_get_elapsed(void) { return Time::GetTime(); }

float be_time_get_unscaled_elapsed(void) { return Time::GetUnscaledTime(); }

float be_time_get_scale(void) { return Time::GetTimeScale(); }

void be_time_set_scale(float scale) { Time::SetTimeScale(scale); }

uint64_t be_time_get_frame_count(void) { return Time::GetFrameCount(); }

float be_time_get_fps(void) { return Time::GetFPS(); }

bool be_time_is_paused(void) { return Time::IsPaused(); }

void be_time_set_paused(bool paused) { Time::SetPaused(paused); }

// ============================================================================
// Scene API
// ============================================================================

be_scene_t be_scene_get_active(void) {
  // Return a simple ID (we can expand this later)
  return ScriptAPI::s_ActiveScene ? 1 : BE_INVALID_SCENE;
}

const char *be_scene_get_name(be_scene_t scene) {
  if (!ScriptAPI::s_ActiveScene || scene == BE_INVALID_SCENE) {
    return "";
  }
  return ScriptAPI::s_ActiveScene->GetName().c_str();
}

void be_scene_load(const char *scene_path) {
  if (!ScriptAPI::s_SceneManager || !scene_path) {
    BE_CORE_ERROR("[ScriptAPI] Cannot load scene: no scene manager");
    return;
  }
  ScriptAPI::s_SceneManager->LoadScene(scene_path);
}

void be_scene_load_additive(const char *scene_path) {
  if (!ScriptAPI::s_SceneManager || !scene_path) {
    BE_CORE_ERROR("[ScriptAPI] Cannot load scene: no scene manager");
    return;
  }
  ScriptAPI::s_SceneManager->LoadSceneAdditive(scene_path);
}

uint32_t be_scene_get_entity_count(void) {
  if (!ScriptAPI::s_ActiveScene) {
    return 0;
  }
  return static_cast<uint32_t>(ScriptAPI::s_ActiveScene->GetEntityCount());
}

// ============================================================================
// Physics API (Stubs - implement when Physics system is added)
// ============================================================================

bool be_rigidbody_exists(be_entity_t entity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return false;
  }
  return e.HasComponent<RigidbodyComponent>();
}

void be_rigidbody_add_force(be_entity_t entity, be_vec3_t force) {
  // TODO: Implement when physics engine is integrated
  BE_CORE_WARN("[ScriptAPI] be_rigidbody_add_force not yet implemented");
}

void be_rigidbody_add_impulse(be_entity_t entity, be_vec3_t impulse) {
  // TODO: Implement when physics engine is integrated
  BE_CORE_WARN("[ScriptAPI] be_rigidbody_add_impulse not yet implemented");
}

be_vec3_t be_rigidbody_get_velocity(be_entity_t entity) {
  be_vec3_t result = {0.0f, 0.0f, 0.0f};

  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return result;
  }

  auto *rb = e.TryGetComponent<RigidbodyComponent>();
  if (rb) {
    return ScriptAPI::ToBeVec3(rb->LinearVelocity);
  }
  return result;
}

void be_rigidbody_set_velocity(be_entity_t entity, be_vec3_t velocity) {
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    return;
  }

  auto *rb = e.TryGetComponent<RigidbodyComponent>();
  if (rb) {
    rb->LinearVelocity = ScriptAPI::ToGlmVec3(velocity);
  }
}

be_raycast_hit_t be_physics_raycast(be_vec3_t origin, be_vec3_t direction,
                                    float max_distance) {
  be_raycast_hit_t result = {};
  result.hit = false;
  result.distance = max_distance;
  result.entity = BE_INVALID_ENTITY;

  // TODO: Implement when physics engine is integrated
  BE_CORE_WARN("[ScriptAPI] be_physics_raycast not yet implemented");

  return result;
}

// ============================================================================
// Audio API (Stubs - implement when Audio system is added)
// ============================================================================

be_audio_source_t be_audio_play(const char *clip_path) {
  // TODO: Implement when audio system is integrated
  BE_CORE_WARN("[ScriptAPI] be_audio_play not yet implemented");
  return 0;
}

be_audio_source_t be_audio_play_at_position(const char *clip_path,
                                            be_vec3_t position) {
  // TODO: Implement when audio system is integrated
  BE_CORE_WARN("[ScriptAPI] be_audio_play_at_position not yet implemented");
  return 0;
}

void be_audio_stop(be_audio_source_t source) {
  // TODO: Implement when audio system is integrated
}

void be_audio_set_volume(be_audio_source_t source, float volume) {
  // TODO: Implement when audio system is integrated
}

void be_audio_set_master_volume(float volume) {
  // TODO: Implement when audio system is integrated
}

// ============================================================================
// Debug & Logging API
// ============================================================================

void be_log(be_log_level_t level, const char *message) {
  if (!message) {
    return;
  }

  switch (level) {
  case BE_LOG_TRACE:
    BE_CORE_TRACE("[Script] {}", message);
    break;
  case BE_LOG_DEBUG:
    BE_CORE_DEBUG("[Script] {}", message);
    break;
  case BE_LOG_INFO:
    BE_CORE_INFO("[Script] {}", message);
    break;
  case BE_LOG_WARN:
    BE_CORE_WARN("[Script] {}", message);
    break;
  case BE_LOG_ERROR:
    BE_CORE_ERROR("[Script] {}", message);
    break;
  case BE_LOG_CRITICAL:
    BE_CORE_CRITICAL("[Script] {}", message);
    break;
  }
}

void be_log_trace(const char *message) { be_log(BE_LOG_TRACE, message); }

void be_log_debug(const char *message) { be_log(BE_LOG_DEBUG, message); }

void be_log_info(const char *message) { be_log(BE_LOG_INFO, message); }

void be_log_warn(const char *message) { be_log(BE_LOG_WARN, message); }

void be_log_error(const char *message) { be_log(BE_LOG_ERROR, message); }

void be_debug_draw_line(be_vec3_t start, be_vec3_t end, be_color_t color,
                        float duration) {
  // TODO: Implement debug drawing system
}

void be_debug_draw_sphere(be_vec3_t center, float radius, be_color_t color,
                          float duration) {
  // TODO: Implement debug drawing system
}

void be_debug_draw_box(be_vec3_t center, be_vec3_t size, be_color_t color,
                       float duration) {
  // TODO: Implement debug drawing system
}

// ============================================================================
// Script Registration
// ============================================================================

bool be_script_register(be_entity_t entity, be_script_callbacks_t *callbacks) {
  if (entity == BE_INVALID_ENTITY || !callbacks) {
    return false;
  }

  // Check if entity is valid
  Entity e = ScriptAPI::GetEntityFromHandle(entity);
  if (!e.IsValid()) {
    BE_CORE_ERROR("[ScriptAPI] Cannot register script for invalid entity");
    return false;
  }

  // Store callbacks
  ScriptAPI::s_ScriptCallbacks[entity] = *callbacks;

  // Call on_create immediately
  if (callbacks->on_create) {
    callbacks->on_create(entity, callbacks->user_data);
  }

  BE_CORE_TRACE("[ScriptAPI] Registered script for entity {}", entity);
  return true;
}

void be_script_unregister(be_entity_t entity) {
  auto it = ScriptAPI::s_ScriptCallbacks.find(entity);
  if (it != ScriptAPI::s_ScriptCallbacks.end()) {
    // Call on_destroy
    if (it->second.on_destroy) {
      it->second.on_destroy(entity, it->second.user_data);
    }
    ScriptAPI::s_ScriptCallbacks.erase(it);
    BE_CORE_TRACE("[ScriptAPI] Unregistered script for entity {}", entity);
  }
}

// ============================================================================
// Memory
// ============================================================================

void *be_alloc(size_t size) { return std::malloc(size); }

void be_free(void *ptr) { std::free(ptr); }

char *be_strdup(const char *str) {
  if (!str) {
    return nullptr;
  }
  size_t len = std::strlen(str) + 1;
  char *result = static_cast<char *>(be_alloc(len));
  if (result) {
    std::memcpy(result, str, len);
  }
  return result;
}
} // extern "C"
