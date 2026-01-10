// Event.hpp
#pragma once

#include <Core.hpp>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <ostream>
#include <string>

namespace BeEngine {
// EVENT TYPES

enum class EventType : uint16_t {
  None = 0,
  // Window events
  WindowClose,
  WindowResize,
  WindowFocus,
  WindowLostFocus,
  WindowMoved,
  WindowMinimized,
  WindowMaximized,
  WindowRestored,

  // Application events
  AppTick,
  AppUpdate,
  AppRender,
  AppQuit,

  // Input events - Keyboard
  KeyPressed,
  KeyReleased,
  KeyTyped,

  // Input events - Mouse
  MouseButtonPressed,
  MouseButtonReleased,
  MouseMoved,
  MouseScrolled,
  MouseEnter,
  MouseLeave,

  // Input events - Gamepad
  GamepadConnected,
  GamepadDisconnected,
  GamepadButtonPressed,
  GamepadButtonReleased,
  GamepadAxisMoved,

  // Touch events (for mobile)
  TouchBegan,
  TouchMoved,
  TouchEnded,
  TouchCancelled,

  // File events
  FileDropped,

  // Network events (for multiplayer)
  NetworkConnected,
  NetworkDisconnected,
  NetworkData,

  // Audio events
  AudioDeviceAdded,
  AudioDeviceRemoved,

  // Custom events (user-defined)
  Custom
};

// EVENT CATEGORIES
enum class EventCategory : uint32_t {
  None = 0,
  EventCategoryApplication = BIT(0),
  EventCategoryInput = BIT(1),
  EventCategoryKeyboard = BIT(2),
  EventCategoryMouse = BIT(3),
  EventCategoryMouseButton = BIT(4),
  EventCategoryGamepad = BIT(5),
  EventCategoryTouch = BIT(6),
  EventCategoryWindow = BIT(7),
  EventCategoryNetwork = BIT(8),
  EventCategoryAudio = BIT(9),
  EventCategoryFile = BIT(10),
  EventCategoryCustom = BIT(11)
};

// EVENT PRIORITY
enum class EventPriority : uint8_t {
  Low = 0,
  Normal = 1,
  High = 2,
  Critical = 3
};

// EVENT BASE CLASS
class BE_API Event {
public:
  virtual ~Event() = default;

  // Core Event Information
  NODISCARD virtual EventType GetEventType() const = 0;
  NODISCARD virtual const char *GetName() const = 0;
  NODISCARD virtual int GetCategoryFlags() const = 0;
  NODISCARD virtual std::string ToString() const { return GetName(); }

  // Category Checking
  NODISCARD bool IsInCategory(EventCategory category) const {
    return (GetCategoryFlags() & static_cast<uint32_t>(category)) != 0;
  }

  // Event Handling
  bool IsHandled = false;

  // Event Priority
  NODISCARD EventPriority GetPriority() const { return m_Priority; };
  void SetPriority(EventPriority priority) { m_Priority = priority; }

  // Timestamp (When Event was Created)
  NODISCARD std::chrono::steady_clock::time_point GetTimestamp() const {
    return m_Timestamp;
  }

  // Frame Number (for debugging)
  NODISCARD uint64_t GetFrameNumber() const { return m_FrameNumber; }
  void setFrameNumber(uint64_t frame) { m_FrameNumber = frame; }

  // Event ID (unique per event instance)
  NODISCARD uint64_t GetEventID() const { return m_EventID; }

protected:
  Event()
      : m_Priority(EventPriority::Normal),
        m_Timestamp(std::chrono::steady_clock::now()),
        m_EventID(s_NextEventID.fetch_add(1, std::memory_order_relaxed)) {}

private:
  EventPriority m_Priority{};
  std::chrono::steady_clock::time_point m_Timestamp;
  uint64_t m_EventID;
  uint64_t m_FrameNumber{0};
  static std::atomic<uint64_t> s_NextEventID;
};

// EVENT MACROS
#define EVENT_CLASS_TYPE(type)                                                 \
  static EventType GetStaticType() { return EventType::type; }                 \
  virtual EventType GetEventType() const override { return GetStaticType(); }  \
  virtual const char *GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category)                                         \
  virtual int GetCategoryFlags() const override { return category; }

// EVENT DISPATCHER (Immediate Mode)
class BE_API EventDispatcher {
public:
  explicit EventDispatcher(Event &event) : m_Event(event) {}

  // Template that only requires event type
  template <typename T, typename F> bool Dispatch(F &&func) {
    if (m_Event.GetEventType() == T::GetStaticType()) {
      m_Event.IsHandled |= func(static_cast<T &>(m_Event));
      return true;
    }
    return false;
  }

private:
  Event &m_Event;
};

// EVENT CALLBACK
using EventCallbackFn = std::function<void(Event &)>;

// STREAM OPERATOR
inline std::ostream &operator<<(std::ostream &os, const Event &e) {
  return os << e.ToString();
}

} // namespace BeEngine
