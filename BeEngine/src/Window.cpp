#include "Window.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "GLFW/glfw3.h"
#include "KeyCodes.hpp"
#include "Logs/Log.hpp"
#include <cstdint>
#include <memory>
#include <sys/types.h>

namespace BeEngine {

// Track number of windows for GLFW initialization/termination
static uint8_t s_GLFWWindowCount{0};

// GLFW Error Callback
static void GLFWErrorCallback(int error, const char *description) {
  BE_CORE_ERROR("GLFW Error ({}): {}", error, description);
}

// ============================================================================
// Static Factory Method
// ============================================================================

std::unique_ptr<Window> Window::Create(const WindowProps &props) {
  return std::make_unique<Window>(props);
}

// ============================================================================
// Constructor & Destructor
// ============================================================================

Window::Window(const WindowProps &props) { Init(props); }

Window::~Window() { ShutDown(); }

// ============================================================================
// Initialization
// ============================================================================

void Window::Init(const WindowProps &props) {
  m_Data.title = props.title;
  m_Data.width = props.width.value;
  m_Data.height = props.height.value;
  m_Data.vSync = props.vSync;

  BE_CORE_INFO("Creating Window '{} ({} x {})'", props.title,
               props.height.value, props.width.value);

  // Initialize GLFW (only once for first window)
  if (s_GLFWWindowCount == 0) {
    int success = glfwInit();
    if (success == GLFW_FALSE) {
      BE_CORE_CRITICAL("Failed to Initialize GLFW!");
      return;
    }
  }

  glfwSetErrorCallback(GLFWErrorCallback);
  BE_CORE_INFO("GLFW initialized successfully!");

  // Create window
  m_Window = glfwCreateWindow(static_cast<int>(props.width.value),
                              static_cast<int>(props.height.value),
                              m_Data.title.c_str(), nullptr, nullptr);

  if (m_Window == nullptr) {
    BE_CORE_CRITICAL("Failed to Create a GLFW window!");
    glfwTerminate();
    return;
  }

  ++s_GLFWWindowCount;

  // Make OpenGL context current (needed for OpenGL calls)
  glfwMakeContextCurrent(m_Window);

  // Store our WindowData in GLFW's user pointer (for callbacks)
  glfwSetWindowUserPointer(m_Window, &m_Data);

  // Enable VSync
  SetVSync(m_Data.vSync);

  // setup all event callbacks
  SetupCallbacks();

  BE_CORE_INFO("Window Created Successfully");
}

// ============================================================================
// Shutdown
// ============================================================================

void Window::ShutDown() {
  if (m_Window != nullptr) {
    glfwDestroyWindow(m_Window);
    --s_GLFWWindowCount;

    BE_CORE_INFO("Window Destroyed!");

    if (s_GLFWWindowCount == 0) {
      glfwTerminate();
      BE_CORE_INFO("GLFW Terminated");
    }
  }
}

// ============================================================================
// Event Callbacks
// ============================================================================

void Window::SetupCallbacks() {

  // Window Resize
  glfwSetWindowSizeCallback(
      m_Window, [](GLFWwindow *window, int width, int height) {
        Width w{static_cast<uint32_t>(width)};
        Height h{static_cast<uint32_t>(height)};
        WindowData &data =
            *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.width = w.value;
        data.height = h.value;

        WindowResizeEvent event(WindowResizeEvent::WindowSize{
            .Width = data.height, .Height = data.height});
        if (data.eventCallback) {
          data.eventCallback(event);
        }
      });

  // Window Close
  glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
    WindowData &data =
        *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
    WindowCloseEvent event;
    data.eventCallback(event);
  });

  // Keyboard input
  glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int /*scancode*/,
                                  int action, int /*mods*/) {
    WindowData &data =
        *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
    switch (action) {
    case GLFW_PRESS: {
      KeyPressedEvent event(static_cast<KeyCode>(key), false);
      data.eventCallback(event);
      break;
    }
    case GLFW_RELEASE: {
      KeyReleasedEvent event(static_cast<KeyCode>(key));
      data.eventCallback(event);
      break;
    }
    case GLFW_REPEAT: {
      KeyPressedEvent event(static_cast<KeyCode>(key), true);
      data.eventCallback(event);
      break;
    }
    default:
      break;
    }
  });

  // Character Input (for text input)
  glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int codepoint) {
    WindowData &data =
        *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

    KeyTypedEvent event(static_cast<KeyCode>(codepoint));
    data.eventCallback(event);
  });

  // Mouse Button
  glfwSetMouseButtonCallback(
      m_Window, [](GLFWwindow *window, int button, int action, int /*mods*/) {
        WindowData &data =
            *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

        switch (action) {
        case GLFW_PRESS: {
          MouseButtonPressedEvent event(static_cast<MouseButton>(button));
          data.eventCallback(event);
          break;
        }
        case GLFW_RELEASE: {
          MouseButtonReleasedEvent event(static_cast<MouseButton>(button));
          data.eventCallback(event);
          break;
        }
        default:
          break;
        }
      });

  // Mouse Scroll
  glfwSetCursorPosCallback(
      m_Window, [](GLFWwindow *window, double xPos, double yPos) {
        WindowData &data =
            *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

        // Calculate delta (if you want to track it)
        static double lastX = xPos;
        static double lastY = yPos;
        auto deltaX = static_cast<float>(xPos - lastX);
        auto deltaY = static_cast<float>(yPos - lastY);
        lastX = xPos;
        lastY = yPos;

        MouseMovedEvent event(static_cast<float>(xPos),
                              static_cast<float>(yPos), deltaX, deltaY);

        if (data.eventCallback) {
          data.eventCallback(event);
        }
      });

  // Cursor Enter/Leave Window
  glfwSetCursorEnterCallback(m_Window, [](GLFWwindow *window, int entered) {
    WindowData &data =
        *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

    if (entered) {
      MouseEnterEvent event;
      data.eventCallback(event);
    } else {
      MouseLeaveEvent event;
      data.eventCallback(event);
    }
  });
}

// ============================================================================
// Public Methodss
// ============================================================================

void Window::OnUpdate() {
  // Poll for events (keyboard, mouse, Window events)
  glfwPollEvents();

  // swap front and back buffers (for rendering)
  glfwSwapBuffers(m_Window);
}

bool Window::shouldClose() const {
  return static_cast<bool>(glfwWindowShouldClose(m_Window));
}

void Window::SetVSync(bool enabled) {
  // 1 = enable VSync, 0 = disable
  glfwSwapInterval(enabled ? 1 : 0);
  m_Data.vSync = enabled;

  BE_CORE_TRACE("VSync: {}", enabled ? "Enabled" : "Disabled");
}

void Window::SetTitle(const std::string &title) {
  m_Data.title = title;
  glfwSetWindowTitle(m_Window, title.c_str());
}

} // namespace BeEngine
