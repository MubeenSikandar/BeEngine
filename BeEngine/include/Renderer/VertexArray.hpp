#pragma once
#include "Core.hpp"
#include "Renderer/IndexBuffer.hpp"
#include "Renderer/VertexBuffer.hpp"
#include <memory>

namespace BeEngine {

/**
 * @brief Vertex array object - describes vertex data layout
 */
class BE_API VertexArray {
public:
  virtual ~VertexArray() = default;

  virtual void Bind() const = 0;
  virtual void Unbind() const = 0;

  virtual void AddVertexBuffer(const Ref<VertexBuffer> &vertexBuffer) = 0;
  virtual void SetIndexBuffer(const Ref<IndexBuffer> &indexBuffer) = 0;

  NODISCARD virtual const Ref<IndexBuffer> &GetIndexBuffer() const = 0;

  static Ref<VertexArray> Create();
};
} // namespace BeEngine
