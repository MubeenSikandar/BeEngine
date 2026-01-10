#pragma once

#include <Core.hpp>
#include <cstdint>
#include <string_view>

namespace BeEngine {

/**
 * @brief Platform-independent mouse button codes
 * @note Button numbering follows GLFW convention (Button0 = Left)
 * @note For platform-specific code, use GetPlatformButtonCode()
 */
enum class MouseButton : uint16_t {
  // Sentinel values
  None = 0xFF, // No button or invalid button

  // Standard buttons (guaranteed on all mice)
  Left = 0,   // Primary button (left for right-handed, right for left-handed)
  Right = 1,  // Secondary button
  Middle = 2, // Wheel click

  // Extended buttons (common on modern mice)
  Button3 = 3, // Often "Back" button
  Button4 = 4, // Often "Forward" button
  Button5 = 5,
  Button6 = 6,
  Button7 = 7,

  // Additional buttons for gaming mice
  Button8 = 8,
  Button9 = 9,
  Button10 = 10,
  Button11 = 11,
  Button12 = 12,
  Button13 = 13,
  Button14 = 14,
  Button15 = 15,

  // Aliases for common buttons
  Primary = Left,
  Secondary = Right,
  Tertiary = Middle,
  Back = Button3,    // Browser back
  Forward = Button4, // Browser forward

  // Legacy numeric aliases (for GLFW compatibility)
  Button0 = Left,
  Button1 = Right,
  Button2 = Middle,

  // Metadata
  LastButton = Button15,
  MaxButtons = 16 // For array sizing
};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Check if button is one of the standard three (left/right/middle)
 */
NODISCARD constexpr bool IsStandardButton(MouseButton button) noexcept {
  return button == MouseButton::Left || button == MouseButton::Right ||
         button == MouseButton::Middle;
}

/**
 * @brief Check if button code is valid
 */
NODISCARD constexpr bool IsValidButton(MouseButton button) noexcept {
  auto btn = static_cast<uint16_t>(button);
  return btn < static_cast<uint16_t>(MouseButton::MaxButtons);
}

/**
 * @brief Get zero-based index for array lookups
 * @return Index 0-15, or -1 if invalid
 */
NODISCARD constexpr int GetButtonIndex(MouseButton button) noexcept {
  if (!IsValidButton(button)) {
    return -1;
  }
  return static_cast<int>(button);
}

/**
 * @brief Convert mouse button to human-readable string
 * @return String representation or "Unknown" if not recognized
 */
NODISCARD constexpr std::string_view ToString(MouseButton button) noexcept {
  switch (button) {
  case MouseButton::None:
    return "None";
  case MouseButton::Left:
    return "Left";
  case MouseButton::Right:
    return "Right";
  case MouseButton::Middle:
    return "Middle";
  case MouseButton::Button3:
    return "Button3 (Back)";
  case MouseButton::Button4:
    return "Button4 (Forward)";
  case MouseButton::Button5:
    return "Button5";
  case MouseButton::Button6:
    return "Button6";
  case MouseButton::Button7:
    return "Button7";
  case MouseButton::Button8:
    return "Button8";
  case MouseButton::Button9:
    return "Button9";
  case MouseButton::Button10:
    return "Button10";
  case MouseButton::Button11:
    return "Button11";
  case MouseButton::Button12:
    return "Button12";
  case MouseButton::Button13:
    return "Button13";
  case MouseButton::Button14:
    return "Button14";
  case MouseButton::Button15:
    return "Button15";
  default:
    return "Unknown";
  }
}

/**
 * @brief Get short name (for UI display)
 */
NODISCARD constexpr std::string_view
ToShortString(MouseButton button) noexcept {
  switch (button) {
  case MouseButton::Left:
    return "LMB";
  case MouseButton::Right:
    return "RMB";
  case MouseButton::Middle:
    return "MMB";
  case MouseButton::Button3:
    return "MB3";
  case MouseButton::Button4:
    return "MB4";
  case MouseButton::Button5:
    return "MB5";
  case MouseButton::Button6:
    return "MB6";
  case MouseButton::Button7:
    return "MB7";
  default: {
    if (IsValidButton(button)) {
      // Return "MB8", "MB9", etc.
      // Note: In constexpr context, we can't use dynamic strings
      return "MBX";
    }
    return "???";
  }
  }
}

} // namespace BeEngine
