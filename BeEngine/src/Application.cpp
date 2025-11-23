// src/Application.cpp
#include "Application.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "Logs/Log.hpp"
#include <chrono>
#include <thread>

namespace BeEngine {

// Initialize m_EventQueue in member initializer list
Application::Application()
    : m_EventQueue(Config{.maxQueue = 1000,
                          .dropOnOverflow = true,
                          .enableLogging = true,
                          .enableProfiling =
                              true}) // Construct EventQueue here, not in body!
{
  BE_CORE_INFO("Application constructor called");

  // Subscribe to window close events
  m_EventQueue.Subscribe(
      EventType::WindowClose,
      [this](Event &e) {
        BE_CORE_INFO("Window close event received");
        m_Running = false;
        return true; // Event handled
      },
      10 // High priority
  );
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

  uint64_t frameNumber = 0;

  TestEvents();

  while (m_Running) {
    frameNumber++;

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

    // Example: Exit after 300 frames for testing
    if (frameNumber >= 300) {
      BE_CORE_INFO("Test complete, exiting...");
      m_EventQueue.QueueEvent<WindowCloseEvent>();
    }
  }

  BE_CORE_INFO("Application loop ended");
}

void Application::TestEvents() {
  BE_CORE_INFO("Testing event system...");

  // Test window resize event
  m_EventQueue.QueueEvent<WindowResizeEvent>(
      WindowResizeEvent::WindowSize{1920, 1080});

  // Test key events
  m_EventQueue.QueueEvent<KeyPressedEvent>(KeyCode::A, false);
  m_EventQueue.QueueEvent<KeyPressedEvent>(KeyCode::Escape, false);
  m_EventQueue.QueueEvent<KeyReleasedEvent>(KeyCode::A);

  // Test mouse events
  m_EventQueue.QueueEvent<MouseMovedEvent>(100.0F, 200.0F, 5.0F, 10.0F);
  m_EventQueue.QueueEvent<MouseButtonPressedEvent>(MouseButton::Left);
  m_EventQueue.QueueEvent<MouseScrolledEvent>(0.0F, 1.0F);

  // Process these test events immediately
  m_EventQueue.ProcessEvents();

  BE_CORE_INFO("Event system test complete");
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
