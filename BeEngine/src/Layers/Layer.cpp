// src/Layers/Layer.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {
Layer::Layer(std::string name) : m_DebugName(std::move(name)) {
  BE_CORE_TRACE("Layer '{}' Created", m_DebugName);
}
} // namespace BeEngine
