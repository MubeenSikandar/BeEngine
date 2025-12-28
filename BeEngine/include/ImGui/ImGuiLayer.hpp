#pragma once

#include "Core.hpp"
#include "Layers/Layer.hpp"

namespace BeEngine {
class BE_API ImGuiLayer : public Layer {
public:
  ImGuiLayer();
  ~ImGuiLayer() override = default;

  void OnAttach() override;
  void OnDetach() override;
  void OnEvent(Event &e) override;

  /**
   * @brief Begin ImGui frame
   *
   * Called at the start of frame, before any ImGui rendering.
   */
  void Begin();

  /**
   * @brief End ImGui frame
   *
   * Called at the end of frame, after all ImGui rendering.
   * Renders ImGui draw data.
   */
  void End();

  /**
   * @brief Block ImGui events
   *
   * When true, ImGui won't capture input events.
   */
  void SetBlockEvent(bool block) { m_BlockEvents = block; }

  /**
   * @brief Check if ImGui wants to capture mouse
   */
  NODISCARD bool WantCaptureMouse() const;

  /**
   * @brief Check if ImGui wants to capture keyboard
   */
  NODISCARD bool WantCaptureKeyboard() const;

  /**
   * @brief Setup dockspace for the main window
   *
   * Creates a fullscreen dockspace that fills the entire window.
   * Call this in your OnImGuiRender() implementation.
   */
  void BeginDockspace();

  /**
   * @brief End dockspace
   */
  void EndDockspace();

  /**
   * @brief Enable/disable dockspace
   */
  void SetDockspaceEnabled(bool enabled) { m_DockspaceEnabled = enabled; }

  /**
   * @brief Check if dockspace is enabled
   */
  NODISCARD bool IsDockspaceEnabled() const { return m_DockspaceEnabled; }

private:
  bool m_BlockEvents{true};
  bool m_DockspaceEnabled{true};
  float m_Time{0.0F};
};
} // namespace BeEngine
