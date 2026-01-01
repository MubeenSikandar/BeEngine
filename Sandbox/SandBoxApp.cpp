// Sandbox/SandboxApp.cpp
#include "Application.hpp"
#include "EntryPoint.hpp"
#include "PCH/BeEnginePCH.hpp"
#include "SandboxLayer.hpp"

class Sandbox : public BeEngine::Application {
public:
  Sandbox() {
    BE_INFO("╔════════════════════════════════════════╗");
    BE_INFO("║     BeEngine OpenGL Sandbox App        ║");
    BE_INFO("╚════════════════════════════════════════╝");

    // Push your layers
    PushLayer(std::make_shared<SandboxLayer>());

    BE_INFO("Sandbox Application created!");

    SetupEventListeners();
  }

  ~Sandbox() override {
    BE_INFO("===========================================");
    BE_INFO("   Sandbox Application Shutting Down");
    BE_INFO("===========================================");
  }

  void OnImGuiRender() {
    // Example dockable windows

    // Statistics window
    ImGui::Begin("Statistics");
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("Frame Time: %.3f ms", 1000.0F / ImGui::GetIO().Framerate);
    ImGui::Text("Tick Count: %llu",
                static_cast<long long unsigned>(m_TickCount));
    ImGui::Text("Total Time: %.2f s", m_TotalTime);
    ImGui::End();

    // Controls window
    ImGui::Begin("Controls");
    ImGui::Text("Press ESC to close");
    ImGui::Text("Press Space to do something");
    if (ImGui::Button("Queue Close Event")) {
      GetEventQueue().QueueEvent<BeEngine::WindowCloseEvent>();
    }
    ImGui::End();

    // Debug window
    ImGui::Begin("Debug Info");
    ImGui::Text("Application: BeEngine Sandbox");
    ImGui::Text("Renderer: OpenGL");
    ImGui::Separator();
    ImGui::Checkbox("VSync", &m_VSync);
    ImGui::End();
  }

  void OnEvent(BeEngine::Event &event) override {
    BeEngine::EventDispatcher dispatcher(event);

    // Only specify event type, function type is auto-deduced!
    dispatcher.Dispatch<BeEngine::WindowResizeEvent>(
        [this](BeEngine::WindowResizeEvent &e) { return OnWindowResize(e); });

    dispatcher.Dispatch<BeEngine::KeyPressedEvent>(
        [this](BeEngine::KeyPressedEvent &e) { return OnKeyPressed(e); });
    dispatcher.Dispatch<BeEngine::GamepadButtonPressedEvent>(
        [this](BeEngine::GamepadButtonPressedEvent &e) {
          return OnGamepadButtonPressed(e);
        });
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
          if (m_AccumulatedTime >= 1.0F) {
            BE_INFO("FPS: ~{:.1f}", m_TickCount / m_AccumulatedTime);
            m_AccumulatedTime = 0.0F;
            m_TickCount = 0;
          }

          m_TotalTime += updateEvent.GetDeltaTime();
          if (!m_HasRequestedClose && m_TotalTime >= 5.0F) {
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

    // Gamepad connection events
    GetEventQueue().Subscribe(
        BeEngine::EventType::GamepadConnected, [](BeEngine::Event &e) {
          auto &gamepadEvent =
              static_cast<BeEngine::GamepadConnectedEvent &>(e);
          BE_INFO("Gamepad Connected: {} ({})",
                  BeEngine::ToString(gamepadEvent.GetGamepadID()),
                  gamepadEvent.GetGamepadName());
          return false;
        });

    GetEventQueue().Subscribe(
        BeEngine::EventType::GamepadDisconnected, [](BeEngine::Event &e) {
          auto &gamepadEvent =
              static_cast<BeEngine::GamepadDisconnectedEvent &>(e);
          BE_WARN("Gamepad Disconnected: {}",
                  BeEngine::ToString(gamepadEvent.GetGamepadID()));
          return false;
        });

    // Gamepad button events
    GetEventQueue().Subscribe(
        BeEngine::EventType::GamepadButtonPressed, [](BeEngine::Event &e) {
          auto &buttonEvent =
              static_cast<BeEngine::GamepadButtonPressedEvent &>(e);
          BE_INFO("Gamepad {} Button Pressed: {}",
                  BeEngine::ToString(buttonEvent.GetGamepadID()),
                  BeEngine::ToString(buttonEvent.GetButton()));
          return false;
        });

    GetEventQueue().Subscribe(
        BeEngine::EventType::GamepadButtonReleased, [](BeEngine::Event &e) {
          auto &buttonEvent =
              static_cast<BeEngine::GamepadButtonReleasedEvent &>(e);
          BE_INFO("Gamepad {} Button Released: {}",
                  BeEngine::ToString(buttonEvent.GetGamepadID()),
                  BeEngine::ToString(buttonEvent.GetButton()));
          return false;
        });

    // Gamepad axis events (with deadzone filtering to avoid spam)
    GetEventQueue().Subscribe(
        BeEngine::EventType::GamepadAxisMoved, [](BeEngine::Event &e) {
          auto &axisEvent = static_cast<BeEngine::GamepadAxisMovedEvent &>(e);
          // Only log significant movements to avoid console spam
          if (std::abs(axisEvent.GetValue()) > 0.2f) {
            BE_TRACE("Gamepad {} Axis {}: {:.3f}",
                     BeEngine::ToString(axisEvent.GetGamepadID()),
                     BeEngine::ToString(axisEvent.GetAxis()),
                     axisEvent.GetValue());
          }
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

  bool OnGamepadButtonPressed(BeEngine::GamepadButtonPressedEvent &e) {
    // Example: Start button closes the app (like Escape key)
    if (e.GetButton() == BeEngine::GamepadButton::Start) {
      BE_WARN("Start button pressed - shutting down");
      GetEventQueue().QueueEvent<BeEngine::WindowCloseEvent>();
      return true;
    }
    return false;
  }

  uint64_t m_TickCount = 0;
  float m_AccumulatedTime = 0.0F;
  float m_TotalTime = 0.0F;
  bool m_HasRequestedClose{false};
  bool m_VSync{true};
};

BeEngine::Application *BeEngine::CreateApplication() { return new Sandbox(); }
