// Sandbox/SandboxApp.cpp
#include "../BeEngine/include/Application.hpp"
#include "../BeEngine/include/EntryPoint.hpp"
#include "../BeEngine/include/Events/ApplicationEvent.hpp"
#include "../BeEngine/include/Events/KeyEvent.hpp"
#include "../BeEngine/include/Events/MouseEvent.hpp"
#include "../BeEngine/include/KeyCodes.hpp"
#include "../BeEngine/include/Logs/Log.hpp"
#include "../BeEngine/include/MouseCodes.hpp"

class Sandbox : public BeEngine::Application {
public:
  Sandbox() {
    BE_INFO("╔════════════════════════════════════════╗");
    BE_INFO("║     BeEngine OpenGL Sandbox App       ║");
    BE_INFO("╚════════════════════════════════════════╝");

    // Push your layers

    BE_INFO("Sandbox Application created!");
    BE_INFO("Controls:");
    BE_INFO("  SPACE - Change triangle color");
    BE_INFO("  ESC   - Exit application");

    SetupEventListeners();
  }

  ~Sandbox() override {
    BE_INFO("===========================================");
    BE_INFO("   Sandbox Application Shutting Down");
    BE_INFO("===========================================");
  }

  void OnEvent(BeEngine::Event &event) override {
    BeEngine::EventDispatcher dispatcher(event);

    // Only specify event type, function type is auto-deduced!
    dispatcher.Dispatch<BeEngine::WindowResizeEvent>(
        [this](BeEngine::WindowResizeEvent &e) { return OnWindowResize(e); });

    dispatcher.Dispatch<BeEngine::KeyPressedEvent>(
        [this](BeEngine::KeyPressedEvent &e) { return OnKeyPressed(e); });
  }

private:
  void SetupEventListeners() {
    BE_INFO("Setting up event listeners...");

    // Tick events
    GetEventQueue().Subscribe(BeEngine::EventType::AppTick,
                              [this](BeEngine::Event &e) {
                                m_TickCount++;
                                return false;
                              });

    // Update events
    GetEventQueue().Subscribe(
        BeEngine::EventType::AppUpdate, [this](BeEngine::Event &e) {
          auto &updateEvent = static_cast<BeEngine::AppUpdateEvent &>(e);
          m_AccumulatedTime += updateEvent.GetDeltaTime();

          // Log FPS Evey Second
          if (m_AccumulatedTime >= 1.0f) {
            BE_INFO("FPS: ~{:.1f}", m_TickCount / m_AccumulatedTime);
            m_AccumulatedTime = 0.0F;
            m_TickCount = 0;
          }

          m_TotalTime += updateEvent.GetDeltaTime();
          if (!m_HasRequestedClose && m_TotalTime >= 5.0f) {
            BE_INFO("Test duration complete (5s), closing...");
            GetEventQueue().QueueEvent<BeEngine::WindowCloseEvent>();
            m_HasRequestedClose = true;
          }

          return false;
        });

    // Keyboard events
    GetEventQueue().Subscribe(
        BeEngine::EventType::KeyPressed, [](BeEngine::Event &e) {
          auto &keyEvent = static_cast<BeEngine::KeyPressedEvent &>(e);
          BE_INFO("Key: {} {}", BeEngine::ToString(keyEvent.GetKeyCode()),
                  keyEvent.IsRepeat() ? "[REPEAT]" : "");
          return false;
        });

    // Mouse events
    GetEventQueue().Subscribe(
        BeEngine::EventType::MouseButtonPressed, [](BeEngine::Event &e) {
          auto &mouseEvent =
              static_cast<BeEngine::MouseButtonPressedEvent &>(e);
          BE_INFO("Mouse: {}", BeEngine::ToString(mouseEvent.GetMouseButton()));
          return false;
        });

    BE_INFO("Event listeners setup complete!");
  }

  bool OnWindowResize(BeEngine::WindowResizeEvent &e) {
    BE_TRACE("Window: {}x{}", e.GetWidth(), e.GetHeight());
    return false;
  }

  bool OnKeyPressed(BeEngine::KeyPressedEvent &e) {
    if (e.GetKeyCode() == BeEngine::KeyCode::Escape) {
      BE_WARN("Escape pressed - shutting down");
      GetEventQueue().QueueEvent<BeEngine::WindowCloseEvent>();
      return true;
    }
    return false;
  }

  uint64_t m_TickCount = 0;
  float m_AccumulatedTime = 0.0F;
  float m_TotalTime = 0.0F;
  bool m_HasRequestedClose{false};
};

BeEngine::Application *BeEngine::CreateApplication() { return new Sandbox(); }
