// KeyEvent.hpp
#pragma once

#include "Core.hpp"
#include "Events/Event.hpp"
#include "KeyCodes.hpp"
#include <format> // C++20, or use <string> for older standards
#include <string>

namespace BeEngine {

// ============================================================================
// Base Key Event
// ============================================================================

/**
 * @brief Base class for all keyboard-related events
 * @note Provides common key code access for derived keyboard events
 */
class BE_API KeyEvent : public Event {
public:
  ~KeyEvent() override = default;

  /**
   * @brief Get the key code associated with this event
   * @return The key that triggered this event
   */
  NODISCARD KeyCode GetKeyCode() const noexcept { return m_KeyCode; }

  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryKeyboard) |
                       static_cast<int>(EventCategory::EventCategoryInput))

protected:
  explicit KeyEvent(KeyCode keyCode) noexcept : m_KeyCode(keyCode) {}

  KeyCode m_KeyCode;
};

// ============================================================================
// Key Pressed Event
// ============================================================================

/**
 * @brief Event fired when a key is pressed down
 * @note This is a raw keyboard event that fires on physical key press
 * @note For text input, use KeyTypedEvent instead (handles IME, dead keys,
 * etc.)
 */
class BE_API KeyPressedEvent : public KeyEvent {
public:
  /**
   * @param keycode The key that was pressed
   * @param isRepeat True if this is a repeat event (key held down)
   */
  explicit KeyPressedEvent(KeyCode keycode, bool isRepeat = false) noexcept
      : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

  /**
   * @brief Check if this is a key repeat event
   * @return True if the key is being held down (auto-repeat)
   */
  NODISCARD bool IsRepeat() const noexcept { return m_IsRepeat; }

  NODISCARD std::string ToString() const override {
    return std::format("KeyPressedEvent: {} ({}) (repeat = {})",
                       BeEngine::ToString(m_KeyCode),
                       static_cast<int>(m_KeyCode), m_IsRepeat);
  }

  EVENT_CLASS_TYPE(KeyPressed)

private:
  bool m_IsRepeat;
};

// ============================================================================
// Key Released Event
// ============================================================================

/**
 * @brief Event fired when a key is released
 * @note Guaranteed to fire after KeyPressedEvent for the same key
 */
class BE_API KeyReleasedEvent : public KeyEvent {
public:
  explicit KeyReleasedEvent(KeyCode keyCode) noexcept : KeyEvent(keyCode) {}

  NODISCARD std::string ToString() const override {
    return std::format("KeyReleasedEvent: {} ({})",
                       BeEngine::ToString(m_KeyCode),
                       static_cast<int>(m_KeyCode));
  }

  EVENT_CLASS_TYPE(KeyReleased)
};

// ============================================================================
// Key Typed Event
// ============================================================================

/**
 * @brief Event fired for character input (after OS text processing)
 * @note Use this for text input fields, console input, etc.
 * @note Handles:
 *       - Dead keys (´ + e = é)
 *       - Input Method Editors (IME) for Asian languages
 *       - Modifier combinations (Shift+A = 'A')
 * @note Does NOT fire for non-printable keys (Ctrl, Alt, F1, etc.)
 *
 * @example
 * // User presses Shift+A:
 * // 1. KeyPressedEvent(LeftShift)
 * // 2. KeyPressedEvent(A)
 * // 3. KeyTypedEvent(A)  ← Use this for text input
 * // 4. KeyReleasedEvent(A)
 * // 5. KeyReleasedEvent(LeftShift)
 */
class BE_API KeyTypedEvent : public KeyEvent {
public:
  explicit KeyTypedEvent(KeyCode keyCode) noexcept : KeyEvent(keyCode) {}

  /**
   * @brief Get the character representation of this typed key
   * @return The character, or '\0' if not printable
   */
  NODISCARD char GetCharacter() const noexcept { return ToChar(m_KeyCode); }

  NODISCARD std::string ToString() const override {
    char ch = GetCharacter();
    if (ch != '\0') {
      return std::format("KeyTypedEvent: '{}' ({})", ch,
                         static_cast<int>(m_KeyCode));
    }
    return std::format("KeyTypedEvent: {} ({})", BeEngine::ToString(m_KeyCode),
                       static_cast<int>(m_KeyCode));
  }

  EVENT_CLASS_TYPE(KeyTyped)
};

} // namespace BeEngine
