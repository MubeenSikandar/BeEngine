// Sandbox/src/TriangleLayer.hpp
#pragma once

#include "PCH/BeEnginePCH.hpp"

/**
 * @brief Simple layer that renders a colored triangle
 *
 * This is your first OpenGL test!
 * It demonstrates:
 * - Vertex Buffer Objects (VBO)
 * - Vertex Array Objects (VAO)
 * - Shaders (GLSL)
 * - Basic rendering
 */
class TriangleLayer : public BeEngine::Layer {
public:
  TriangleLayer() : Layer("Triangle") {}

  void OnAttach() override {
    BE_INFO("===========================================");
    BE_INFO("   TriangleLayer - OpenGL Test");
    BE_INFO("===========================================");

    CreateTriangle();
    CreateShader();

    BE_INFO("Triangle created successfully!");
    BE_INFO("Press SPACE to change color");
  }

  void OnDetach() override {
    // Cleanup OpenGL resources
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteProgram(m_ShaderProgram);

    BE_INFO("TriangleLayer detached");
  }

  void OnUpdate(BeEngine::Timestep ts) override {
    // Update triangle rotation
    m_Rotation += m_RotationSpeed * ts.GetSeconds(); // 45 degrees per second
    if (m_Rotation > 360.0f) {
      m_Rotation -= 360.0f;
    }
  }

  void OnRender() override {
    // Use our shader
    glUseProgram(m_ShaderProgram);

    // Send rotation to shader
    int rotationLoc = glGetUniformLocation(m_ShaderProgram, "u_Rotation");
    glUniform1f(rotationLoc, glm::radians(m_Rotation));

    // Send color to shader
    int colorLoc = glGetUniformLocation(m_ShaderProgram, "u_Color");
    glUniform3fv(colorLoc, 1, &m_Color[0]);

    // Draw the triangle
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
  }

  void OnEvent(BeEngine::Event &e) override {
    BeEngine::EventDispatcher dispatcher(e);

    dispatcher.Dispatch<BeEngine::KeyPressedEvent>(
        [this](BeEngine::KeyPressedEvent &ke) {
          if (ke.GetKeyCode() == BeEngine::KeyCode::Space && !ke.IsRepeat()) {
            // Cycle through colors
            m_ColorIndex = (m_ColorIndex + 1) % 5;
            m_Color = m_Colors[m_ColorIndex];
            BE_INFO("Color changed!");
            return true;
          }
          return false;
        });
  }

  void OnImGuiRender() override {
    ImGui::Begin("Triangle Controls");

    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();

    ImGui::SliderFloat("Rotation Speed", &m_RotationSpeed, 0.0f, 180.0f);
    ImGui::ColorEdit3("Color", &m_Color[0]);

    if (ImGui::Button("Reset Rotation")) {
      m_Rotation = 0.0f;
    }

    ImGui::End();
  }

private:
  void CreateTriangle() {
    // Triangle vertices (x, y, z)
    float vertices[] = {
        // Position
        0.0f,  0.5f,  0.0f, // Top
        -0.5f, -0.5f, 0.0f, // Bottom Left
        0.5f,  -0.5f, 0.0f  // Bottom Right
    };

    // Create VAO (Vertex Array Object)
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Create VBO (Vertex Buffer Object)
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configure vertex attributes
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }

  void CreateShader() {
    // Vertex Shader
    const char *vertexShaderSource = R"(
            #version 410 core

            layout(location = 0) in vec3 a_Position;

            uniform float u_Rotation;

            void main() {
                // Simple 2D rotation around Z axis
                float c = cos(u_Rotation);
                float s = sin(u_Rotation);
                mat2 rotation = mat2(c, s, -s, c);

                vec2 rotated = rotation * a_Position.xy;
                gl_Position = vec4(rotated, a_Position.z, 1.0);
            }
        )";

    // Fragment Shader
    const char *fragmentShaderSource = R"(
            #version 410 core

            out vec4 FragColor;

            uniform vec3 u_Color;

            void main() {
                FragColor = vec4(u_Color, 1.0);
            }
        )";

    // Compile vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    CheckShaderCompilation(vertexShader, "VERTEX");

    // Compile fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    CheckShaderCompilation(fragmentShader, "FRAGMENT");

    // Link shaders into program
    m_ShaderProgram = glCreateProgram();
    glAttachShader(m_ShaderProgram, vertexShader);
    glAttachShader(m_ShaderProgram, fragmentShader);
    glLinkProgram(m_ShaderProgram);
    CheckProgramLinking(m_ShaderProgram);

    // Delete shaders (they're linked into program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
  }

  void CheckShaderCompilation(unsigned int shader, const char *type) {
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 512, nullptr, infoLog);
      BE_CORE_ERROR("Shader compilation failed ({}): {}", type, infoLog);
    }
  }

  void CheckProgramLinking(unsigned int program) {
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(program, 512, nullptr, infoLog);
      BE_CORE_ERROR("Shader program linking failed: {}", infoLog);
    }
  }

  // OpenGL objects
  unsigned int m_VAO = 0; // Vertex Array Object
  unsigned int m_VBO = 0; // Vertex Buffer Object
  unsigned int m_ShaderProgram = 0;

  // Animation
  float m_Rotation = 0.0f;
  float m_RotationSpeed = 45.0f;

  // Colors
  glm::vec3 m_Colors[5] = {
      {1.0f, 0.0f, 0.0f}, // Red
      {0.0f, 1.0f, 0.0f}, // Green
      {0.0f, 0.0f, 1.0f}, // Blue
      {1.0f, 1.0f, 0.0f}, // Yellow
      {1.0f, 0.0f, 1.0f}  // Magenta
  };
  glm::vec3 m_Color = m_Colors[0];
  int m_ColorIndex = 0;
};
