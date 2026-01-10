// BeEngine/src/Renderer/Renderer.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

RendererAPI::~RendererAPI() = default;

// Static member initialization
Scope<RendererAPI> Renderer::s_RendererAPI = nullptr;
RenderAPI RendererAPI::s_CurrentAPI = RenderAPI::OpenGL;

// ===== RendererAPI Factory =====

Scope<RendererAPI> RendererAPI::Create() {
  switch (s_CurrentAPI) {
  case RenderAPI::None:
    BE_CORE_CRITICAL("RendererAPI::None is not supported!");
    return nullptr;

  case RenderAPI::OpenGL:
    return CreateScope<OpenGLRendererAPI>();

  case RenderAPI::Vulkan:
    BE_CORE_CRITICAL("Vulkan renderer not implemented yet!");
    return nullptr;

  default:
    BE_CORE_CRITICAL("Unknown RendererAPI!");
    return nullptr;
  }
}

// ===== Renderer Implementation =====

void Renderer::Init() {
  BE_CORE_INFO("Initializing Renderer with API: {}",
               static_cast<int>(RendererAPI::GetAPI()));

  s_RendererAPI = RendererAPI::Create();

  if (!s_RendererAPI) {
    BE_CORE_CRITICAL("Failed to create RendererAPI!");
    return;
  }

  s_RendererAPI->Init();

  BE_CORE_INFO("Renderer initialized successfully!");
}

void Renderer::Shutdown() {
  BE_CORE_INFO("Shutting down Renderer...");

  if (s_RendererAPI) {
    s_RendererAPI->Shutdown();
    s_RendererAPI.reset();
  }
}

void Renderer::BeginFrame() { s_RendererAPI->BeginFrame(); }

void Renderer::EndFrame() { s_RendererAPI->EndFrame(); }

void Renderer::Clear(float r, float g, float b, float a) {
  s_RendererAPI->Clear(r, g, b, a);
}

void Renderer::SetClearColor(const glm::vec4 &color) {
  s_RendererAPI->SetClearColor(color);
}

void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width,
                           uint32_t height) {
  s_RendererAPI->SetViewport(x, y, width, height);
}

void Renderer::OnWindowResize(uint32_t width, uint32_t height) {
  SetViewport(0, 0, width, height);
}

void Renderer::DrawIndexed(uint32_t indexCount, uint32_t startIndex) {
  s_RendererAPI->DrawIndexed(indexCount, startIndex);
}

void Renderer::Draw(uint32_t vertexCount, uint32_t startVertex) {
  s_RendererAPI->Draw(vertexCount, startVertex);
}

void Renderer::EnableDepthTest(bool enable) {
  s_RendererAPI->EnableDepthTest(enable);
}

void Renderer::EnableBlending(bool enable) {
  s_RendererAPI->EnableBlending(enable);
}

void Renderer::EnableCulling(bool enable) {
  s_RendererAPI->EnableCulling(enable);
}

void Renderer::SetWireframeMode(bool enable) {
  s_RendererAPI->SetWireframeMode(enable);
}

} // namespace BeEngine
