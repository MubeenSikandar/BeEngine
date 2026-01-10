#pragma once

#include "Events/Event.hpp"
#include <Codes/GamepadCodes.hpp>
#include <Core.hpp>
#include <format>
#include <string>
#include <sys/stat.h>

namespace BeEngine {
// ============================================================================
// GAMEPAD CONNECTION EVENTS
// ============================================================================

/**
 * @brief Event fired when a gamepad is connected
 */
class BE_API GamepadConnectedEvent : public Event {
public:
  explicit GamepadConnectedEvent(GamepadID id, std::string name = "Unknown")
      : m_GamepadID(id), m_GamepadName(std::move(name)) {}

  NODISCARD GamepadID GetGamepadID() const noexcept { return m_GamepadID; }
  NODISCARD const std::string &GetGamepadName() const noexcept {
    return m_GamepadName;
  }

  NODISCARD std::string ToString() const override {
    return std::format("GamepadConnectedEvent: {} ({})",
                       BeEngine::ToString(m_GamepadID), m_GamepadName);
  }

  EVENT_CLASS_TYPE(GamepadConnected);
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryGamepad) |
                       static_cast<int>(EventCategory::EventCategoryInput));

private:
  GamepadID m_GamepadID{};
  std::string m_GamepadName;
};

/**
 * @brief Event fired when a gamepad is disconnected
 */
class BE_API GamepadDisconnectedEvent : public Event {
public:
  explicit GamepadDisconnectedEvent(GamepadID id) : m_GamepadID(id) {}

  NODISCARD GamepadID GetGamepadID() const noexcept { return m_GamepadID; }

  NODISCARD std::string ToString() const override {
    return std::format("GamepadDisconnectedEvent: {} ",
                       BeEngine::ToString(m_GamepadID));
  }

  EVENT_CLASS_TYPE(GamepadDisconnected);
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryGamepad) |
                       static_cast<int>(EventCategory::EventCategoryInput));

private:
  GamepadID m_GamepadID;
};

// ============================================================================
// GAMEPAD BUTTON EVENTS
// ============================================================================

/**
 * @brief Base class for gamepad button events
 */
class BE_API GamepadButtonEvent : public Event {
public:
  ~GamepadButtonEvent() override = default;

  NODISCARD GamepadID GetGamepadID() const noexcept { return m_GamepadID; }
  NODISCARD GamepadButton GetButton() const noexcept { return m_Button; }

  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryGamepad) |
                       static_cast<int>(EventCategory::EventCategoryInput))

protected:
  GamepadButtonEvent(GamepadID id, GamepadButton button) noexcept
      : m_GamepadID(id), m_Button(button) {}

  GamepadID m_GamepadID;
  GamepadButton m_Button;
};

/**
 * @brief Event fired when a gamepad button is pressed
 */
class BE_API GamepadButtonPressedEvent : public GamepadButtonEvent {
public:
  GamepadButtonPressedEvent(GamepadID id, GamepadButton button)
      : GamepadButtonEvent(id, button) {}

  NODISCARD std::string ToString() const override {
    return std::format("GamepadButtonPressedEvent: {}, Button: {} ({})",
                       BeEngine::ToString(m_GamepadID),
                       BeEngine::ToString(m_Button),
                       static_cast<int>(m_Button));
  }

  EVENT_CLASS_TYPE(GamepadButtonPressed)
};

/**
 * @brief Event fired when a gamepad button is released
 */
class BE_API GamepadButtonReleasedEvent : public GamepadButtonEvent {
public:
  GamepadButtonReleasedEvent(GamepadID id, GamepadButton button) noexcept
      : GamepadButtonEvent(id, button) {}

  NODISCARD std::string ToString() const override {
    return std::format("GamepadButtonReleasedEvent: {}, Button: {} ({})",
                       BeEngine::ToString(m_GamepadID),
                       BeEngine::ToString(m_Button),
                       static_cast<int>(m_Button));
  }

  EVENT_CLASS_TYPE(GamepadButtonReleased)
};

// ============================================================================
// GAMEPAD AXIS EVENTS
// ============================================================================

/**
 * @brief Event fired when a gamepad axis value changes
 * @note Includes analog sticks (LeftX, LeftY, RightX, RightY) and triggers
 */
class BE_API GamepadAxisMovedEvent : public Event {
public:
  GamepadAxisMovedEvent(GamepadID id, GamepadAxis axis, float value) noexcept
      : m_GamepadID(id), m_Axis(axis), m_Value(value) {}

  NODISCARD GamepadID GetGamepadID() const noexcept { return m_GamepadID; }
  NODISCARD GamepadAxis GetAxis() const noexcept { return m_Axis; }
  NODISCARD float GetValue() const noexcept { return m_Value; }

  /**
   * @brief Check if this is a stick axis (not a trigger)
   */
  NODISCARD bool IsStickAxis() const noexcept {
    return BeEngine::IsStickAxis(m_Axis);
  }

  /**
   * @brief Check if this is a trigger axis
   */
  NODISCARD bool IsTriggerAxis() const noexcept {
    return BeEngine::IsTriggerAxis(m_Axis);
  }

  NODISCARD std::string ToString() const override {
    return std::format("GamepadAxisMovedEvent: {}, Axis: {}, Value: {:.3f}",
                       BeEngine::ToString(m_GamepadID),
                       BeEngine::ToString(m_Axis), m_Value);
  }

  EVENT_CLASS_TYPE(GamepadAxisMoved)
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryGamepad) |
                       static_cast<int>(EventCategory::EventCategoryInput))

private:
  GamepadID m_GamepadID;
  GamepadAxis m_Axis;
  float m_Value;
};

// ============================================================================
// GAMEPAD VIBRATION EVENT
// ============================================================================

/**
 * @brief Event for gamepad vibration/rumble feedback
 * @note This is an output event (engine → gamepad) rather than input
 * @note Useful for event-driven rumble systems
 */
class BE_API GamepadVibrationEvent : public Event {
public:
  GamepadVibrationEvent(GamepadID id,
                        const GamepadVibration &vibration) noexcept
      : m_GamepadID(id), m_Vibration(vibration) {}

  NODISCARD GamepadID GetGamepadID() const noexcept { return m_GamepadID; }
  NODISCARD const GamepadVibration &GetVibration() const noexcept {
    return m_Vibration;
  }

  NODISCARD std::string ToString() const override {
    return std::format("GamepadVibrationEvent: {}, Low: {:.2f}, High: {:.2f}, "
                       "Duration: {:.2f}s",
                       BeEngine::ToString(m_GamepadID),
                       m_Vibration.lowFrequency, m_Vibration.highFrequency,
                       m_Vibration.duration);
  }

  EVENT_CLASS_TYPE(Custom) // Custom event type for vibration
  EVENT_CLASS_CATEGORY(static_cast<int>(EventCategory::EventCategoryGamepad) |
                       static_cast<int>(EventCategory::EventCategoryCustom))

private:
  GamepadID m_GamepadID;
  GamepadVibration m_Vibration;
};

} // namespace BeEngine
