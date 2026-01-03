#include "PCH/BeEnginePCH.hpp"

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

  BE_CORE_INFO("Creating Window '{} ({} x {})'", props.title, props.width.value,
               props.height.value);

  // Initialize GLFW (only once for first window)
  if (s_GLFWWindowCount == 0) {
    int success = glfwInit();
    BE_CORE_ASSERT(success, "Failed to initialize GLFW!");

    glfwSetErrorCallback(GLFWErrorCallback);
    BE_CORE_INFO("GLFW initialized successfully");
  }

  // Set OpenGL version BEFORE creating window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // macOS max
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#ifdef BE_DEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif

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

  // Create and initialize graphics context
  m_Context = GraphicsContext::Create(m_Window);
  m_Context->Init();

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
        WindowData &data =
            *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        data.width = static_cast<uint32_t>(width);
        data.height = static_cast<uint32_t>(height);

        WindowResizeEvent event(WindowResizeEvent::WindowSize{
            .Width = data.width, .Height = data.height});

        if (data.eventCallback) {
          data.eventCallback(event);
        }
      });

  // Window Close
  glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window) {
    WindowData &data =
        *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
    WindowCloseEvent event;
    if (data.eventCallback) {
      data.eventCallback(event);
    }
  });

  // Keyboard input
  glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int /*scancode*/,
                                  int action, int /*mods*/) {
    WindowData &data =
        *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
    switch (action) {
    case GLFW_PRESS: {
      KeyPressedEvent event(static_cast<KeyCode>(key), false);
      if (data.eventCallback) {
        data.eventCallback(event);
      }
      break;
    }
    case GLFW_RELEASE: {
      KeyReleasedEvent event(static_cast<KeyCode>(key));
      if (data.eventCallback) {
        data.eventCallback(event);
      }
      break;
    }
    case GLFW_REPEAT: {
      KeyPressedEvent event(static_cast<KeyCode>(key), true);
      if (data.eventCallback) {
        data.eventCallback(event);
      }
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
    if (data.eventCallback) {
      data.eventCallback(event);
    }
  });

  // Mouse Button
  glfwSetMouseButtonCallback(
      m_Window, [](GLFWwindow *window, int button, int action, int /*mods*/) {
        WindowData &data =
            *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

        switch (action) {
        case GLFW_PRESS: {
          MouseButtonPressedEvent event(static_cast<MouseButton>(button));
          if (data.eventCallback) {
            data.eventCallback(event);
          }
          break;
        }
        case GLFW_RELEASE: {
          MouseButtonReleasedEvent event(static_cast<MouseButton>(button));
          if (data.eventCallback) {
            data.eventCallback(event);
          }
          break;
        }
        default:
          break;
        }
      });

  // Mouse Scroll
  glfwSetScrollCallback(
      m_Window, [](GLFWwindow *window, double xOffset, double yOffset) {
        BE_CORE_INFO("GLFW Scroll: x={}, y={}", xOffset, yOffset);
        WindowData &data =
            *static_cast<WindowData *>(glfwGetWindowUserPointer(window));
        MouseScrolledEvent event(static_cast<float>(xOffset),
                                 static_cast<float>(yOffset));
        if (data.eventCallback) {
          data.eventCallback(event);
        }
      });

  // Mouse Movement
  glfwSetCursorPosCallback(
      m_Window, [](GLFWwindow *window, double xPos, double yPos) {
        WindowData &data =
            *static_cast<WindowData *>(glfwGetWindowUserPointer(window));

        float deltaX = static_cast<float>(xPos - data.lastMouseX);
        float deltaY = static_cast<float>(yPos - data.lastMouseY);
        data.lastMouseX = xPos;
        data.lastMouseY = yPos;

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
      if (data.eventCallback) {
        data.eventCallback(event);
      }
    } else {
      MouseLeaveEvent event;
      if (data.eventCallback) {
        data.eventCallback(event);
      }
    }
  });
}

// ============================================================================
// Public Methods
// ============================================================================

void Window::OnUpdate() {
  glfwPollEvents();
  m_Context->SwapBuffers();
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
