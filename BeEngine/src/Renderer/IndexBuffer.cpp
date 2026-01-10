#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

Ref<IndexBuffer> IndexBuffer::Create(const uint32_t *indices, uint32_t count) {
  switch (RendererAPI::GetAPI()) {
  case RenderAPI::OpenGL:
    BE_CORE_TRACE("Creating OpenGL IndexBuffer");
    return CreateRef<OpenGLIndexBuffer>(indices, count);

  case RenderAPI::Vulkan:
    BE_CORE_CRITICAL("Vulkan is not yet supported!");
    BE_CORE_ASSERT(false, "Vulkan IndexBuffer not implemented!");
    return nullptr;

  case RenderAPI::DirectX11:
    BE_CORE_CRITICAL("DirectX 11 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 11 IndexBuffer not implemented!");
    return nullptr;

  case RenderAPI::DirectX12:
    BE_CORE_CRITICAL("DirectX 12 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 12 IndexBuffer not implemented!");
    return nullptr;

  case RenderAPI::Metal:
    BE_CORE_CRITICAL("Metal is not yet supported!");
    BE_CORE_ASSERT(false, "Metal IndexBuffer not implemented!");
    return nullptr;

  case RenderAPI::None:
  default:
    BE_CORE_CRITICAL("RenderAPI::None is not a valid rendering API!");
    BE_CORE_ASSERT(false, "No rendering API selected!");
    return nullptr;
  }
}

} // namespace BeEngine
