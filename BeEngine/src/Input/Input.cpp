#include <Application.hpp>
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

// ===== Static Member Initialization =====]

// Keyboard & Mouse
std::unordered_map<KeyCode, InputState> Input::s_KeyStates;
std::unordered_map<MouseButton, InputState> Input::s_MouseButtonStates;
std::unordered_map<KeyCode, bool> Input::s_PrevKeyStates;
std::unordered_map<MouseButton, bool> Input::s_PrevMouseButtonStates;

glm::vec2 Input::s_MousePosition{0.0F, 0.0F};
glm::vec2 Input::s_LastMousePosition{0.0F, 0.0F};
glm::vec2 Input::s_MouseDelta{0.0F, 0.0F};
glm::vec2 Input::s_MouseScroll{0.0F, 0.0F};

// Gamepad
std::array<Input::GamepadData, static_cast<size_t>(GamepadID::MaxGamepads)>
    Input::s_Gamepads;

// Actions and Axes
std::unordered_map<std::string, InputAction> Input::s_Actions;
std::unordered_map<std::string, InputAxis> Input::s_Axes;
std::unordered_map<std::string, InputAxisBinding> Input::s_AxisBindings;

// System state
InputDevice Input::s_LastUsedDevice{InputDevice::KeyboardMouse};
float Input::s_GamepadDeadZone{0.15F};
GamepadConnectionCallback Input::s_GamepadConnectionCallback{nullptr};

bool Input::s_Initialized{false};
bool Input::s_Enabled{true};
bool Input::s_CursorLocked{false};
float Input::s_DeltaTime{0.016F};

// ===== Initialization =====

void Input::Init() {
  if (s_Initialized) {
    BE_CORE_WARN("Input System already Initialized");
    return;
  }

  BE_CORE_INFO("Initializing Input System...");

  // Register default keyboard axes
  RegisterAxis("Horizontal", KeyCode::A, KeyCode::D, GamepadAxis::LeftX, 0.1F,
               1.0F);
  RegisterAxis("Vertical", KeyCode::S, KeyCode::W, GamepadAxis::LeftY, 0.1F,
               1.0F);
  RegisterAxis("LookHorizontal", KeyCode::Unknown, KeyCode::Unknown,
               GamepadAxis::RightX, 0.0F, 1.0F);
  RegisterAxis("LookVertical", KeyCode::Unknown, KeyCode::Unknown,
               GamepadAxis::RightY, 0.0F, 1.0F);

  // Register default actions
  RegisterAction("Jump", KeyCode::Space, GamepadButton::A);
  RegisterAction("Fire", MouseButton::Button0);
  RegisterAction("AltFire", MouseButton::Button1);
  RegisterAction("Reload", KeyCode::R, GamepadButton::X);
  RegisterAction("Interact", KeyCode::E, GamepadButton::Y);
  RegisterAction("Sprint", KeyCode::LeftShift, GamepadButton::LeftThumb);
  RegisterAction("Crouch", KeyCode::LeftControl, GamepadButton::B);
  RegisterAction("Menu", KeyCode::Escape, GamepadButton::Start);

  // Detect initial gamepad connections
  DetectGamepadConnections();

  s_Initialized = true;
  BE_CORE_INFO("Input System initialized successfully!");
  BE_CORE_INFO("  Connected gamepads: {}", GetConnectedGamepadCount());
}

void Input::Shutdown() {
  BE_CORE_INFO("Shutting down Input System...");

  // Stop all vibrations
  for (size_t i = 0; i < s_Gamepads.size(); ++i) {
    if (s_Gamepads[i].connected) {
      StopGamepadVibration(static_cast<GamepadID>(i));
    }
  }

  s_KeyStates.clear();
  s_MouseButtonStates.clear();
  s_PrevKeyStates.clear();
  s_PrevMouseButtonStates.clear();
  s_Actions.clear();
  s_Axes.clear();
  s_AxisBindings.clear();

  s_Gamepads = {};
  s_GamepadConnectionCallback = nullptr;

  s_Initialized = false;
  BE_CORE_INFO("Input System shut down");
}

void Input::Update() {
  if (!s_Enabled) {
    return;
  }

  auto *window = static_cast<GLFWwindow *>(
      Application::Get().GetWindow().GetNativeWindow());

  // === Update Keyboard ===
  for (int key = static_cast<int>(KeyCode::Space);
       key <= static_cast<int>(KeyCode::Menu); ++key) {
    auto keyCode = static_cast<KeyCode>(key);
    int state = glfwGetKey(window, key);
    bool isDown = (state == GLFW_PRESS || state == GLFW_REPEAT);
    UpdateKeyState(keyCode, isDown);

    if (isDown) {
      s_LastUsedDevice = InputDevice::KeyboardMouse;
    }
  }

  // === Update Mouse Buttons ===
  for (int button = 0; button <= 2; ++button) {
    auto mouseButton = static_cast<MouseButton>(button);
    int state = glfwGetMouseButton(window, button);
    bool isDown = (state == GLFW_PRESS);
    UpdateMouseButtonState(mouseButton, isDown);

    if (isDown) {
      s_LastUsedDevice = InputDevice::KeyboardMouse;
    }
  }

  // === Update Mouse Position ===
  double xPos{}, yPos{};
  glfwGetCursorPos(window, &xPos, &yPos);
  UpdateMousePosition(static_cast<float>(xPos), static_cast<float>(yPos));

  if (glm::length(s_MouseDelta) > 0.1F) {
    s_LastUsedDevice = InputDevice::KeyboardMouse;
  }

  // === Handle Cursor Lock ===
  if (s_CursorLocked) {
    auto &windowRef = Application::Get().GetWindow();
    float centerX = static_cast<float>(windowRef.GetWidth()) / 2.0F;
    float centerY = static_cast<float>(windowRef.GetHeight()) / 2.0F;
    glfwSetCursorPos(window, centerX, centerY);
    s_MousePosition = glm::vec2(centerX, centerY);
  }

  // === Update Gamepads ===
  DetectGamepadConnections();
  UpdateGamepadStates();
}

void Input::EndFrame() {
  // === Transition Keyboard States ===
  for (auto &[key, state] : s_KeyStates) {
    s_PrevKeyStates[key] =
        (state == InputState::Down || state == InputState::Pressed);

    if (state == InputState::Pressed) {
      s_KeyStates[key] = InputState::Down;
    } else if (state == InputState::Released) {
      s_KeyStates[key] = InputState::None;
    }
  }

  // === Transition Mouse States ===
  for (auto &[button, state] : s_MouseButtonStates) {
    s_PrevMouseButtonStates[button] =
        (state == InputState::Down || state == InputState::Pressed);

    if (state == InputState::Pressed) {
      s_MouseButtonStates[button] = InputState::Down;
    } else if (state == InputState::Released) {
      s_MouseButtonStates[button] = InputState::None;
    }
  }

  // === Transition Gamepad States ===
  for (auto &gamepad : s_Gamepads) {
    if (!gamepad.connected) {
      continue;
    }
    for (auto &[button, state] : gamepad.buttonStates) {
      gamepad.prevButtonStates[button] =
          (state == InputState::Down || state == InputState::Pressed);

      if (state == InputState::Pressed) {
        gamepad.buttonStates[button] = InputState::Down;
      } else if (state == InputState::Released) {
        gamepad.buttonStates[button] = InputState::None;
      }
    }

    // Update vibration timer
    if (gamepad.vibrationTimer > 0.0F) {
      gamepad.vibrationTimer -= s_DeltaTime;
      if (gamepad.vibrationTimer <= 0.0F) {
        gamepad.currentVibration = {0.0F, 0.0F, 0.0F};
      }
    }
  }

  // Reset frame-specific data
  s_MouseScroll = glm::vec2(0.0F);
  s_LastMousePosition = s_MousePosition;
}

// ===== Keyboard =====

bool Input::IsKeyPressed(KeyCode key) {
  return GetKeyState(key) == InputState::Pressed;
}

bool Input::IsKeyDown(KeyCode key) {
  auto state = GetKeyState(key);
  return state == InputState::Down || state == InputState::Pressed;
}

bool Input::IsKeyReleased(KeyCode key) {
  return GetKeyState(key) == InputState::Released;
}

// ===== Mouse =====

glm::vec2 Input::GetMousePosition() { return s_MousePosition; }
float Input::GetMouseX() { return s_MousePosition.x; }
float Input::GetMouseY() { return s_MousePosition.y; }
glm::vec2 Input::GetMouseDelta() { return s_MouseDelta; }
glm::vec2 Input::GetMouseScroll() { return s_MouseScroll; }

bool Input::IsMouseButtonPressed(MouseButton button) {
  return GetMouseButtonState(button) == InputState::Pressed;
}

bool Input::IsMouseButtonDown(MouseButton button) {
  auto state = GetMouseButtonState(button);
  return state == InputState::Down || state == InputState::Pressed;
}

bool Input::IsMouseButtonReleased(MouseButton button) {
  return GetMouseButtonState(button) == InputState::Released;
}

// ===== Gamepad =====

bool Input::IsGamepadConnected(GamepadID id) {
  if (id == GamepadID::Any) {
    return GetConnectedGamepadCount() > 0;
  }

  if (!IsValidGamepadID(id)) {
    return false;
  }
  return s_Gamepads[static_cast<size_t>(id)].connected;
}

int Input::GetConnectedGamepadCount() {
  int count = 0;
  for (const auto &gamepad : s_Gamepads) {
    if (gamepad.connected) {
      ++count;
    }
  }
  return count;
}

GamepadState Input::GetGamepadState(GamepadID id) {
  if (!IsValidGamepadID(id)) {
    return {};
  }
  return s_Gamepads[static_cast<size_t>(id)].state;
}

bool Input::IsGamepadButtonPressed(GamepadButton button, GamepadID id) {
  return GetGamepadButtonState(button, id) == InputState::Pressed;
}

bool Input::IsGamepadButtonDown(GamepadButton button, GamepadID id) {
  auto state = GetGamepadButtonState(button, id);
  return state == InputState::Down || state == InputState::Pressed;
}

bool Input::IsGamepadButtonReleased(GamepadButton button, GamepadID id) {
  return GetGamepadButtonState(button, id) == InputState::Released;
}

float Input::GetGamepadAxis(GamepadAxis axis, GamepadID id,
                            bool applyDeadZone) {
  if (!IsValidGamepadID(id) || !s_Gamepads[static_cast<size_t>(id)].connected) {
    return 0.0F;
  }

  if (static_cast<size_t>(axis) >= static_cast<size_t>(GamepadAxis::MaxAxes)) {
    return 0.0F;
  }

  float value =
      s_Gamepads[static_cast<size_t>(id)].axes[static_cast<size_t>(axis)];

  if (applyDeadZone) {
    value = ApplyDeadZone(value, s_GamepadDeadZone);
  }

  return value;
}

glm::vec2 Input::GetGamepadStick(bool leftStick, GamepadID id) {
  GamepadAxis xAxis = leftStick ? GamepadAxis::LeftX : GamepadAxis::RightX;
  GamepadAxis yAxis = leftStick ? GamepadAxis::LeftY : GamepadAxis::RightY;

  glm::vec2 stick(GetGamepadAxis(xAxis, id), GetGamepadAxis(yAxis, id));

  // Apply radial dead zone
  float magnitude = glm::length(stick);
  if (magnitude < s_GamepadDeadZone) {
    return glm::vec2(0.0F);
  }

  // Normalize and remap
  stick = glm::normalize(stick);
  float remapped = (magnitude - s_GamepadDeadZone) / (1.0F - s_GamepadDeadZone);
  return stick * remapped;
}

void Input::SetGamepadVibration(const GamepadVibration &vibration,
                                GamepadID id) {
  if (!IsValidGamepadID(id) || !s_Gamepads[static_cast<size_t>(id)].connected) {
    return;
  }

  s_Gamepads[static_cast<size_t>(id)].currentVibration = vibration;
  s_Gamepads[static_cast<size_t>(id)].vibrationTimer = vibration.duration;

  BE_CORE_TRACE(
      "Gamepad {} vibration set: L={:.2f}, H={:.2f}, Duration={:.2f}s",
      static_cast<int>(id), vibration.lowFrequency, vibration.highFrequency,
      vibration.duration);
}

void Input::StopGamepadVibration(GamepadID id) {
  if (!IsValidGamepadID(id)) {
    return;
  }
  s_Gamepads[static_cast<size_t>(id)].currentVibration = {0.0F, 0.0F, 0.0F};
  s_Gamepads[static_cast<size_t>(id)].vibrationTimer = 0.0F;
}

void Input::SetGamepadDeadZone(float deadZone) {
  s_GamepadDeadZone = std::clamp(deadZone, 0.0F, 0.9F);
  BE_CORE_INFO("Global gamepad dead zone set to {:.2f}", s_GamepadDeadZone);
}

void Input::SetGamepadConnectionCallback(GamepadConnectionCallback callback) {
  s_GamepadConnectionCallback = std::move(callback);
}

// ===== Actions =====

void Input::RegisterAction(const std::string &name, KeyCode primaryKey,
                           KeyCode secondaryKey) {
  InputAction action;
  action.name = name;
  action.primaryKey = primaryKey;
  action.secondaryKey = secondaryKey;
  action.enabled = true;
  s_Actions[name] = action;

  BE_CORE_TRACE("Registered action '{}': Key1={}, Key2={}", name,
                static_cast<int>(primaryKey), static_cast<int>(secondaryKey));
}

void Input::RegisterAction(const std::string &name, MouseButton mouseButton) {
  InputAction action;
  action.name = name;
  action.mouseButton = mouseButton;
  action.enabled = true;
  s_Actions[name] = action;

  BE_CORE_TRACE("Registered action '{}': Mouse={}", name,
                static_cast<int>(mouseButton));
}

void Input::RegisterAction(const std::string &name,
                           GamepadButton gamepadButton) {
  InputAction action;
  action.name = name;
  action.gamepadButton = gamepadButton;
  action.enabled = true;
  s_Actions[name] = action;

  BE_CORE_TRACE("Registered action '{}': Gamepad={}", name,
                ToString(gamepadButton));
}

void Input::RegisterAction(const std::string &name, KeyCode key,
                           GamepadButton gamepadButton) {
  InputAction action;
  action.name = name;
  action.primaryKey = key;
  action.gamepadButton = gamepadButton;
  action.enabled = true;
  s_Actions[name] = action;

  BE_CORE_TRACE("Registered composite action '{}': Key={}, Gamepad={}", name,
                static_cast<int>(key), ToString(gamepadButton));
}

bool Input::IsActionPressed(const std::string &name, GamepadID gamepadId) {
  auto it = s_Actions.find(name);
  if (it == s_Actions.end() || !it->second.enabled) {
    return false;
  }

  const auto &action = it->second;

  // Check keyboard
  if (IsKeyPressed(action.primaryKey) || IsKeyPressed(action.secondaryKey)) {
    s_Actions[name].lastDevice = InputDevice::KeyboardMouse;
    return true;
  }

  // Check mouse
  if (action.mouseButton != MouseButton::None &&
      IsMouseButtonPressed(action.mouseButton)) {
    s_Actions[name].lastDevice = InputDevice::KeyboardMouse;
    return true;
  }

  // Check gamepad
  if (action.gamepadButton != GamepadButton::None) {
    if (IsGamepadButtonPressed(action.gamepadButton, gamepadId)) {
      s_Actions[name].lastDevice = InputDevice::Gamepad;
      return true;
    }
  }

  return false;
}

bool Input::IsActionDown(const std::string &name, GamepadID gamepadId) {
  auto it = s_Actions.find(name);
  if (it == s_Actions.end() || !it->second.enabled) {
    return false;
  }

  const auto &action = it->second;

  if (IsKeyDown(action.primaryKey) || IsKeyDown(action.secondaryKey)) {
    s_Actions[name].lastDevice = InputDevice::KeyboardMouse;
    return true;
  }

  if (action.mouseButton != MouseButton::None &&
      IsMouseButtonDown(action.mouseButton)) {
    s_Actions[name].lastDevice = InputDevice::KeyboardMouse;
    return true;
  }

  if (action.gamepadButton != GamepadButton::None) {
    if (IsGamepadButtonDown(action.gamepadButton, gamepadId)) {
      s_Actions[name].lastDevice = InputDevice::Gamepad;
      return true;
    }
  }

  return false;
}

bool Input::IsActionReleased(const std::string &name, GamepadID gamepadId) {
  auto it = s_Actions.find(name);
  if (it == s_Actions.end() || !it->second.enabled) {
    return false;
  }

  const auto &action = it->second;

  return IsKeyReleased(action.primaryKey) ||
         IsKeyReleased(action.secondaryKey) ||
         (action.mouseButton != MouseButton::None &&
          IsMouseButtonReleased(action.mouseButton)) ||
         (action.gamepadButton != GamepadButton::None &&
          IsGamepadButtonReleased(action.gamepadButton, gamepadId));
}

InputDevice Input::GetActionDevice(const std::string &name) {
  auto it = s_Actions.find(name);
  return (it != s_Actions.end()) ? it->second.lastDevice
                                 : InputDevice::KeyboardMouse;
}

void Input::RebindAction(const std::string &name, KeyCode newKey,
                         bool isPrimary) {
  auto it = s_Actions.find(name);
  if (it == s_Actions.end()) {
    BE_CORE_WARN("Cannot rebind action '{}': Not found", name);
    return;
  }

  if (isPrimary) {
    it->second.primaryKey = newKey;
  } else {
    it->second.secondaryKey = newKey;
  }

  BE_CORE_INFO("Rebound action '{}': {} key = {}", name,
               isPrimary ? "Primary" : "Secondary", static_cast<int>(newKey));
}

void Input::RebindAction(const std::string &name, GamepadButton newButton) {
  auto it = s_Actions.find(name);
  if (it == s_Actions.end()) {
    BE_CORE_WARN("Cannot rebind action '{}': Not found", name);
    return;
  }

  it->second.gamepadButton = newButton;
  BE_CORE_INFO("Rebound action '{}': Gamepad button = {}", name,
               ToString(newButton));
}

// ===== Axes =====

void Input::RegisterAxis(const std::string &name, KeyCode negativeKey,
                         KeyCode positiveKey, GamepadAxis gamepadAxis,
                         float deadZone, float sensitivity) {
  InputAxis axis;
  axis.deadZone = deadZone;
  axis.sensitivity = sensitivity;
  s_Axes[name] = axis;

  InputAxisBinding binding;
  binding.negativeKey = negativeKey;
  binding.positiveKey = positiveKey;
  binding.gamepadAxis = gamepadAxis;
  s_AxisBindings[name] = binding;

  BE_CORE_TRACE("Registered axis '{}': Keys={},{}, GamepadAxis={}", name,
                static_cast<int>(negativeKey), static_cast<int>(positiveKey),
                ToString(gamepadAxis));
}

float Input::GetAxis(const std::string &name, GamepadID gamepadId) {
  auto axisIt = s_Axes.find(name);
  auto bindingIt = s_AxisBindings.find(name);

  if (axisIt == s_Axes.end() || bindingIt == s_AxisBindings.end()) {
    return 0.0F;
  }

  const auto &axis = axisIt->second;
  const auto &binding = bindingIt->second;

  float value = 0.0F;

  // Try gamepad first if connected
  if (binding.gamepadAxis != GamepadAxis::MaxAxes &&
      IsGamepadConnected(gamepadId)) {
    value = GetGamepadAxis(binding.gamepadAxis, gamepadId);
    if (binding.invertGamepadAxis) {
      value = -value;
    }

    if (std::abs(value) > 0.01F) {
      s_LastUsedDevice = InputDevice::Gamepad;
    }
  }

  // Fall back to keyboard if no gamepad input
  if (std::abs(value) < 0.01F) {
    if (IsKeyDown(binding.negativeKey)) {
      value -= 1.0F;
    }
    if (IsKeyDown(binding.positiveKey)) {
      value += 1.0F;
    }
  }

  // Apply dead zone
  value = ApplyDeadZone(value, axis.deadZone);

  // Apply sensitivity
  value *= axis.sensitivity;

  // Apply inversion
  if (axis.inverted) {
    value = -value;
  }

  return std::clamp(value, -1.0F, 1.0F);
}

float Input::GetAxisRaw(const std::string &name, GamepadID gamepadId) {
  auto bindingIt = s_AxisBindings.find(name);
  if (bindingIt == s_AxisBindings.end()) {
    return 0.0F;
  }

  const auto &binding = bindingIt->second;

  // Try gamepad
  if (binding.gamepadAxis != GamepadAxis::MaxAxes &&
      IsGamepadConnected(gamepadId)) {
    float value = GetGamepadAxis(binding.gamepadAxis, gamepadId, false);
    return binding.invertGamepadAxis ? -value : value;
  }

  // Keyboard
  float value = 0.0F;
  if (IsKeyDown(binding.negativeKey)) {
    value -= 1.0F;
  }
  if (IsKeyDown(binding.positiveKey)) {
    value += 1.0F;
  }
  return value;
}

void Input::SetAxisSmoothing(const std::string &name, bool enabled,
                             float speed) {
  auto it = s_Axes.find(name);
  if (it != s_Axes.end()) {
    it->second.smoothing = enabled;
    it->second.smoothingSpeed = speed;
  }
}

// ===== Utility =====

InputDevice Input::GetLastUsedDevice() { return s_LastUsedDevice; }
bool Input::IsUsingGamepad() {
  return s_LastUsedDevice == InputDevice::Gamepad;
}
bool Input::IsUsingKeyboardMouse() {
  return s_LastUsedDevice == InputDevice::KeyboardMouse;
}

// ===== Utility =====

void Input::SetCursorLocked(bool locked) {
  s_CursorLocked = locked;
  auto *window = static_cast<GLFWwindow *>(
      Application::Get().GetWindow().GetNativeWindow());
  glfwSetInputMode(window, GLFW_CURSOR,
                   locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Input::SetCursorVisible(bool visible) {
  auto *window = static_cast<GLFWwindow *>(
      Application::Get().GetWindow().GetNativeWindow());
  glfwSetInputMode(window, GLFW_CURSOR,
                   visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
}

std::string Input::GetKeyName(KeyCode key) {
  return std::string(ToString(key));
}

std::string Input::GetButtonName(GamepadButton button, bool useXboxNames) {
  return std::string(useXboxNames ? ToXboxString(button)
                                  : ToPlayStationString(button));
}

// ===== Debug =====

void Input::PrintBindings() {
  BE_CORE_INFO("===== Input Bindings =====");
  BE_CORE_INFO("Actions:");
  for (const auto &[name, action] : s_Actions) {
    BE_CORE_INFO("  {}: Key={}, Gamepad={}", name,
                 static_cast<int>(action.primaryKey),
                 ToString(action.gamepadButton));
  }

  BE_CORE_INFO("Axes:");
  for (const auto &[name, binding] : s_AxisBindings) {
    BE_CORE_INFO("  {}: Keys={},{}, Gamepad={}", name,
                 static_cast<int>(binding.negativeKey),
                 static_cast<int>(binding.positiveKey),
                 ToString(binding.gamepadAxis));
  }
}

void Input::PrintGamepadInfo(GamepadID id) {
  if (!IsValidGamepadID(id)) {
    BE_CORE_WARN("Invalid gamepad ID: {}", static_cast<int>(id));
    return;
  }

  const auto &gamepad = s_Gamepads[static_cast<size_t>(id)];
  BE_CORE_INFO("Gamepad {}: {}", static_cast<int>(id),
               gamepad.connected ? "Connected" : "Disconnected");

  if (gamepad.connected) {
    BE_CORE_INFO("  Name: {}", gamepad.state.name);
    BE_CORE_INFO("  Buttons: {}, Axes: {}", gamepad.state.buttonCount,
                 gamepad.state.axisCount);
  }
}

// ===== Internal =====

void Input::UpdateKeyState(KeyCode key, bool isDown) {
  bool wasDown = s_PrevKeyStates[key];

  if (isDown && !wasDown) {
    s_KeyStates[key] = InputState::Pressed;
  } else if (!isDown && wasDown) {
    s_KeyStates[key] = InputState::Released;
  } else if (isDown) {
    s_KeyStates[key] = InputState::Down;
  } else {
    s_KeyStates[key] = InputState::None;
  }
}

void Input::UpdateMouseButtonState(MouseButton button, bool isDown) {
  bool wasDown = s_PrevMouseButtonStates[button];

  if (isDown && !wasDown) {
    s_MouseButtonStates[button] = InputState::Pressed;
  } else if (!isDown && wasDown) {
    s_MouseButtonStates[button] = InputState::Released;
  } else if (isDown) {
    s_MouseButtonStates[button] = InputState::Down;
  } else {
    s_MouseButtonStates[button] = InputState::None;
  }
}

void Input::UpdateMousePosition(float x, float y) {
  s_LastMousePosition = s_MousePosition;
  s_MousePosition = glm::vec2(x, y);
  s_MouseDelta = s_MousePosition - s_LastMousePosition;
}

void Input::UpdateMouseScroll(float xOffset, float yOffset) {
  s_MouseScroll = glm::vec2(xOffset, yOffset);
}

void Input::DetectGamepadConnections() {
  for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; ++i) {
    bool connected = glfwJoystickPresent(i) == GLFW_TRUE;
    bool wasConnected = s_Gamepads[i].connected;

    if (connected != wasConnected) {
      s_Gamepads[i].connected = connected;

      if (connected) {
        const char *name = glfwGetJoystickName(i);
        s_Gamepads[i].state.name = name ? name : "Unknown Gamepad";
        s_Gamepads[i].state.connected = true;

        BE_CORE_INFO("Gamepad {} connected: {}", i, s_Gamepads[i].state.name);
      } else {
        BE_CORE_INFO("Gamepad {} disconnected", i);
        s_Gamepads[i].state = {};
      }

      if (s_GamepadConnectionCallback) {
        s_GamepadConnectionCallback(static_cast<GamepadID>(i), connected);
      }
    }
  }
}

void Input::UpdateGamepadStates() {
  for (int jid = GLFW_JOYSTICK_1; jid <= GLFW_JOYSTICK_LAST; ++jid) {
    auto &gamepad = s_Gamepads[jid];
    if (!gamepad.connected) {
      continue;
    }

    // Prefer GLFW gamepad API (standardized mapping)
    if (glfwJoystickIsGamepad(jid)) {
      GLFWgamepadstate state{};
      if (!glfwGetGamepadState(jid, &state)) {
        continue;
      }
      // --- Buttons ---
      for (int b = 0; b <= GLFW_GAMEPAD_BUTTON_LAST; ++b) {
        auto button = static_cast<GamepadButton>(b);
        bool isDown = state.buttons[b] == GLFW_PRESS;

        bool wasDown = gamepad.prevButtonStates[button];
        InputState &inputState = gamepad.buttonStates[button];

        if (isDown && !wasDown) {
          inputState = InputState::Pressed;
        } else if (!isDown && wasDown) {
          inputState = InputState::Released;
        } else if (isDown) {
          inputState = InputState::Down;
        } else {
          inputState = InputState::None;
        }

        if (isDown) {
          s_LastUsedDevice = InputDevice::Gamepad;
        }
      }

      // --- Axes ---
      auto mapTrigger = [](float raw) { return (raw + 1.0F) * 0.5F; };

      gamepad.axes[static_cast<size_t>(GamepadAxis::LeftX)] =
          state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
      gamepad.axes[static_cast<size_t>(GamepadAxis::LeftY)] =
          state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
      gamepad.axes[static_cast<size_t>(GamepadAxis::RightX)] =
          state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
      gamepad.axes[static_cast<size_t>(GamepadAxis::RightY)] =
          state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
      gamepad.axes[static_cast<size_t>(GamepadAxis::LeftTrigger)] =
          mapTrigger(state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER]);
      gamepad.axes[static_cast<size_t>(GamepadAxis::RightTrigger)] =
          mapTrigger(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER]);

      gamepad.state.axisCount = static_cast<uint8_t>(GamepadAxis::MaxAxes);
      gamepad.state.isXInput = true;
    } else {
      // --- Fallback: Raw joystick API ---
      int buttonCount = 0;
      const unsigned char *buttons = glfwGetJoystickButtons(jid, &buttonCount);
      gamepad.state.buttonCount = static_cast<uint8_t>(buttonCount);

      for (int b = 0;
           b < std::min(buttonCount, static_cast<int>(GamepadButton::Count));
           ++b) {
        auto button = static_cast<GamepadButton>(b);
        bool isDown = buttons[b] == GLFW_PRESS;

        bool wasDown = gamepad.prevButtonStates[button];
        InputState &inputState = gamepad.buttonStates[button];

        if (isDown && !wasDown) {
          inputState = InputState::Pressed;
        } else if (!isDown && wasDown) {
          inputState = InputState::Released;
        } else if (isDown) {
          inputState = InputState::Down;
        } else {
          inputState = InputState::None;
        }
      }

      int axisCount = 0;
      const float *axes = glfwGetJoystickAxes(jid, &axisCount);
      gamepad.state.axisCount = static_cast<uint8_t>(axisCount);

      for (int a = 0;
           a < std::min(axisCount, static_cast<int>(GamepadAxis::MaxAxes));
           ++a) {
        gamepad.axes[a] = axes[a];
      }

      gamepad.state.isXInput = false;
    }

    // --- Rumble (placeholder – platform-specific backend needed) ---
    // GLFW does not support vibration natively
    // Hook SDL / XInput / HIDAPI here later
  }
}

InputState Input::GetKeyState(KeyCode key) {
  auto it = s_KeyStates.find(key);
  return (it != s_KeyStates.end()) ? it->second : InputState::None;
}

InputState Input::GetMouseButtonState(MouseButton button) {
  auto it = s_MouseButtonStates.find(button);
  return (it != s_MouseButtonStates.end()) ? it->second : InputState::None;
}

InputState Input::GetGamepadButtonState(GamepadButton button, GamepadID id) {
  if (id == GamepadID::Any) {
    // Check all connected gamepads
    for (const auto &gamepad : s_Gamepads) {
      if (!gamepad.connected) {
        continue;
      }
      auto it = gamepad.buttonStates.find(button);
      if (it != gamepad.buttonStates.end() && it->second != InputState::None) {
        return it->second;
      }
    }
    return InputState::None;
  }

  if (!IsValidGamepadID(id)) {
    return InputState::None;
  }

  const auto &gamepad = s_Gamepads[static_cast<size_t>(id)];
  if (!gamepad.connected) {
  }
  return InputState::None;

  auto it = gamepad.buttonStates.find(button);
  return (it != gamepad.buttonStates.end()) ? it->second : InputState::None;
}

float Input::ApplyDeadZone(float value, float deadZone) {
  if (std::abs(value) < deadZone) {
    return 0.0F;
  }

  // Remap value from [deadZone, 1.0] to [0.0, 1.0]
  float sign = (value > 0.0F) ? 1.0F : -1.0F;
  float absValue = std::abs(value);
  float remapped = (absValue - deadZone) / (1.0F - deadZone);

  return sign * remapped;
}

} // namespace BeEngine
