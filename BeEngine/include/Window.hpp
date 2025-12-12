#pragma once

#include "Core.hpp"
#include "Events/Event.hpp"
#include <GLFW/glfw3.h>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

// Forward declare GLFW types to avoid including GLFW in header
struct GLFWwindow {};

namespace BeEngine {

struct Width {
  uint32_t value;
};

struct Height {
  uint32_t value;
};

struct WindowProps {
  std::string title;
  Width width;
  Height height;
  bool vSync;

  WindowProps(std::string title = "BeEngine", Width width = {1280},
              Height height = {720}, bool vSync = true)
      : title(std::move(title)), width(width), height(height), vSync(vSync) {}
};

/**
 * @brief Cross-platform window abstraction using GLFW
 * @note Currently supports Windows, macOS, and Linux via GLFW
 */

class BE_API Window {
public:
  using EventCallbackFn = std::function<void(Event &)>;

  /**
   * @brief Create a window with specified properties
   * @param props Window creation properties
   * @return Unique pointer to window instance
   */

  static std::unique_ptr<Window>
  Create(const WindowProps &props = WindowProps());

  Window(const WindowProps &props);
  ~Window();

  // Non-Copyable
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  /**
   * @brief Poll events and swap buffers (call once per frame)
   */
  void OnUpdate();

  /**
   * @brief Poll events and swap buffers (call once per frame)
   */

  NODISCARD bool shouldClose() const;

  // Getters
  NODISCARD uint32_t GetWidth() const { return m_Data.width; }
  NODISCARD uint32_t GetHeight() const { return m_Data.height; }
  NODISCARD std::string GetTitle() const { return m_Data.title; }
  NODISCARD bool IsVSynce() const { return m_Data.vSync; }

  /**
   * @brief Get native GLFW window handle
   * @note Use this to interface with GLFW directly if needed
   */

  NODISCARD void *GetNativeWindow() const { return m_Window; }

  /**
   * @brief Set callback for window events
   * @param callback Function to call when events occur
   */

  void SetEventCallback(const EventCallbackFn &callback) {
    m_Data.eventCallback = callback;
  }

  /**
   * @brief Enable/disable vertical sync
   */

  void SetVSync(bool enabled);

  /**
   * @brief Set window title
   */

  void SetTitle(const std::string &title);

private:
  void Init(const WindowProps &props);
  void ShutDown();
  void SetupCallbacks();

  GLFWwindow *m_Window;

  struct WindowData {
    std::string title;
    uint32_t width;
    uint32_t height;
    bool vSync;
    EventCallbackFn eventCallback;
  };

  WindowData m_Data;
};
} // namespace BeEngine
