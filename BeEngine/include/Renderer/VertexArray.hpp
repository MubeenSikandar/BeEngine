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

  virtual void AddVertexBuffer(std::shared_ptr<VertexBuffer> vertexBuffer) = 0;
  virtual void SetIndexBuffer(std::shared_ptr<IndexBuffer> indexBuffer) = 0;

  NODISCARD virtual const std::shared_ptr<IndexBuffer> &
  GetIndexBuffer() const = 0;

  static std::shared_ptr<VertexArray> Create();
};
} // namespace BeEngine
