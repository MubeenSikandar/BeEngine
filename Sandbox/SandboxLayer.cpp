#include "SandboxLayer.hpp"
#include <array>

void SandboxLayer::SetupTriangle() {
  BE_INFO("Setting up triangle geometry...");

  std::array<float, 9> vertices = {-0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
                                   0.0f,  0.0f,  0.5f, 0.0f};

  m_VertexArray = BeEngine::VertexArray::Create();
  auto vb = BeEngine::VertexBuffer::Create(
      vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));
  m_VertexArray->AddVertexBuffer(vb);

  std::string vertexSrc = R"(
    #version 410 core
    layout(location = 0) in vec3 a_Position;
    void main() {
      gl_Position = vec4(a_Position, 1.0);
    }
  )";

  std::string fragmentSrc = R"(
      #version 410 core
      layout(location = 0) out vec4 o_Color;

      void main() {
          o_Color = vec4(1.0, 0.5, 0.2, 1.0);  // Orange
      }
  )";

  m_Shader = BeEngine::Shader::Create(vertexSrc, fragmentSrc);
  BE_INFO("Triangle setup complete!");
}
