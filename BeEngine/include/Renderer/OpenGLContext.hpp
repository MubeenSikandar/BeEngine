#pragma once
#include "GraphicsContext.hpp"

// Forward declare GLFWwindow to avoid including GLFW in header
struct GLFWwindow;

namespace BeEngine {

    /**
     * @brief OpenGL-specific graphics context
     *
     * Manages OpenGL context creation and activation.
     * Uses GLFW for cross-platform window/context management.
     */
     class BE_API OpenGLContext : public GraphicsContext{
         public:
         /**
           * @brief Construct OpenGL context
           * @param windowHandle GLFW window handle
           */
           explicit OpenGLContext(GLFWwindow* windowHandle);
           ~OpenGLContext() override;

           void Init() override;
           void SwapBuffers() override;
           void MakeCurrent() override;

           const char* GetRendererInfo() const override {return m_RendererInfo;}
           const char* GetVersionInfo() const override {return m_VersionInfo;}

           private:
           GLFWwindow* m_WindowHandle;

           // Context information (cached after Init())
           const char* m_RendererInfo{"Unknown"};
           const char* m_VersionInfo{"Unknown"};

     };

}
