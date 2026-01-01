#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

std::shared_ptr<VertexArray> VertexArray::Create() {
  switch (RendererAPI::GetAPI()) {
  case RenderAPI::OpenGL:
    BE_CORE_TRACE("Creating OpenGL VertexArray");
    return std::make_shared<OpenGLVertexArray>();

  case RenderAPI::Vulkan:
    BE_CORE_CRITICAL("Vulkan is not yet supported!");
    BE_CORE_ASSERT(false, "Vulkan VertexArray not implemented!");
    return nullptr;

  case RenderAPI::DirectX11:
    BE_CORE_CRITICAL("DirectX 11 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 11 VertexArray not implemented!");
    return nullptr;

  case RenderAPI::DirectX12:
    BE_CORE_CRITICAL("DirectX 12 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 12 VertexArray not implemented!");
    return nullptr;

  case RenderAPI::Metal:
    BE_CORE_CRITICAL("Metal is not yet supported!");
    BE_CORE_ASSERT(false, "Metal VertexArray not implemented!");
    return nullptr;

  case RenderAPI::None:
  default:
    BE_CORE_CRITICAL("RenderAPI::None is not a valid rendering API!");
    BE_CORE_ASSERT(false, "No rendering API selected!");
    return nullptr;
  }
}

} // namespace BeEngine
