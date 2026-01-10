// MouseEvent.hpp
#pragma once

#include <Codes/MouseCodes.hpp>
#include <Core.hpp>
#include <Events/Event.hpp>
#include <format>
#include <string>

namespace BeEngine {

// ============================================================================
// Mouse Moved Event
// ============================================================================

/**
 * @brief Event fired when the mouse cursor moves
 * @note Coordinates are in screen space (pixels from top-left corner)
 * @note Delta values represent movement since the last frame
 */
class BE_API MouseMovedEvent : public Event {
public:
  /**
   * @param x Absolute X position in pixels (0 = left edge)
   * @param y Absolute Y position in pixels (0 = top edge)
   * @param deltaX Horizontal movement since last event (positive = right)
   * @param deltaY Vertical movement since last event (positive = down)
   */

  MouseMovedEvent(float X, float Y, float deltaX, float deltaY) noexcept
      : m_MouseX(X), m_MouseY(Y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}

  // Absolute position getters
  NODISCARD float GetX() const noexcept { return m_MouseX; }
  NODISCARD float GetY() const noexcept { return m_MouseY; }

  // Delta (movement) getters
  NODISCARD float GetDeltaX() const noexcept { return m_DeltaX; }
  NODISCARD float GetDeltaY() const noexcept { return m_DeltaY; }

  NODISCARD std::string ToString() const override {
    return std::format(
        "MouseMovedEvent: ({:.1f}, {:.1f}) delta: ({:.1f}, {:.1f})", m_MouseX,
        m_MouseY, m_DeltaX, m_DeltaY);
  }

  EVENT_CLASS_TYPE(MouseMoved)
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryMouse) |
                       static_cast<int>(EventCategory::EventCategoryInput))

private:
  float m_MouseX, m_MouseY; // Absolute position
  float m_DeltaX, m_DeltaY; // Movement delta
};

// ============================================================================
// Mouse Scrolled Event
// ============================================================================

/**
 * @brief Event fired when the mouse wheel is scrolled
 * @note Most mice only support vertical scrolling (yOffset)
 * @note Positive yOffset = scroll up, negative = scroll down
 * @note Horizontal scrolling (xOffset) is supported on some mice/trackpads
 */
class BE_API MouseScrolledEvent : public Event {
public:
  /**
   * @param xOffset Horizontal scroll amount (positive = right)
   * @param yOffset Vertical scroll amount (positive = up/away from user)
   */
  MouseScrolledEvent(float xOffset, float yOffset) noexcept
      : m_XOffset(xOffset), m_YOffset(yOffset) {}

  NODISCARD float GetXOffset() const noexcept { return m_XOffset; }
  NODISCARD float GetYOffset() const noexcept { return m_YOffset; }

  NODISCARD std::string ToString() const override {
    return std::format("MouseScrolledEvent: ({:.2f}, {:.2f})", m_XOffset,
                       m_YOffset);
  }

  EVENT_CLASS_TYPE(MouseScrolled)
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryMouse) |
                       static_cast<int>(EventCategory::EventCategoryInput))

private:
  float m_XOffset;
  float m_YOffset;
};

// ============================================================================
// Mouse Button Event (Base Class)
// ============================================================================

/**
 * @brief Base class for mouse button events
 */
class BE_API MouseButtonEvent : public Event {
public:
  NODISCARD MouseButton GetMouseButton() const noexcept { return m_Button; }

  EVENT_CLASS_CATEGORY(
      static_cast<int>(EventCategory::EventCategoryMouse) |
      static_cast<int>(EventCategory::EventCategoryInput) |
      static_cast<int>(EventCategory::EventCategoryMouseButton))

protected:
  explicit MouseButtonEvent(MouseButton button) noexcept : m_Button(button) {}

  MouseButton m_Button;
};

// ============================================================================
// Mouse Button Pressed Event
// ============================================================================

/**
 * @brief Event fired when a mouse button is pressed down
 */
class BE_API MouseButtonPressedEvent : public MouseButtonEvent {
public:
  explicit MouseButtonPressedEvent(MouseButton button) noexcept
      : MouseButtonEvent(button) {}

  NODISCARD std::string ToString() const override {
    return std::format("MouseButtonPressedEvent: {} ({})",
                       BeEngine::ToString(m_Button),
                       static_cast<int>(m_Button));
  }

  EVENT_CLASS_TYPE(MouseButtonPressed)
};

// ============================================================================
// Mouse Button Released Event
// ============================================================================

/**
 * @brief Event fired when a mouse button is released
 */
class BE_API MouseButtonReleasedEvent : public MouseButtonEvent {
public:
  explicit MouseButtonReleasedEvent(MouseButton button) noexcept
      : MouseButtonEvent(button) {}

  NODISCARD std::string ToString() const override {
    return std::format("MouseButtonReleasedEvent: {} ({})",
                       BeEngine::ToString(m_Button),
                       static_cast<int>(m_Button));
  }

  EVENT_CLASS_TYPE(MouseButtonReleased)
};

// ============================================================================
// Mouse Enter Event
// ============================================================================

/**
 * @brief Event fired when the mouse cursor enters the window
 */
class BE_API MouseEnterEvent : public Event {
public:
  MouseEnterEvent() noexcept = default;

  NODISCARD std::string ToString() const override { return "MouseEnterEvent"; }

  EVENT_CLASS_TYPE(MouseEnter)
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryMouse) |
                       static_cast<int>(EventCategory::EventCategoryInput))
};

// ============================================================================
// Mouse Leave Event
// ============================================================================

/**
 * @brief Event fired when the mouse cursor leaves the window
 */
class BE_API MouseLeaveEvent : public Event {
public:
  MouseLeaveEvent() noexcept = default;

  NODISCARD std::string ToString() const override { return "MouseLeaveEvent"; }

  EVENT_CLASS_TYPE(MouseLeave)
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryMouse) |
                       static_cast<int>(EventCategory::EventCategoryInput))
};

} // namespace BeEngine
