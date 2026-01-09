#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

Ref<VertexBuffer> VertexBuffer::Create(const void *vertices, uint32_t size) {
  switch (RendererAPI::GetAPI()) {
  case RenderAPI::OpenGL:
    BE_CORE_TRACE("Creating OpenGL VertexBuffer");
    return CreateRef<OpenGLVertexBuffer>(vertices, size);

  case RenderAPI::Vulkan:
    BE_CORE_CRITICAL("Vulkan is not yet supported!");
    BE_CORE_ASSERT(false, "Vulkan VertexBuffer not implemented!");
    return nullptr;

  case RenderAPI::DirectX11:
    BE_CORE_CRITICAL("DirectX 11 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 11 VertexBuffer not implemented!");
    return nullptr;

  case RenderAPI::DirectX12:
    BE_CORE_CRITICAL("DirectX 12 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 12 VertexBuffer not implemented!");
    return nullptr;

  case RenderAPI::Metal:
    BE_CORE_CRITICAL("Metal is not yet supported!");
    BE_CORE_ASSERT(false, "Metal VertexBuffer not implemented!");
    return nullptr;

  case RenderAPI::None:
  default:
    BE_CORE_CRITICAL("RenderAPI::None is not a valid rendering API!");
    BE_CORE_ASSERT(false, "No rendering API selected!");
    return nullptr;
  }
}
} // namespace BeEngine
