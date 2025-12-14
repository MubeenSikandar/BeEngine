#pragma once

#include "Core.hpp"
#include "Layers/Layer.hpp"
#include <memory>
#include <vector>
namespace BeEngine {
/**
 * @brief Manages a stack of application layers
 *
 * The LayerStack manages the lifecycle and execution order of layers.
 *
 * Architecture:
 * - Layers: Normal application layers (bottom of stack)
 * - Overlays: Always on top (UI, debug tools, etc.)
 *
 * Execution Order:
 * - OnUpdate: Forward (layers first, then overlays)
 * - OnEvent: Reverse (overlays first, then layers)
 *
 * This ensures overlays (like UI) can intercept events before
 * they reach gameplay layers.
 *
 * Visual representation:
 *
 *   [Overlay 2]  <- Top (receives events first)
 *   [Overlay 1]
 *   ------------ <- Layer insert index
 *   [Layer 2]
 *   [Layer 1]    <- Bottom (updates first)
 *
 */
class BE_API LayerStack {
public:
  /**
   * @brief Construct an empty layer stack
   */
  LayerStack() = default;

  /**
   * @brief Destructor - detaches and destroys all layers
   */
  ~LayerStack();

  // Non-copyable (layers have unique ownership)
  LayerStack(const LayerStack &) = delete;
  LayerStack &operator=(const LayerStack &) = delete;

  // Movable
  LayerStack(LayerStack &&) noexcept = delete;
  LayerStack &operator=(LayerStack &&) noexcept = delete;

  // ===== Layer Management =====
  /**
   * @brief Push a layer onto the stack (before overlays)
   * @param layer Shared pointer to the layer
   *
   * Layers are inserted before overlays and receive events after overlays.
   * OnAttach() is called immediately.
   */
  void PushLayer(std::shared_ptr<Layer> layer);

  /**
   * @brief Push an overlay onto the stack (always on top)
   * @param overlay Shared pointer to the overlay
   *
   * Overlays are always at the top of the stack and receive events first.
   * OnAttach() is called immediately.
   */
  void PushOverlay(std::shared_ptr<Layer> overlay);

  /**
   * @brief Remove a layer from the stack
   * @param layer The layer to remove
   *
   * OnDetach() is called immediately.
   * The layer is destroyed when the last shared_ptr reference is released.
   */
  void PopLayer(const std::shared_ptr<Layer> &layer);

  /**
   * @brief Remove an overlay from the stack
   * @param overlay The overlay to remove
   *
   * OnDetach() is called immediately.
   */
  void PopOverlay(const std::shared_ptr<Layer> &overlay);

  /**
   * @brief Remove all layers and overlays
   *
   * OnDetach() is called for each layer in reverse order.
   */
  void Clear();

  // ===== Query Methods =====

  /**
   * @brief Find a layer by name
   * @param name The debug name of the layer
   * @return Shared pointer to the layer, or nullptr if not found
   */
  NODISCARD std::shared_ptr<Layer> GetLayer(const std::string &name) const;

  /**
   * @brief Check if a layer exists in the stack
   */
  NODISCARD bool HasLayer(const std::string &name) const;

  /**
   * @brief Get the number of layers (excluding overlays)
   */
  NODISCARD size_t GetLayerCount() const { return m_LayerInsertIndex; }

  /**
   * @brief Get the number of overlays
   */
  NODISCARD size_t GetOverlayCount() const {
    return m_Layers.size() - m_LayerInsertIndex;
  }

  /**
   * @brief Get total number of layers and overlays
   */
  NODISCARD size_t GetTotalCount() const { return m_Layers.size(); }

  /**
   * @brief Check is the stack is empty
   */
  NODISCARD bool isEmpty() const { return m_Layers.empty(); }

  // ===== Iteration Support =====

  // Forward iteration (for updates)
  using iterator = std::vector<std::shared_ptr<Layer>>::iterator;
  using const_iterator = std::vector<std::shared_ptr<Layer>>::const_iterator;

  iterator begin() { return m_Layers.begin(); }
  iterator end() { return m_Layers.end(); }
  NODISCARD const_iterator begin() const { return m_Layers.begin(); }
  NODISCARD const_iterator end() const { return m_Layers.end(); }
  NODISCARD const_iterator cbegin() const { return m_Layers.cbegin(); }
  NODISCARD const_iterator cend() const { return m_Layers.cend(); }

  // Reverse iteration (for events)
  using reverse_iterator =
      std::vector<std::shared_ptr<Layer>>::reverse_iterator;
  using const_reverse_iterator =
      std::vector<std::shared_ptr<Layer>>::const_reverse_iterator;

  reverse_iterator rbegin() { return m_Layers.rbegin(); }
  reverse_iterator rend() { return m_Layers.rend(); }

  NODISCARD const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
  NODISCARD const_reverse_iterator rend() const { return m_Layers.rend(); }

  NODISCARD const_reverse_iterator crbegin() const {
    return m_Layers.crbegin();
  }
  NODISCARD const_reverse_iterator crend() const { return m_Layers.crend(); }

  // ===== Debug & Profiling =====
  void PrintDebugInfo() const;

private:
  /**
   * @brief Internal storage for layers
   *
   * Layout: [Layer1, Layer2, ..., Overlay1, Overlay2, ...]
   *                              ^
   *                         m_LayerInsertIndex
   */
  std::vector<std::shared_ptr<Layer>> m_Layers;

  /**
   * @brief Index where new layers are inserted (before overlays)
   *
   * All layers before this index are normal layers.
   * All layers at or after this index are overlays.
   */
  size_t m_LayerInsertIndex{};
};
} // namespace BeEngine
