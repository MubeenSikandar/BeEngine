// Renderer/BufferLayout.hpp
#pragma once

#include "Core.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace BeEngine {

/**
 * @brief Shader data types for vertex attributes
 */
enum class ShaderDataType : uint8_t {
  None = 0,
  Float,
  Float2,
  Float3,
  Float4,
  Int,
  Int2,
  Int3,
  Int4,
  Mat3,
  Mat4,
  Bool
};

/**
 * @brief Get size in bytes of a ShaderDataType
 */
inline uint32_t ShaderDataTypeSize(ShaderDataType type) {
  switch (type) {
  case ShaderDataType::Float:
    return 4;
  case ShaderDataType::Float2:
    return 4 * 2;
  case ShaderDataType::Float3:
    return 4 * 3;
  case ShaderDataType::Float4:
    return 4 * 4;
  case ShaderDataType::Int:
    return 4;
  case ShaderDataType::Int2:
    return 4 * 2;
  case ShaderDataType::Int3:
    return 4 * 3;
  case ShaderDataType::Int4:
    return 4 * 4;
  case ShaderDataType::Mat3:
    return 4 * 3 * 3;
  case ShaderDataType::Mat4:
    return 4 * 4 * 4;
  case ShaderDataType::Bool:
    return 1;
  case ShaderDataType::None:
    return 0;
  }
  return 0;
}

/**
 * @brief A single element in a buffer layout (one vertex attribute)
 */
struct BufferElement {
  std::string Name;
  ShaderDataType Type{ShaderDataType::None};
  uint32_t Size{0};
  uint32_t Offset{0};
  bool Normalized{false};

  BufferElement() = default;

  BufferElement(ShaderDataType type, std::string name, bool normalized = false)
      : Name(std::move(name)), Type(type), Size(ShaderDataTypeSize(type)),
        Offset(0), Normalized(normalized) {}

  /**
   * @brief Get component count (e.g., Float3 = 3 components)
   */
  NODISCARD uint32_t GetComponentCount() const {
    switch (Type) {
    case ShaderDataType::Float:
      return 1;
    case ShaderDataType::Float2:
      return 2;
    case ShaderDataType::Float3:
      return 3;
    case ShaderDataType::Float4:
      return 4;
    case ShaderDataType::Int:
      return 1;
    case ShaderDataType::Int2:
      return 2;
    case ShaderDataType::Int3:
      return 3;
    case ShaderDataType::Int4:
      return 4;
    case ShaderDataType::Mat3:
      return 3 * 3;
    case ShaderDataType::Mat4:
      return 4 * 4;
    case ShaderDataType::Bool:
      return 1;
    case ShaderDataType::None:
      return 0;
    }
    return 0;
  }
};

/**
 * @brief Describes the layout of vertex data in a buffer
 *
 * Example:
 *   BufferLayout layout = {
 *       { ShaderDataType::Float3, "a_Position" },
 *       { ShaderDataType::Float3, "a_Color" },
 *       { ShaderDataType::Float2, "a_TexCoord" }
 *   };
 */
class BufferLayout {
public:
  BufferLayout() = default;

  BufferLayout(std::initializer_list<BufferElement> elements)
      : m_Elements(elements) {
    CalculateOffsetsAndStride();
  }

  NODISCARD const std::vector<BufferElement> &GetElements() const {
    return m_Elements;
  }
  NODISCARD uint32_t GetStride() const { return m_Stride; }

  // Iterators for range-based for loops
  std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
  std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
  NODISCARD std::vector<BufferElement>::const_iterator begin() const {
    return m_Elements.begin();
  }
  NODISCARD std::vector<BufferElement>::const_iterator end() const {
    return m_Elements.end();
  }

private:
  void CalculateOffsetsAndStride() {
    uint32_t offset = 0;
    m_Stride = 0;
    for (auto &element : m_Elements) {
      element.Offset = offset;
      offset += element.Size;
      m_Stride += element.Size;
    }
  }

  std::vector<BufferElement> m_Elements;
  uint32_t m_Stride = 0;
};

} // namespace BeEngine
