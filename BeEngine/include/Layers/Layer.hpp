#pragma once

#include <Core.hpp>
#include <Events/Event.hpp>
#include <Time/Timestep.hpp>
#include <string>

namespace BeEngine {
/**
 * @brief Base class for application layers
 *
 * A Layer represents a logical section of your application that can be
 * pushed onto the LayerStack. Layers are updated in order and receive
 * events in reverse order (overlays first).
 *
 * Common layer types:
 * - Game Layer: Main game logic
 * - UI Layer: User interface rendering
 * - Debug Layer: Debug overlays (FPS counter, console, etc.)
 * - Editor Layer: Scene editing tools
 *
 * Lifecycle:
 * 1. Construction
 * 2. OnAttach() - Called when pushed to stack
 * 3. OnUpdate(ts) - Called every frame (forward order)
 * 4. OnEvent(e) - Called when events occur (reverse order)
 * 5. OnImGuiRender() - Called for ImGui rendering (forward order)
 * 6. OnDetach() - Called when popped from stack
 * 7. Destruction
 *
 */

class BE_API Layer {
public:
  /**
   * @brief Construct a layer with a debug name
   * @param name Debug name for profiling and logging (default: "Layer")
   */
  explicit Layer(std::string name = "Layer");

  /**
   * @brief Virtual destructor for proper cleanup
   */
  virtual ~Layer() = default;

  // ===== Lifecycle Methods =====

  /**
   * @brief Called when layer is pushed to the stack
   *
   * Use this to initialize resources, load assets, set up systems, etc.
   * This is called AFTER construction but BEFORE first update.
   */
  virtual void OnAttach() {};

  /**
   * @brief Called when layer is popped from the stack
   *
   * Use this to clean up resources, save state, etc.
   * This is called BEFORE destruction.
   */
  virtual void OnDetach() {};

  // ===== Update Methods =====

  /**
   * @brief Called every frame to update layer logic
   * @param ts Time elapsed since last frame
   *
   * This is where your main game logic goes. Use the timestep
   * to make updates frame-rate independent.
   *
   * Example:
   *   position += velocity * ts.GetSeconds();
   */
  virtual void OnUpdate(Timestep ts) {};

  /**
   * @brief Called every frame for fixed-timestep physics
   * @param ts Fixed timestep (usually 1/60 = 0.0166s)
   *
   * Use this for physics updates that need a fixed timestep.
   * Called multiple times per frame if framerate is high.
   */
  virtual void OnFixedUpdate(float fixedDeltaTime) {}

  // ===== Rendering Methods =====

  /**
   * @brief Called every frame for ImGui rendering
   *
   * Use this to render debug UI, tools, editors, etc.
   * Only called if ImGui is enabled in the application.
   *
   * Example:
   *   ImGui::Begin("Debug");
   *   ImGui::Text("FPS: %.1f", ts.GetFPS());
   *   ImGui::End();
   */
  virtual void OnRender() {}

  /**
   * @brief Called every frame for ImGui rendering
   *
   * Use this to render debug UI, tools, editors, etc.
   * Only called if ImGui is enabled in the application.
   *
   * Example:
   *   ImGui::Begin("Debug");
   *   ImGui::Text("FPS: %.1f", ts.GetFPS());
   *   ImGui::End();
   */
  virtual void OnImGuiRender() {}

  // ===== Event Handling =====

  /**
   * @brief Called when an event is dispatched
   * @param event The event to handle
   *
   * Events propagate in REVERSE order (overlays receive first).
   * Set event.handled = true to prevent further propagation.
   *
   * Example:
   *   EventDispatcher dispatcher(event);
   *   dispatcher.Dispatch<KeyPressedEvent>([](KeyPressedEvent& e) {
   *       if (e.GetKeyCode() == Key::Escape) {
   *           // Handle escape key
   *           return true; // Event handled
   *       }
   *       return false;
   *   });
   */
  virtual void OnEvent(Event &event) {}

  // ===== Getters =====

  /**
   * @brief Get the debug name of this layer
   */
  NODISCARD const std::string &GetName() const { return m_DebugName; }

  /**
   * @brief Check if layer is enabled
   *
   * Disabled layers skip OnUpdate and OnEvent (but still render ImGui)
   */
  NODISCARD bool IsEnabled() const { return m_Enabled; }

  /**
   * @brief Enable or disable this layer
   */
  void SetEnabled(bool enabled) { m_Enabled = enabled; }

protected:
  std::string m_DebugName;
  bool m_Enabled{true};

private:
  float m_UpdateTime{};
  float m_EventTime{};

  friend class LayerStack; // Allow LayerStack to access profiling data
};
} // namespace BeEngine
