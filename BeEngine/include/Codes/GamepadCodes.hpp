#pragma once

#include "Core.hpp"
#include <cstdint>
#include <string>
namespace BeEngine {
/**
 * @brief Platform-independent gamepad button codes
 * @note Based on standard gamepad layout (Xbox/PlayStation/Switch compatible)
 * @note Supports multiple simultaneous gamepads (up to 16)
 */
enum class GamepadButton : int16_t {
  // Sentinel
  None = -1,

  // Face buttons(right side)
  // Xbox:        A, B, X, Y
  // PlayStation: Cross, Circle, Square, Triangle
  // Nintendo:    B, A, Y, X
  FaceBottom = 0, // A / Cross / B
  FaceRight = 1,  // B / Circle / A
  FaceLeft = 2,   // X / Square / Y
  FaceTop = 3,    // Y / Triangle / X

  // Shoulder buttons
  LeftBumper = 4,  // LB / L1 / L
  RightBumper = 5, // RB / R1 / R

  // Triggers (digital)
  LeftTrigger = 6,  // LT / L2 / ZL (digital press)
  RightTrigger = 7, // RT / R2 / ZR (digital press)

  // Menu buttons
  Back = 8,   // Back / Share / Minus
  Start = 9,  // Start / Options / Plus
  Guide = 10, // Xbox button / PS button / Home

  // Stick buttons (L3/R3)
  LeftThumb = 11,  // L3 / L-Click
  RightThumb = 12, // R3 / R-Click

  // D-Pad
  DPadUp = 13,
  DPadRight = 14,
  DPadDown = 15,
  DPadLeft = 16,

  // Additional buttons (for extended controllers)
  Extra1 = 17, // Paddle 1 / Back button
  Extra2 = 18, // Paddle 2
  Extra3 = 19, // Paddle 3
  Extra4 = 20, // Paddle 4
  Extra5 = 21, // Touchpad click / Misc

  // Aliases for clarity
  A = FaceBottom,
  B = FaceRight,
  X = FaceLeft,
  Y = FaceTop,
  Cross = FaceBottom,
  Circle = FaceRight,
  Square = FaceLeft,
  Triangle = FaceTop,

  LB = LeftBumper,
  RB = RightBumper,
  LT = LeftTrigger,
  RT = RightTrigger,
  L1 = LeftBumper,
  R1 = RightBumper,
  L2 = LeftTrigger,
  R2 = RightTrigger,
  L3 = LeftThumb,
  R3 = RightThumb,

  Select = Back,
  Share = Back,
  Options = Start,
  Menu = Start,
  Home = Guide,
  Xbox = Guide,
  PlayStation = Guide,

  // Metadata
  Count = 22,
  MaxSupported = 32
};

/**
 * @brief Gamepad analog axes
 * @note Values range from -1.0 (left/down) to +1.0 (right/up)
 */
enum class GamepadAxis : uint8_t {
  // Left stick
  LeftX = 0, // Left (-1) to Right (+1)
  LeftY = 1, // Down (-1) to Up (+1)

  // Right stick
  RightX = 2, // Left (-1) to Right (+1)
  RightY = 3, // Down (-1) to Up (+1)

  // Analog triggers (0.0 to 1.0)
  LeftTrigger = 4,  // LT / L2 analog value
  RightTrigger = 5, // RT / R2 analog value

  // Metadata
  MaxAxes = 6
};

/**
 * @brief Gamepad ID for multi-controller support
 */
enum class GamepadID : uint8_t {
  Gamepad0 = 0, // Player 1
  Gamepad1 = 1, // Player 2
  Gamepad2 = 2, // Player 3
  Gamepad3 = 3, // Player 4
  Gamepad4 = 4,
  Gamepad5 = 5,
  Gamepad6 = 6,
  Gamepad7 = 7,
  Gamepad8 = 8,
  Gamepad9 = 9,
  Gamepad10 = 10,
  Gamepad11 = 11,
  Gamepad12 = 12,
  Gamepad13 = 13,
  Gamepad14 = 14,
  Gamepad15 = 15,

  // Aliases
  Player1 = Gamepad0,
  Player2 = Gamepad1,
  Player3 = Gamepad2,
  Player4 = Gamepad3,

  Any = 0xFF, // Check any connected gamepad
  MaxGamepads = 16
};

/**
 * @brief Gamepad state information
 */
struct GamepadState {
  bool connected{false};
  std::string name{"Unknown"};
  bool isXInput{false}; // Xbox controller protocol
  bool hasRumble{false};
  bool hasGyro{false};
  bool hasTouchpad{false};
  uint8_t buttonCount{0};
  uint8_t axisCount{0};
};

/**
 * @brief Vibration/Rumble motor control
 */
struct GamepadVibration {
  float lowFrequency{0.0F};  // Left motor (0.0 to 1.0)
  float highFrequency{0.0F}; // Right motor (0.0 to 1.0)
  float duration{0.0F};      // Seconds (0 = infinite)
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Check if button is a face button
 */
NODISCARD constexpr bool IsFaceButton(GamepadButton button) noexcept {
  return button == GamepadButton::FaceBottom ||
         button == GamepadButton::FaceRight ||
         button == GamepadButton::FaceLeft || button == GamepadButton::FaceTop;
}

/**
 * @brief Check if button is a shoulder button
 */
NODISCARD constexpr bool IsShoulderButton(GamepadButton button) noexcept {
  return button == GamepadButton::LeftBumper ||
         button == GamepadButton::RightBumper ||
         button == GamepadButton::LeftTrigger ||
         button == GamepadButton::RightTrigger;
}

/**
 * @brief Check if button is a D-Pad button
 */
NODISCARD constexpr bool IsDPadButton(GamepadButton button) noexcept {
  return button == GamepadButton::DPadUp ||
         button == GamepadButton::DPadRight ||
         button == GamepadButton::DPadDown || button == GamepadButton::DPadLeft;
}

/**
 * @brief Check if axis is a stick axis
 */
NODISCARD constexpr bool IsStickAxis(GamepadAxis axis) noexcept {
  return axis == GamepadAxis::LeftX || axis == GamepadAxis::LeftY ||
         axis == GamepadAxis::RightX || axis == GamepadAxis::RightY;
}

/**
 * @brief Check if axis is a trigger axis
 */
NODISCARD constexpr bool IsTriggerAxis(GamepadAxis axis) noexcept {
  return axis == GamepadAxis::LeftTrigger || axis == GamepadAxis::RightTrigger;
}

/**
 * @brief Check if gamepad ID is valid
 */
NODISCARD constexpr bool IsValidGamepadID(GamepadID id) noexcept {
  return static_cast<uint8_t>(id) <
         static_cast<uint8_t>(GamepadID::MaxGamepads);
}

/**
 * @brief Convert gamepad button to string (for debugging/UI)
 */
NODISCARD constexpr std::string ToString(GamepadButton button) noexcept {
  switch (button) {
  case GamepadButton::None:
    return "None";
  case GamepadButton::FaceBottom:
    return "A/Cross";
  case GamepadButton::FaceRight:
    return "B/Circle";
  case GamepadButton::FaceLeft:
    return "X/Square";
  case GamepadButton::FaceTop:
    return "Y/Triangle";
  case GamepadButton::LeftBumper:
    return "LB/L1";
  case GamepadButton::RightBumper:
    return "RB/R1";
  case GamepadButton::LeftTrigger:
    return "LT/L2";
  case GamepadButton::RightTrigger:
    return "RT/R2";
  case GamepadButton::Back:
    return "Back/Share";
  case GamepadButton::Start:
    return "Start/Options";
  case GamepadButton::Guide:
    return "Guide/Home";
  case GamepadButton::LeftThumb:
    return "L3";
  case GamepadButton::RightThumb:
    return "R3";
  case GamepadButton::DPadUp:
    return "D-Pad Up";
  case GamepadButton::DPadRight:
    return "D-Pad Right";
  case GamepadButton::DPadDown:
    return "D-Pad Down";
  case GamepadButton::DPadLeft:
    return "D-Pad Left";
  case GamepadButton::Extra1:
    return "Extra1";
  case GamepadButton::Extra2:
    return "Extra2";
  case GamepadButton::Extra3:
    return "Extra3";
  case GamepadButton::Extra4:
    return "Extra4";
  case GamepadButton::Extra5:
    return "Extra5";
  default:
    return "Unknown";
  }
}

/**
 * @brief Convert gamepad button to Xbox naming
 */
NODISCARD constexpr std::string ToXboxString(GamepadButton button) noexcept {
  switch (button) {
  case GamepadButton::FaceBottom:
    return "A";
  case GamepadButton::FaceRight:
    return "B";
  case GamepadButton::FaceLeft:
    return "X";
  case GamepadButton::FaceTop:
    return "Y";
  case GamepadButton::LeftBumper:
    return "LB";
  case GamepadButton::RightBumper:
    return "RB";
  case GamepadButton::LeftTrigger:
    return "LT";
  case GamepadButton::RightTrigger:
    return "RT";
  case GamepadButton::Back:
    return "Back";
  case GamepadButton::Start:
    return "Start";
  case GamepadButton::Guide:
    return "Xbox";
  case GamepadButton::LeftThumb:
    return "LS";
  case GamepadButton::RightThumb:
    return "RS";
  default:
    return ToString(button);
  }
}

/**
 * @brief Convert gamepad button to PlayStation naming
 */
NODISCARD constexpr std::string
ToPlayStationString(GamepadButton button) noexcept {
  switch (button) {
  case GamepadButton::FaceBottom:
    return "Cross";
  case GamepadButton::FaceRight:
    return "Circle";
  case GamepadButton::FaceLeft:
    return "Square";
  case GamepadButton::FaceTop:
    return "Triangle";
  case GamepadButton::LeftBumper:
    return "L1";
  case GamepadButton::RightBumper:
    return "R1";
  case GamepadButton::LeftTrigger:
    return "L2";
  case GamepadButton::RightTrigger:
    return "R2";
  case GamepadButton::Back:
    return "Share";
  case GamepadButton::Start:
    return "Options";
  case GamepadButton::Guide:
    return "PS";
  case GamepadButton::LeftThumb:
    return "L3";
  case GamepadButton::RightThumb:
    return "R3";
  default:
    return ToString(button);
  }
}

/**
 * @brief Convert gamepad axis to string
 */
NODISCARD constexpr std::string ToString(GamepadAxis axis) noexcept {
  switch (axis) {
  case GamepadAxis::LeftX:
    return "Left Stick X";
  case GamepadAxis::LeftY:
    return "Left Stick Y";
  case GamepadAxis::RightX:
    return "Right Stick X";
  case GamepadAxis::RightY:
    return "Right Stick Y";
  case GamepadAxis::LeftTrigger:
    return "Left Trigger";
  case GamepadAxis::RightTrigger:
    return "Right Trigger";
  default:
    return "Unknown";
  }
}

/**
 * @brief Convert gamepad ID to string
 */
NODISCARD constexpr std::string ToString(GamepadID id) noexcept {
  switch (id) {
  case GamepadID::Gamepad0:
    return "Player 1";
  case GamepadID::Gamepad1:
    return "Player 2";
  case GamepadID::Gamepad2:
    return "Player 3";
  case GamepadID::Gamepad3:
    return "Player 4";
  case GamepadID::Any:
    return "Any Gamepad";
  default: {
    if (IsValidGamepadID(id)) {
      return "Player X"; // Would need runtime formatting
    }
    return "Invalid";
  }
  }
}

/**
 * @brief Get button icon (Unicode character for UI)
 * @note Requires font with gamepad symbols
 */
NODISCARD constexpr const char *GetButtonIcon(GamepadButton button) noexcept {
  switch (button) {
  case GamepadButton::FaceBottom:
    return "🅰";
  case GamepadButton::FaceRight:
    return "🅱";
  case GamepadButton::FaceLeft:
    return "🅇";
  case GamepadButton::FaceTop:
    return "🅈";
  case GamepadButton::DPadUp:
    return "⬆";
  case GamepadButton::DPadRight:
    return "➡";
  case GamepadButton::DPadDown:
    return "⬇";
  case GamepadButton::DPadLeft:
    return "⬅";
  default:
    return "●";
  }
}

} // namespace BeEngine
