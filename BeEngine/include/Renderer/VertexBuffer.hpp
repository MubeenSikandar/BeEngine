#pragma once
#include "Core.hpp"
#include "Renderer/BufferLayout.hpp"
#include <cstdint>
#include <memory>

namespace BeEngine {
/**
 * @brief Abstract vertex buffer interface
 * @note API-agnostic - works with OpenGL, Vulkan, DirectX, etc.
 */

class BE_API VertexBuffer {
public:
  virtual ~VertexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  /**
   * @brief Set the layout describing vertex attributes
   */
  virtual void SetLayout(const BufferLayout &layout) = 0;

  /**
   * @brief Get the current buffer layout
   */
  NODISCARD virtual const BufferLayout &GetLayout() const = 0;

  /**
   * @brief Factory method - creates buffer for current API
   * @param vertices Pointer to vertex data
   * @param size Size in bytes
   */
  static Ref<VertexBuffer> Create(const void *vertices, uint32_t size);
};
} // namespace BeEngine
