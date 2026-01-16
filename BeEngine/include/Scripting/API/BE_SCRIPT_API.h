// include/Scripting/API/BE_SCRIPT_API.h
#ifndef BE_SCRIPT_API_H
#define BE_SCRIPT_API_H

// ============================================================================
// Cross-language compatible includes
// ============================================================================

#ifdef __cplusplus
// C++ includes
#include <cstddef>
#include <cstdint>
#else
// C includes
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// API Version
// ============================================================================

#define BE_SCRIPT_API_VERSION_MAJOR 1
#define BE_SCRIPT_API_VERSION_MINOR 0
#define BE_SCRIPT_API_VERSION_PATCH 0

// ============================================================================
// Export Macros
// ============================================================================

#if defined(_WIN32) || defined(_WIN64)
#ifdef BE_DYNAMIC_LINK
#ifdef BE_BUILD_DLL
#define BE_SCRIPT_API __declspec(dllexport)
#else
#define BE_SCRIPT_API __declspec(dllimport)
#endif
#else
#define BE_SCRIPT_API
#endif
#else
#define BE_SCRIPT_API __attribute__((visibility("default")))
#endif

// ============================================================================
// Basic Types
// ============================================================================

typedef uint64_t be_entity_t;
typedef uint32_t be_scene_t;
typedef uint32_t be_resource_t;
typedef uint32_t be_audio_source_t;

#define BE_INVALID_ENTITY ((be_entity_t)0)
#define BE_INVALID_SCENE ((be_scene_t)0)

// ============================================================================
// Math Types
// ============================================================================

typedef struct be_vec2_t {
  float x, y;
} be_vec2_t;

typedef struct be_vec3_t {
  float x, y, z;
} be_vec3_t;

typedef struct be_vec4_t {
  float x, y, z, w;
} be_vec4_t;

typedef struct be_quat_t {
  float x, y, z, w;
} be_quat_t;

typedef struct be_color_t {
  float r, g, b, a;
} be_color_t;

// Inline constructors
static inline be_vec2_t be_vec2(float x, float y) {
  be_vec2_t v = {x, y};
  return v;
}

static inline be_vec3_t be_vec3(float x, float y, float z) {
  be_vec3_t v = {x, y, z};
  return v;
}

static inline be_vec4_t be_vec4(float x, float y, float z, float w) {
  be_vec4_t v = {x, y, z, w};
  return v;
}

static inline be_quat_t be_quat_identity(void) {
  be_quat_t q = {0.0f, 0.0f, 0.0f, 1.0f};
  return q;
}

static inline be_color_t be_color(float r, float g, float b, float a) {
  be_color_t c = {r, g, b, a};
  return c;
}

// ============================================================================
// Key Codes
// ============================================================================

typedef enum be_key_code_t {
  BE_KEY_NONE = 0,
  BE_KEY_UNKNOWN = 1,
  BE_KEY_SPACE = 32,
  BE_KEY_APOSTROPHE = 39,
  BE_KEY_COMMA = 44,
  BE_KEY_MINUS = 45,
  BE_KEY_PERIOD = 46,
  BE_KEY_SLASH = 47,
  BE_KEY_0 = 48,
  BE_KEY_1,
  BE_KEY_2,
  BE_KEY_3,
  BE_KEY_4,
  BE_KEY_5,
  BE_KEY_6,
  BE_KEY_7,
  BE_KEY_8,
  BE_KEY_9,
  BE_KEY_SEMICOLON = 59,
  BE_KEY_EQUAL = 61,
  BE_KEY_A = 65,
  BE_KEY_B,
  BE_KEY_C,
  BE_KEY_D,
  BE_KEY_E,
  BE_KEY_F,
  BE_KEY_G,
  BE_KEY_H,
  BE_KEY_I,
  BE_KEY_J,
  BE_KEY_K,
  BE_KEY_L,
  BE_KEY_M,
  BE_KEY_N,
  BE_KEY_O,
  BE_KEY_P,
  BE_KEY_Q,
  BE_KEY_R,
  BE_KEY_S,
  BE_KEY_T,
  BE_KEY_U,
  BE_KEY_V,
  BE_KEY_W,
  BE_KEY_X,
  BE_KEY_Y,
  BE_KEY_Z,
  BE_KEY_LEFT_BRACKET = 91,
  BE_KEY_BACKSLASH = 92,
  BE_KEY_RIGHT_BRACKET = 93,
  BE_KEY_GRAVE_ACCENT = 96,
  BE_KEY_ESCAPE = 256,
  BE_KEY_ENTER = 257,
  BE_KEY_TAB = 258,
  BE_KEY_BACKSPACE = 259,
  BE_KEY_INSERT = 260,
  BE_KEY_DELETE = 261,
  BE_KEY_RIGHT = 262,
  BE_KEY_LEFT = 263,
  BE_KEY_DOWN = 264,
  BE_KEY_UP = 265,
  BE_KEY_PAGE_UP = 266,
  BE_KEY_PAGE_DOWN = 267,
  BE_KEY_HOME = 268,
  BE_KEY_END = 269,
  BE_KEY_CAPS_LOCK = 280,
  BE_KEY_SCROLL_LOCK = 281,
  BE_KEY_NUM_LOCK = 282,
  BE_KEY_PRINT_SCREEN = 283,
  BE_KEY_PAUSE = 284,
  BE_KEY_F1 = 290,
  BE_KEY_F2,
  BE_KEY_F3,
  BE_KEY_F4,
  BE_KEY_F5,
  BE_KEY_F6,
  BE_KEY_F7,
  BE_KEY_F8,
  BE_KEY_F9,
  BE_KEY_F10,
  BE_KEY_F11,
  BE_KEY_F12,
  BE_KEY_KEYPAD_0 = 320,
  BE_KEY_KEYPAD_1,
  BE_KEY_KEYPAD_2,
  BE_KEY_KEYPAD_3,
  BE_KEY_KEYPAD_4,
  BE_KEY_KEYPAD_5,
  BE_KEY_KEYPAD_6,
  BE_KEY_KEYPAD_7,
  BE_KEY_KEYPAD_8,
  BE_KEY_KEYPAD_9,
  BE_KEY_KEYPAD_DECIMAL = 330,
  BE_KEY_KEYPAD_DIVIDE = 331,
  BE_KEY_KEYPAD_MULTIPLY = 332,
  BE_KEY_KEYPAD_SUBTRACT = 333,
  BE_KEY_KEYPAD_ADD = 334,
  BE_KEY_KEYPAD_ENTER = 335,
  BE_KEY_KEYPAD_EQUAL = 336,
  BE_KEY_LEFT_SHIFT = 340,
  BE_KEY_LEFT_CONTROL = 341,
  BE_KEY_LEFT_ALT = 342,
  BE_KEY_LEFT_SUPER = 343,
  BE_KEY_RIGHT_SHIFT = 344,
  BE_KEY_RIGHT_CONTROL = 345,
  BE_KEY_RIGHT_ALT = 346,
  BE_KEY_RIGHT_SUPER = 347,
  BE_KEY_MENU = 348
} be_key_code_t;

// ============================================================================
// Mouse Buttons
// ============================================================================

typedef enum be_mouse_button_t {
  BE_MOUSE_BUTTON_LEFT = 0,
  BE_MOUSE_BUTTON_RIGHT = 1,
  BE_MOUSE_BUTTON_MIDDLE = 2,
  BE_MOUSE_BUTTON_3 = 3,
  BE_MOUSE_BUTTON_4 = 4,
  BE_MOUSE_BUTTON_5 = 5
} be_mouse_button_t;

// ============================================================================
// Gamepad
// ============================================================================

typedef enum be_gamepad_id_t {
  BE_GAMEPAD_0 = 0,
  BE_GAMEPAD_1 = 1,
  BE_GAMEPAD_2 = 2,
  BE_GAMEPAD_3 = 3,
  BE_GAMEPAD_ANY = 0xFF
} be_gamepad_id_t;

typedef enum be_gamepad_button_t {
  BE_GAMEPAD_BUTTON_A = 0,
  BE_GAMEPAD_BUTTON_B = 1,
  BE_GAMEPAD_BUTTON_X = 2,
  BE_GAMEPAD_BUTTON_Y = 3,
  BE_GAMEPAD_BUTTON_LEFT_BUMPER = 4,
  BE_GAMEPAD_BUTTON_RIGHT_BUMPER = 5,
  BE_GAMEPAD_BUTTON_BACK = 6,
  BE_GAMEPAD_BUTTON_START = 7,
  BE_GAMEPAD_BUTTON_GUIDE = 8,
  BE_GAMEPAD_BUTTON_LEFT_THUMB = 9,
  BE_GAMEPAD_BUTTON_RIGHT_THUMB = 10,
  BE_GAMEPAD_BUTTON_DPAD_UP = 11,
  BE_GAMEPAD_BUTTON_DPAD_RIGHT = 12,
  BE_GAMEPAD_BUTTON_DPAD_DOWN = 13,
  BE_GAMEPAD_BUTTON_DPAD_LEFT = 14
} be_gamepad_button_t;

typedef enum be_gamepad_axis_t {
  BE_GAMEPAD_AXIS_LEFT_X = 0,
  BE_GAMEPAD_AXIS_LEFT_Y = 1,
  BE_GAMEPAD_AXIS_RIGHT_X = 2,
  BE_GAMEPAD_AXIS_RIGHT_Y = 3,
  BE_GAMEPAD_AXIS_LEFT_TRIGGER = 4,
  BE_GAMEPAD_AXIS_RIGHT_TRIGGER = 5
} be_gamepad_axis_t;

// ============================================================================
// Log Levels
// ============================================================================

typedef enum be_log_level_t {
  BE_LOG_TRACE = 0,
  BE_LOG_DEBUG = 1,
  BE_LOG_INFO = 2,
  BE_LOG_WARN = 3,
  BE_LOG_ERROR = 4,
  BE_LOG_CRITICAL = 5
} be_log_level_t;

// ============================================================================
// Script Lifecycle Callbacks
// ============================================================================

// Function pointer types for script callbacks
typedef void (*be_on_create_fn)(be_entity_t entity, void *user_data);
typedef void (*be_on_destroy_fn)(be_entity_t entity, void *user_data);
typedef void (*be_on_update_fn)(be_entity_t entity, float delta_time,
                                void *user_data);
typedef void (*be_on_fixed_update_fn)(be_entity_t entity, float fixed_delta,
                                      void *user_data);
typedef void (*be_on_late_update_fn)(be_entity_t entity, float delta_time,
                                     void *user_data);
typedef void (*be_on_enable_fn)(be_entity_t entity, void *user_data);
typedef void (*be_on_disable_fn)(be_entity_t entity, void *user_data);

// Collision callbacks
typedef void (*be_on_collision_enter_fn)(be_entity_t self, be_entity_t other,
                                         be_vec3_t contact_point,
                                         be_vec3_t normal, void *user_data);
typedef void (*be_on_collision_stay_fn)(be_entity_t self, be_entity_t other,
                                        void *user_data);
typedef void (*be_on_collision_exit_fn)(be_entity_t self, be_entity_t other,
                                        void *user_data);
typedef void (*be_on_trigger_enter_fn)(be_entity_t self, be_entity_t other,
                                       void *user_data);
typedef void (*be_on_trigger_exit_fn)(be_entity_t self, be_entity_t other,
                                      void *user_data);

// Complete callbacks structure
typedef struct be_script_callbacks_t {
  // Lifecycle
  be_on_create_fn on_create;
  be_on_destroy_fn on_destroy;
  be_on_update_fn on_update;
  be_on_fixed_update_fn on_fixed_update;
  be_on_late_update_fn on_late_update;
  be_on_enable_fn on_enable;
  be_on_disable_fn on_disable;

  // Physics (for future use)
  be_on_collision_enter_fn on_collision_enter;
  be_on_collision_stay_fn on_collision_stay;
  be_on_collision_exit_fn on_collision_exit;
  be_on_trigger_enter_fn on_trigger_enter;
  be_on_trigger_exit_fn on_trigger_exit;

  // User data passed to all callbacks
  void *user_data;
} be_script_callbacks_t;

// ============================================================================
// Physics Types
// ============================================================================

typedef struct be_raycast_hit_t {
  bool hit;
  be_vec3_t point;
  be_vec3_t normal;
  float distance;
  be_entity_t entity;
} be_raycast_hit_t;

// ============================================================================
// Entity API
// ============================================================================

BE_SCRIPT_API be_entity_t be_entity_create(const char *name);
BE_SCRIPT_API be_entity_t be_entity_create_with_uuid(uint64_t uuid,
                                                     const char *name);
BE_SCRIPT_API void be_entity_destroy(be_entity_t entity);
BE_SCRIPT_API be_entity_t be_entity_duplicate(be_entity_t entity);
BE_SCRIPT_API bool be_entity_is_valid(be_entity_t entity);

BE_SCRIPT_API const char *be_entity_get_name(be_entity_t entity);
BE_SCRIPT_API void be_entity_set_name(be_entity_t entity, const char *name);
BE_SCRIPT_API bool be_entity_is_enabled(be_entity_t entity);
BE_SCRIPT_API void be_entity_set_enabled(be_entity_t entity, bool enabled);

BE_SCRIPT_API be_entity_t be_entity_get_parent(be_entity_t entity);
BE_SCRIPT_API void be_entity_set_parent(be_entity_t entity, be_entity_t parent);
BE_SCRIPT_API void be_entity_remove_parent(be_entity_t entity);
BE_SCRIPT_API uint32_t be_entity_get_child_count(be_entity_t entity);
BE_SCRIPT_API be_entity_t be_entity_get_child(be_entity_t entity,
                                              uint32_t index);

BE_SCRIPT_API be_entity_t be_entity_find_by_name(const char *name);

// ============================================================================
// Transform API
// ============================================================================

BE_SCRIPT_API be_vec3_t be_transform_get_position(be_entity_t entity);
BE_SCRIPT_API void be_transform_set_position(be_entity_t entity,
                                             be_vec3_t position);

BE_SCRIPT_API be_quat_t be_transform_get_rotation(be_entity_t entity);
BE_SCRIPT_API void be_transform_set_rotation(be_entity_t entity,
                                             be_quat_t rotation);

BE_SCRIPT_API be_vec3_t be_transform_get_euler_angles(be_entity_t entity);
BE_SCRIPT_API void be_transform_set_euler_angles(be_entity_t entity,
                                                 be_vec3_t euler);

BE_SCRIPT_API be_vec3_t be_transform_get_scale(be_entity_t entity);
BE_SCRIPT_API void be_transform_set_scale(be_entity_t entity, be_vec3_t scale);

BE_SCRIPT_API be_vec3_t be_transform_get_world_position(be_entity_t entity);
BE_SCRIPT_API be_vec3_t be_transform_get_forward(be_entity_t entity);
BE_SCRIPT_API be_vec3_t be_transform_get_right(be_entity_t entity);
BE_SCRIPT_API be_vec3_t be_transform_get_up(be_entity_t entity);

BE_SCRIPT_API void be_transform_translate(be_entity_t entity, be_vec3_t delta);
BE_SCRIPT_API void be_transform_rotate(be_entity_t entity,
                                       be_vec3_t euler_delta);
BE_SCRIPT_API void be_transform_look_at(be_entity_t entity, be_vec3_t target);

// ============================================================================
// Input API
// ============================================================================

// Keyboard
BE_SCRIPT_API bool be_input_get_key(int32_t key_code);
BE_SCRIPT_API bool be_input_get_key_down(int32_t key_code);
BE_SCRIPT_API bool be_input_get_key_up(int32_t key_code);

// Mouse
BE_SCRIPT_API bool be_input_get_mouse_button(int32_t button);
BE_SCRIPT_API bool be_input_get_mouse_button_down(int32_t button);
BE_SCRIPT_API bool be_input_get_mouse_button_up(int32_t button);
BE_SCRIPT_API be_vec2_t be_input_get_mouse_position(void);
BE_SCRIPT_API be_vec2_t be_input_get_mouse_delta(void);
BE_SCRIPT_API be_vec2_t be_input_get_mouse_scroll(void);
BE_SCRIPT_API void be_input_set_cursor_visible(bool visible);
BE_SCRIPT_API void be_input_set_cursor_locked(bool locked);

// Gamepad
BE_SCRIPT_API bool be_input_is_gamepad_connected(int32_t gamepad_id);
BE_SCRIPT_API bool be_input_get_gamepad_button(int32_t gamepad_id,
                                               int32_t button);
BE_SCRIPT_API bool be_input_get_gamepad_button_down(int32_t gamepad_id,
                                                    int32_t button);
BE_SCRIPT_API float be_input_get_gamepad_axis(int32_t gamepad_id, int32_t axis);
BE_SCRIPT_API be_vec2_t be_input_get_gamepad_stick(int32_t gamepad_id,
                                                   bool left_stick);

// Actions
BE_SCRIPT_API bool be_input_is_action_pressed(const char *action_name);
BE_SCRIPT_API bool be_input_is_action_down(const char *action_name);
BE_SCRIPT_API bool be_input_is_action_released(const char *action_name);
BE_SCRIPT_API float be_input_get_axis(const char *axis_name);
BE_SCRIPT_API float be_input_get_axis_raw(const char *axis_name);

// ============================================================================
// Time API
// ============================================================================

BE_SCRIPT_API float be_time_get_delta(void);
BE_SCRIPT_API float be_time_get_unscaled_delta(void);
BE_SCRIPT_API float be_time_get_fixed_delta(void);
BE_SCRIPT_API float be_time_get_elapsed(void);
BE_SCRIPT_API float be_time_get_unscaled_elapsed(void);
BE_SCRIPT_API float be_time_get_scale(void);
BE_SCRIPT_API void be_time_set_scale(float scale);
BE_SCRIPT_API uint64_t be_time_get_frame_count(void);
BE_SCRIPT_API float be_time_get_fps(void);
BE_SCRIPT_API bool be_time_is_paused(void);
BE_SCRIPT_API void be_time_set_paused(bool paused);

// ============================================================================
// Scene API
// ============================================================================

BE_SCRIPT_API be_scene_t be_scene_get_active(void);
BE_SCRIPT_API const char *be_scene_get_name(be_scene_t scene);
BE_SCRIPT_API void be_scene_load(const char *scene_path);
BE_SCRIPT_API void be_scene_load_additive(const char *scene_path);
BE_SCRIPT_API uint32_t be_scene_get_entity_count(void);

// ============================================================================
// Physics API
// ============================================================================

BE_SCRIPT_API bool be_rigidbody_exists(be_entity_t entity);
BE_SCRIPT_API void be_rigidbody_add_force(be_entity_t entity, be_vec3_t force);
BE_SCRIPT_API void be_rigidbody_add_impulse(be_entity_t entity,
                                            be_vec3_t impulse);
BE_SCRIPT_API be_vec3_t be_rigidbody_get_velocity(be_entity_t entity);
BE_SCRIPT_API void be_rigidbody_set_velocity(be_entity_t entity,
                                             be_vec3_t velocity);
BE_SCRIPT_API be_raycast_hit_t be_physics_raycast(be_vec3_t origin,
                                                  be_vec3_t direction,
                                                  float max_distance);

// ============================================================================
// Audio API
// ============================================================================

BE_SCRIPT_API be_audio_source_t be_audio_play(const char *clip_path);
BE_SCRIPT_API be_audio_source_t be_audio_play_at_position(const char *clip_path,
                                                          be_vec3_t position);
BE_SCRIPT_API void be_audio_stop(be_audio_source_t source);
BE_SCRIPT_API void be_audio_set_volume(be_audio_source_t source, float volume);
BE_SCRIPT_API void be_audio_set_master_volume(float volume);

// ============================================================================
// Debug & Logging API
// ============================================================================

BE_SCRIPT_API void be_log(be_log_level_t level, const char *message);
BE_SCRIPT_API void be_log_trace(const char *message);
BE_SCRIPT_API void be_log_debug(const char *message);
BE_SCRIPT_API void be_log_info(const char *message);
BE_SCRIPT_API void be_log_warn(const char *message);
BE_SCRIPT_API void be_log_error(const char *message);

BE_SCRIPT_API void be_debug_draw_line(be_vec3_t start, be_vec3_t end,
                                      be_color_t color, float duration);
BE_SCRIPT_API void be_debug_draw_sphere(be_vec3_t center, float radius,
                                        be_color_t color, float duration);
BE_SCRIPT_API void be_debug_draw_box(be_vec3_t center, be_vec3_t size,
                                     be_color_t color, float duration);

// ============================================================================
// Script Registration
// ============================================================================

BE_SCRIPT_API bool be_script_register(be_entity_t entity,
                                      be_script_callbacks_t *callbacks);
BE_SCRIPT_API void be_script_unregister(be_entity_t entity);

// ============================================================================
// Memory
// ============================================================================

BE_SCRIPT_API void *be_alloc(size_t size);
BE_SCRIPT_API void be_free(void *ptr);
BE_SCRIPT_API char *be_strdup(const char *str);

#ifdef __cplusplus
}
#endif

#endif // BE_SCRIPT_API_H
