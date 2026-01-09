#pragma once
#include "Core.hpp"
#include <cstdint>
#include <memory>

namespace BeEngine {

/**
 * @brief Index buffer - stores indices for indexed rendering
 */
class BE_API IndexBuffer {
public:
  virtual ~IndexBuffer() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;
  NODISCARD virtual uint32_t GetCount() const = 0;

  /**
   * @brief Create an index buffer
   * @param indices Pointer to index data
   * @param count Number of indices
   */
  static Ref<IndexBuffer> Create(const uint32_t *indices, uint32_t count);
};
} // namespace BeEngine
