// src/Application.cpp
#include "Application.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/Event.hpp"
#include "Logs/Log.hpp"
#include "Window.hpp"
#include <chrono>
#include <thread>

namespace BeEngine {

// Initialize m_EventQueue in member initializer list
Application::Application()
    : m_EventQueue(Config{.maxQueue = 1000,
                          .dropOnOverflow = true,
                          .enableLogging = false,
                          .enableProfiling =
                              true}) // Construct EventQueue here, not in body!
{
  BE_CORE_INFO("Application constructor called");

  // Create Window
  m_Window = Window::Create(WindowProps("BeEngine", Width{1280}, Height{720}));

  // Wire window events to event queue
  m_Window->SetEventCallback([this](Event &e) {
    EventDispatcher dispatcher(e);

    // Handle Window Close Event
    dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent &) {
      BE_CORE_INFO("Window Close Requested");
      m_Running = false;
      return true;
    });

    if (!e.IsHandled) {
      OnEvent(e);
    }
  });

  BE_CORE_INFO("Window created and event system initialized!");
}

Application::~Application() {
  BE_CORE_INFO("Application destructor called");

  // Print event statistics
  auto stats = m_EventQueue.GetStats();
  BE_CORE_INFO("Event Statistics:");
  BE_CORE_INFO("  Total Queued: {}", stats.totalEventsQueued);
  BE_CORE_INFO("  Total Processed: {}", stats.totalEventsProcessed);
  BE_CORE_INFO("  Total Dropped: {}", stats.totalEventsDropped);
  BE_CORE_INFO("  Max Queue Size: {}", stats.maxQueueSize);
  if (stats.totalEventsProcessed > 0) {
    BE_CORE_INFO("  Avg Process Time: {:.3f}ms", stats.averageProcessTimeMs);
  }
}

void Application::Run() {
  BE_CORE_INFO("Application started");

  while (m_Running && !m_Window->shouldClose()) {

    // ✅ Update window (poll events + swap buffers)
    m_Window->OnUpdate();
    // Process all pending events (max 5ms per frame)
    ProcessEvents();

    // Queue a tick event
    m_EventQueue.QueueEvent<AppTickEvent>();

    // Queue an update event with delta time
    // In a real engine, you'd calculate actual delta time
    float deltaTime = 0.016f; // ~60 FPS
    m_EventQueue.QueueEvent<AppUpdateEvent>(deltaTime);

    // Simulate some work (remove in real engine)
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }

  BE_CORE_INFO("Application loop ended");
}

void Application::ProcessEvents() {
  // Process events with a time budget of 5ms per frame
  m_EventQueue.ProcessEventsWithBudget(5.0);
}

void Application::OnEvent(Event &event) {
  BE_CORE_TRACE("Event received: {}", event.ToString());

  // Default implementation does nothing
  // Derived classes can override this
}

} // namespace BeEngine
