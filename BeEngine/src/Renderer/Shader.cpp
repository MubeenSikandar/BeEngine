#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

Ref<Shader> Shader::Create(const std::string &vertexSrc,
                           const std::string &fragmentSrc) {
  switch (RendererAPI::GetAPI()) {
  case RenderAPI::OpenGL:
    BE_CORE_TRACE("Creating OpenGL Shader");
    return CreateRef<OpenGLShader>(vertexSrc, fragmentSrc);

  case RenderAPI::Vulkan:
    BE_CORE_CRITICAL("Vulkan is not yet supported!");
    BE_CORE_ASSERT(false, "Vulkan Shader not implemented!");
    return nullptr;

  case RenderAPI::DirectX11:
    BE_CORE_CRITICAL("DirectX 11 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 11 Shader not implemented!");
    return nullptr;

  case RenderAPI::DirectX12:
    BE_CORE_CRITICAL("DirectX 12 is not yet supported!");
    BE_CORE_ASSERT(false, "DirectX 12 Shader not implemented!");
    return nullptr;

  case RenderAPI::Metal:
    BE_CORE_CRITICAL("Metal is not yet supported!");
    BE_CORE_ASSERT(false, "Metal Shader not implemented!");
    return nullptr;

  case RenderAPI::None:
  default:
    BE_CORE_CRITICAL("RenderAPI::None is not a valid rendering API!");
    BE_CORE_ASSERT(false, "No rendering API selected!");
    return nullptr;
  }
}

} // namespace BeEngine
