#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

struct OpenGLCallback {
  unsigned source{};
  unsigned type{};
  unsigned id{};
  unsigned severity{};
  int length{};
  const char *message{};
  const void *userParam{};
};
// ===== OpenGL Debug Callback =====
static void OpenGLMessageCallback(unsigned source, unsigned type, unsigned id,
                                  unsigned severity, int length,
                                  const char *message, const void *userParam) {
  switch (severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    BE_CORE_CRITICAL("[OpenGL] {}", message);
    return;
  case GL_DEBUG_SEVERITY_MEDIUM:
    BE_CORE_ERROR("[OpenGL] {}", message);
    return;
  case GL_DEBUG_SEVERITY_LOW:
    BE_CORE_WARN("[OpenGL] {}", message);
    return;
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    BE_CORE_TRACE("[OpenGL] {}", message);
    return;
  }
}

// ===== Initialization =====
void OpenGLRendererAPI::Init() {
  BE_CORE_INFO("Initializing OpenGL Renderer");

#ifdef BE_DEBUG
  if (glDebugMessageCallback != nullptr) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(OpenGLMessageCallback, nullptr);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                          GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

    BE_CORE_INFO("OpenGL debug output enabled");
  } else {
    BE_CORE_WARN("OpenGL debug output not available (macOS OpenGL 4.1)");
  }
#endif

  // Enable blending by default
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Enable depth testing by default
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);

  // Enable face culling by default
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW); // Counter-clockwise winding

  BE_CORE_INFO("OpenGL Renderer initialized successfully!");
}

void OpenGLRendererAPI::Shutdown() {
  BE_CORE_INFO("Shutting down OpenGL Renderer...");
  // OpenGL cleanup is handled by context destruction
}

// ===== Frame Management =====

void OpenGLRendererAPI::BeginFrame() {
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::EndFrame() {
  // Nothing to do for OpenGL
  // Swap buffers is handled by the Window
}

// ===== Clear Operations =====

void OpenGLRendererAPI::Clear(float r, float g, float b, float a) {
  glClearColor(r, g, b, a);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRendererAPI::SetClearColor(const glm::vec4 &color) {
  m_ClearColor = color;
  glClearColor(color.r, color.g, color.b, color.a);
}

// ===== Viewport =====

void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width,
                                    uint32_t height) {
  glViewport(x, y, width, height);
}

// ===== Drawing =====

void OpenGLRendererAPI::DrawIndexed(uint32_t indexCount, uint32_t startIndex) {
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT,
                 (void *)(startIndex * sizeof(uint32_t)));
}

void OpenGLRendererAPI::Draw(uint32_t vertexCount, uint32_t startVertex) {
  glDrawArrays(GL_TRIANGLES, startVertex, vertexCount);
}

// ===== State Management =====

void OpenGLRendererAPI::EnableDepthTest(bool enable) {
  if (enable) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void OpenGLRendererAPI::EnableBlending(bool enable) {
  if (enable) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  } else {
    glDisable(GL_BLEND);
  }
}

void OpenGLRendererAPI::EnableCulling(bool enable) {
  if (enable) {
    glEnable(GL_CULL_FACE);
  } else {
    glDisable(GL_CULL_FACE);
  }
}

void OpenGLRendererAPI::SetWireframeMode(bool enable) {
  if (enable) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}

} // namespace BeEngine
