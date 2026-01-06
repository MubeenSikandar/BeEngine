#include "SandboxLayer.hpp"
#include <array>

void SandboxLayer::SetupTriangle() {
  BE_INFO("Setting up triangle geometry...");

  std::array<float, 9> vertices = {
      -0.5F, -0.5F, 0.0F, // Bottom-left
      0.5F,  -0.5F, 0.0F, // Bottom-right
      0.0F,  0.5F,  0.0F  // Top-center
  };

  m_VertexArray = BeEngine::VertexArray::Create();

  auto vb = BeEngine::VertexBuffer::Create(
      vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));
  vb->SetLayout({{BeEngine::ShaderDataType::Float3, "a_Position"}});

  m_VertexArray->AddVertexBuffer(vb);

  std::string vertexSrc = R"(
      #version 410 core
      layout(location = 0) in vec3 a_Position;
      uniform mat4 u_ViewProjection;
      void main() {
          gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
      }
  )";

  std::string fragmentSrc = R"(
      #version 410 core
      layout(location = 0) out vec4 o_Color;
      void main() {
          o_Color = vec4(0.5, 0.9, 0.9, 1.0);
      }
  )";

  m_Shader = BeEngine::Shader::Create(vertexSrc, fragmentSrc);

  BE_INFO("Triangle setup complete!");
}
