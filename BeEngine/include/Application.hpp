// include/Application.hpp
#pragma once

#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

/**
 * @brief Main application class for BeEngine
 *
 * The Application class is the core of your game/application.
 * It manages:
 * - Window creation and management
 * - Event system and dispatch
 * - Layer stack (game logic organization)
 * - Main game loop
 * - Frame timing
 *
 * Usage:
 * 1. Create a class that inherits from Application
 * 2. Override OnEvent() if needed
 * 3. Push your layers in the constructor
 * 4. Define CreateApplication() to return your app
 */

class BE_API Application {
public:
  Application();
  virtual ~Application();

  // Non-copyable, non-movable (singleton-like)
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;
  Application(Application &&) = delete;
  Application &operator=(Application &&) = delete;

  /**
   * @brief Main application loop
   *
   * Runs until the window is closed or m_Running is set to false.
   * Each frame:
   * 1. Calculates delta time
   * 2. Polls window events
   * 3. Processes event queue
   * 4. Updates all layers (if not minimized)
   * 5. Renders ImGui (if enabled)
   * 6. Swaps window buffers
   */
  void Run();

  /**
   * @brief Application-level event handler
   * @param event The event to handle
   *
   * Override this to handle application-wide events.
   * Layer events are handled automatically by the layer stack.
   */
  virtual void OnEvent(Event &event);

  // ===== Layer Management =====

  /**
   * @brief Push a layer onto the stack
   * @param layer Shared pointer to the layer
   *
   * Layers are updated in the order they're pushed.
   * Events are dispatched in reverse order (last pushed receives first).
   */
  void PushLayer(std::shared_ptr<Layer> layer);

  /**
   * @brief Push an overlay onto the stack (always on top)
   * @param overlay Shared pointer to the overlay
   *
   * Overlays are always at the top of the stack and receive events first.
   * Use for UI, debug tools, etc.
   */
  void PushOverlay(std::shared_ptr<Layer> overlay);

  /**
   * @brief Remove a layer from the stack
   */
  void PopLayer(const std::shared_ptr<Layer> &layer);

  /**
   * @brief Remove an overlay from the stack
   */
  void PopOverlay(const std::shared_ptr<Layer> &overlay);

  // ===== Getters =====

  /**
   * @brief Get the application window
   */
  NODISCARD Window &GetWindow() { return *m_Window; }

  /**
   * @brief Get the layer stack
   */
  NODISCARD LayerStack &GetLayerStack() { return m_LayerStack; }

  /**
   * @brief Get the event queue
   */
  EventQueue &GetEventQueue() { return m_EventQueue; }

  /**
   * @brief Get singleton instance
   */
  static Application &Get() { return *s_Instance; }

  /**
   * @brief Check if application is running
   */
  NODISCARD bool IsRunning() const { return m_Running; }

  /**
   * @brief Request application shutdown
   */
  void Close() { m_Running = false; }

  void SetBlockEvents(bool block) { m_BlockEvents = block; }
  bool IsBlockingEvents() const { return m_BlockEvents; }

private:
  // ===== Event Handlers =====
  bool OnWindowClose(WindowCloseEvent &e);
  bool OnWindowResize(WindowResizeEvent &e);

  // ===== Helper Methods =====

  /**
   * @brief Process events from the event queue
   */
  void ProcessEvents();

  /**
   * @brief Update all layers
   * @param ts Delta time since last frame
   */
  void UpdateLayers(Timestep ts);

  /**
   * @brief Updates all layers using a fixed timestep
   * @param fixedDeltaTime Fixed timestep duration
   */
  void FixedUpdateLayers(float fixedDeltaTime);

  /**
   * @brief Dispatch event to all layers (reverse order)
   */
  void DispatchEventToLayers(Event &event);

  // ===== Member Variables =====

  std::unique_ptr<Window> m_Window;
  LayerStack m_LayerStack;
  EventQueue m_EventQueue;
  std::shared_ptr<ImGuiLayer> m_ImGuiLayer;

  bool m_Running = true;
  bool m_Minimized = false;

  bool m_BlockEvents = true;

  static Application *s_Instance;
};

/**
 * @brief Entry point - defined by CLIENT
 *
 * You must define this function in your application code.
 * It should return a new instance of your Application subclass.
 */
Application *CreateApplication();
} // namespace BeEngine
