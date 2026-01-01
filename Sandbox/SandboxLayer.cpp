// SandboxLayer.cpp
#include "SandboxLayer.hpp"
#include <array>
#include <cstdint>

void SandboxLayer::SetupTriangle() {
  BE_INFO("Setting up triangle geometry...");

  // Triangle vertices (x, y, z)
  std::array<float, 9> vertices = {
      -0.5f, -0.5f, 0.0f, // Bottom left
      0.5f,  -0.5f, 0.0f, // Bottom right
      0.0f,  0.5f,  0.0f  // Top
  };

  // Indices
  std::array<uint32_t, 3> indices = {0, 2, 1};

  // Create vertex array
  BE_INFO("Creating VertexArray...");
  m_VertexArray = BeEngine::VertexArray::Create();

  if (!m_VertexArray) {
    BE_CORE_ERROR("Failed to create VertexArray!");
    return;
  }

  // Create vertex buffer
  BE_INFO("Creating VertexBuffer...");
  auto vb = BeEngine::VertexBuffer::Create(
      vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));

  if (!vb) {
    BE_CORE_ERROR("Failed to create VertexBuffer!");
    return;
  }

  m_VertexArray->AddVertexBuffer(vb);

  // Create index buffer
  BE_INFO("Creating IndexBuffer...");
  auto ib = BeEngine::IndexBuffer::Create(
      indices.data(), static_cast<uint32_t>(indices.size()));

  if (!ib) {
    BE_CORE_ERROR("Failed to create IndexBuffer!");
    return;
  }

  m_VertexArray->SetIndexBuffer(ib);

  // Create shader (use 410 for macOS compatibility)
  BE_INFO("Creating Shader...");
  std::string vertexSrc = R"(
    #version 410 core
    layout(location = 0) in vec3 a_Position;

    void main() {
      gl_Position = vec4(a_Position, 1.0);
    }
  )";

  std::string fragmentSrc = R"(
    #version 410 core
    layout(location = 0) out vec4 color;

    void main() {
      color = vec4(1.0, 0.5, 0.2, 1.0);  // Orange
    }
  )";

  m_Shader = BeEngine::Shader::Create(vertexSrc, fragmentSrc);

  if (!m_Shader) {
    BE_CORE_ERROR("Failed to create Shader!");
    return;
  }

  BE_INFO("Triangle setup complete!");
  BE_INFO("  Vertices: 3");
  BE_INFO("  Indices: 3");
  BE_INFO("  Shader: GLSL 410");
}
