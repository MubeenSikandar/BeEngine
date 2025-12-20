#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {
LayerStack::~LayerStack() {
  BE_CORE_TRACE("Destroying Layer Stack with {} Layers", m_Layers.size());

  // Detach all layers in reverse order
  for (auto &layer : m_Layers | std::views::reverse) {
    if (layer) {
      BE_CORE_TRACE("Detaching Layers '{}'", layer->GetName());
      (layer)->OnDetach();
    }
  }

  m_Layers.clear();
}

void LayerStack::PushLayer(std::shared_ptr<Layer> layer) {
  if (!layer) {
    BE_CORE_ERROR("Attempted to push null layer!");
    return;
  }

  BE_CORE_INFO("Pushing layer '{}' at index {}", layer->GetName(),
               m_LayerInsertIndex);

  auto insertPos =
      m_Layers.begin() + static_cast<std::ptrdiff_t>(m_LayerInsertIndex);

  m_Layers.emplace(insertPos, std::move(layer));
  ++m_LayerInsertIndex;

  m_Layers[m_LayerInsertIndex - 1]->OnAttach();
}

void LayerStack::PushOverlay(std::shared_ptr<Layer> overlay) {
  if (!overlay) {
    BE_CORE_ERROR("Attempted to push null overlay!");
    return;
  }

  BE_CORE_INFO("Pushing overlay '{}' at end", overlay->GetName());

  m_Layers.emplace_back(std::move(overlay));
  m_Layers.back()->OnAttach();
}

void LayerStack::PopLayer(const std::shared_ptr<Layer> &layer) {
  if (!layer) {
    BE_CORE_WARN("Attempted to pop null layer");
    return;
  }

  // Find layer in the layers section (before insert index)
  auto it = std::find(m_Layers.begin(),
                      m_Layers.begin() + static_cast<long>(m_LayerInsertIndex),
                      layer);

  if (it != m_Layers.begin() + static_cast<long>(m_LayerInsertIndex)) {
    BE_CORE_INFO("Popping Layer '{}'", layer->GetName());

    // call on detach
    layer->OnDetach();

    m_Layers.erase(it);
    m_LayerInsertIndex--;
  } else {
    BE_CORE_WARN("Layer '{}' not found in layer stack!", layer->GetName());
  }
}

void LayerStack::PopOverlay(const std::shared_ptr<Layer> &overlay) {
  if (!overlay) {
    BE_CORE_WARN("Attempted to pull null overlay");
    return;
  }

  // Find layer in the overlay section (after insert index)
  auto it = std::find(m_Layers.begin() + static_cast<long>(m_LayerInsertIndex),
                      m_Layers.end(), overlay);

  if (it != m_Layers.end()) {
    BE_CORE_INFO("Popping overlay '{}'", overlay->GetName());

    // call on detach
    overlay->OnDetach();

    m_Layers.erase(it);
  } else {
    BE_CORE_WARN("Overlay '{}' not found in layer stack!", overlay->GetName());
  }
}

void LayerStack::Clear() {
  BE_CORE_INFO("Clearing all {} layers from stack", m_Layers.size());

  // Detach all layers in reverse order
  for (auto &layer : m_Layers | std::views::reverse) {
    if (layer) {
      BE_CORE_TRACE("Detaching layers '{}'", layer->GetName());
      layer->OnDetach();
    }
  }

  m_Layers.clear();
  m_LayerInsertIndex = 0;
}

std::shared_ptr<Layer> LayerStack::GetLayer(const std::string &name) const {
  auto it = std::ranges::find_if(m_Layers.begin(), m_Layers.end(),
                                 [&name](const std::shared_ptr<Layer> &layer) {
                                   return layer && layer->GetName() == name;
                                 });

  return (it != m_Layers.end()) ? *it : nullptr;
}

bool LayerStack::HasLayer(const std::string &name) const {
  return GetLayer(name) != nullptr;
}

void LayerStack::PrintDebugInfo() const {
  BE_CORE_INFO("=== LayerStack Debug Info ===");
  BE_CORE_INFO("Total Layers: {}", m_Layers.size());
  BE_CORE_INFO("Normal Layers: {}", m_LayerInsertIndex);
  BE_CORE_INFO("Overlays: {}", m_Layers.size() - m_LayerInsertIndex);
  BE_CORE_INFO("");

  // Print Layers
  if (m_LayerInsertIndex > 0) {
    BE_CORE_INFO("Layers: ");
    for (size_t i{}; i < m_LayerInsertIndex; i++) {
      if (m_Layers[i]) {
        BE_CORE_INFO(" [{}] {} ({})", i, m_Layers[i]->GetName(),
                     m_Layers[i]->IsEnabled() ? "Enabled" : "Disabled");
      }
    }
  }

  // Print overlays
  if (m_Layers.size() > m_LayerInsertIndex) {
    BE_CORE_INFO("Overlays:");
    for (size_t i{m_LayerInsertIndex}; i < m_Layers.size(); i++) {
      if (m_Layers[i]) {
        BE_CORE_INFO(" [{}] {} ({})", i, m_Layers[i]->GetName(),
                     m_Layers[i]->IsEnabled() ? "Enabled" : "Disabled");
      }
    }
  }
  BE_CORE_INFO("=============================");
}

} // namespace BeEngine
