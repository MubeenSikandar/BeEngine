#pragma once

#include "Core.hpp"
#include <cstdint>
#include <string_view>

namespace BeEngine {

/**
 * @brief Platform-independent keyboard key codes
 * @note Values are based on GLFW but abstracted for portability
 * @note Use KeyCode::None to represent "no key pressed"
 */
enum class KeyCode : uint16_t {
  // Special values
  None = 0,
  Unknown = 1,

  // Printable keys (ASCII-compatible)
  Space = 32,
  Apostrophe = 39, // '
  Comma = 44,      // ,
  Minus = 45,      // -
  Period = 46,     // .
  Slash = 47,      // /

  // Number keys (top row)
  Key0 = 48,
  Key1 = 49,
  Key2 = 50,
  Key3 = 51,
  Key4 = 52,
  Key5 = 53,
  Key6 = 54,
  Key7 = 55,
  Key8 = 56,
  Key9 = 57,

  Semicolon = 59, // ;
  Equal = 61,     // =

  // Letter keys
  A = 65,
  B = 66,
  C = 67,
  D = 68,
  E = 69,
  F = 70,
  G = 71,
  H = 72,
  I = 73,
  J = 74,
  K = 75,
  L = 76,
  M = 77,
  N = 78,
  O = 79,
  P = 80,
  Q = 81,
  R = 82,
  S = 83,
  T = 84,
  U = 85,
  V = 86,
  W = 87,
  X = 88,
  Y = 89,
  Z = 90,

  LeftBracket = 91,  // [
  Backslash = 92,    // '\'
  RightBracket = 93, // ]
  GraveAccent = 96,  // `

  // Non-US keys (international keyboards)
  World1 = 161,
  World2 = 162,

  // Function keys
  Escape = 256,
  Enter = 257,
  Tab = 258,
  Backspace = 259,
  Insert = 260,
  Delete = 261,

  // Arrow keys
  Right = 262,
  Left = 263,
  Down = 264,
  Up = 265,

  // Navigation keys
  PageUp = 266,
  PageDown = 267,
  Home = 268,
  End = 269,

  // Lock keys
  CapsLock = 280,
  ScrollLock = 281,
  NumLock = 282,
  PrintScreen = 283,
  Pause = 284,

  // Function keys (F1-F25)
  F1 = 290,
  F2 = 291,
  F3 = 292,
  F4 = 293,
  F5 = 294,
  F6 = 295,
  F7 = 296,
  F8 = 297,
  F9 = 298,
  F10 = 299,
  F11 = 300,
  F12 = 301,
  F13 = 302,
  F14 = 303,
  F15 = 304,
  F16 = 305,
  F17 = 306,
  F18 = 307,
  F19 = 308,
  F20 = 309,
  F21 = 310,
  F22 = 311,
  F23 = 312,
  F24 = 313,
  F25 = 314,

  // Keypad (numeric pad)
  Keypad0 = 320,
  Keypad1 = 321,
  Keypad2 = 322,
  Keypad3 = 323,
  Keypad4 = 324,
  Keypad5 = 325,
  Keypad6 = 326,
  Keypad7 = 327,
  Keypad8 = 328,
  Keypad9 = 329,

  KeypadDecimal = 330,
  KeypadDivide = 331,
  KeypadMultiply = 332,
  KeypadSubtract = 333,
  KeypadAdd = 334,
  KeypadEnter = 335,
  KeypadEqual = 336,

  // Modifier keys (specific)
  LeftShift = 340,
  LeftControl = 341,
  LeftAlt = 342,
  LeftSuper = 343, // Windows key / Command key

  RightShift = 344,
  RightControl = 345,
  RightAlt = 346,
  RightSuper = 347,

  Menu = 348 // Context menu key
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Check if a key code represents a printable character
 */
NODISCARD constexpr bool IsPrintable(KeyCode key) noexcept {
  auto k = static_cast<uint16_t>(key);
  return (k >= 32 && k <= 126); // ASCII printable range
}

/**
 * @brief Check if a key code is a modifier key
 */
NODISCARD constexpr bool IsModifier(KeyCode key) noexcept {
  return key == KeyCode::LeftShift || key == KeyCode::RightShift ||
         key == KeyCode::LeftControl || key == KeyCode::RightControl ||
         key == KeyCode::LeftAlt || key == KeyCode::RightAlt ||
         key == KeyCode::LeftSuper || key == KeyCode::RightSuper;
}

/**
 * @brief Check if a key code is a function key (F1-F25)
 */
NODISCARD constexpr bool IsFunctionKey(KeyCode key) noexcept {
  auto k = static_cast<uint16_t>(key);
  return k >= 290 && k <= 314;
}

/**
 * @brief Check if a key code is from the numeric keypad
 */
NODISCARD constexpr bool IsKeypadKey(KeyCode key) noexcept {
  auto k = static_cast<uint16_t>(key);
  return k >= 320 && k <= 336;
}

/**
 * @brief Convert key code to human-readable string (for debugging)
 * @return String representation or "Unknown" if not recognized
 */
NODISCARD constexpr std::string_view ToString(KeyCode key) noexcept {
  switch (key) {
  case KeyCode::None:
    return "None";
  case KeyCode::Space:
    return "Space";
  case KeyCode::Enter:
    return "Enter";
  case KeyCode::Escape:
    return "Escape";
  case KeyCode::Backspace:
    return "Backspace";
  case KeyCode::Tab:
    return "Tab";
  case KeyCode::A:
    return "A";
  case KeyCode::B:
    return "B";
  // ... add more as needed for debugging
  default:
    return "Unknown";
  }
}

/**
 * @brief Get the character representation of a key (if printable)
 * @return The character, or '\0' if not printable
 */
NODISCARD constexpr char ToChar(KeyCode key) noexcept {
  if (!IsPrintable(key)) {
    return '\0';
  }
  return static_cast<char>(key);
}

} // namespace BeEngine
