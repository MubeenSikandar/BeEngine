// ApplicationEvent.hpp
#pragma once

#include <Core.hpp>
#include <Events/Event.hpp>
#include <cstdint>
#include <format>
#include <string>

namespace BeEngine {
/**
 * @brief Event fired when window dimensions change
 * @note Includes both user resizing and programmatic changes
 */

class BE_API WindowResizeEvent : public Event {
public:
  struct WindowSize {
    uint32_t Width;
    uint32_t Height;
  };

  WindowResizeEvent(const WindowSize &size) noexcept
      : m_Width(size.Width), m_Height(size.Height) {}

  NODISCARD uint32_t GetWidth() const noexcept { return m_Width; }
  NODISCARD uint32_t GetHeight() const noexcept { return m_Height; }

  NODISCARD std::string ToString() const override {
    return std::format("WindowResizeEvent: {}x{}", m_Height, m_Width);
  }

  EVENT_CLASS_TYPE(WindowResize)
  EVENT_CLASS_CATEGORY(
      static_cast<int>(EventCategory::EventCategoryApplication) |
      static_cast<int>(EventCategory::EventCategoryWindow))

private:
  uint32_t m_Width;
  uint32_t m_Height;
};

/**
 * @brief Event fired when window close is requested
 * @note Does not guarantee window will close - can be cancelled by handlers
 */

class BE_API WindowCloseEvent : public Event {
public:
  WindowCloseEvent() noexcept = default;

  EVENT_CLASS_TYPE(WindowClose)
  EVENT_CLASS_CATEGORY(
      static_cast<int>(EventCategory::EventCategoryApplication) |
      static_cast<int>(EventCategory::EventCategoryWindow))
};

/**
 * @brief Event fired once per frame for fixed-timestep logic
 */

class BE_API AppTickEvent : public Event {
public:
  AppTickEvent() noexcept = default;

  EVENT_CLASS_TYPE(AppTick)
  EVENT_CLASS_CATEGORY(
      static_cast<int>(EventCategory::EventCategoryApplication))
};

/**
 * @brief Event fired for frame update logic (variable timestep)
 */

class BE_API AppUpdateEvent : public Event {
public:
  explicit AppUpdateEvent(float deltaTime) noexcept : m_DeltaTime(deltaTime) {}

  NODISCARD float GetDeltaTime() const noexcept { return m_DeltaTime; }

  NODISCARD std::string ToString() const override {
    return std::format("AppUpdateEvent: dt={:.3f}ms", m_DeltaTime * 1000.0f);
  }

  EVENT_CLASS_TYPE(AppUpdate)
  EVENT_CLASS_CATEGORY(
      static_cast<int>(EventCategory::EventCategoryApplication))

private:
  float m_DeltaTime;
};

/**
 * @brief Event fired when rendering should occur
 */

class BE_API AppRenderEvent : public Event {
public:
  AppRenderEvent() noexcept = default;

  EVENT_CLASS_TYPE(AppRender)
  EVENT_CLASS_CATEGORY(
      static_cast<int>(EventCategory::EventCategoryApplication))
};
} // namespace BeEngine
