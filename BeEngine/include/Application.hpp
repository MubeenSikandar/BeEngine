// include/Application.hpp
#pragma once

#include "Core.hpp"
#include "Events/Event.hpp"
#include "Events/EventQueue.hpp"

namespace BeEngine {

class BE_API Application {
public:
  Application();
  virtual ~Application();

  // Main application loop
  void Run();

  // Event handling (can be overridden by derived classes)
  virtual void OnEvent(Event &event);

  // Get the event queue
  EventQueue &GetEventQueue() { return m_EventQueue; }

  // Prevent copying
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

protected:
  bool m_Running = true;
  EventQueue m_EventQueue;

private:
  void ProcessEvents();
  void TestEvents();
};

// To be defined by client application
Application *CreateApplication();

} // namespace BeEngine
