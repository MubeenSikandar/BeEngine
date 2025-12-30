#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : m_WindowHandle(windowHandle) {
        BE_CORE_ASSERT(windowHandle, "Window Handle is Null!");
    }

    OpenGLContext::~OpenGLContext(){
         // GLFW handles context destruction when window is destroyed
    }

    void OpenGLContext::Init(){
        BE_CORE_INFO("Initializing OpenGL Context...");

        // Make this context current
         MakeCurrent();

         // Load OpenGL function pointers with GLAD
           int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
           BE_CORE_ASSERT(status, "Failed to initialize GLAD!");

           // Log OpenGL information
           m_RendererInfo = (const char*)glGetString(GL_RENDERER);
           m_VersionInfo = (const char*)glGetString(GL_VERSION);

           BE_CORE_INFO("OpenGL Context Initialized:");
           BE_CORE_INFO("  Vendor:   {}", (const char*)glGetString(GL_VENDOR));
           BE_CORE_INFO("  Renderer: {}", m_RendererInfo);
           BE_CORE_INFO("  Version:  {}", m_VersionInfo);
           BE_CORE_INFO("  GLSL:     {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

           // Optional: Check for required OpenGL version
           // GLint major, minor;
           // glGetIntegerv(GL_MAJOR_VERSION, &major);
           // glGetIntegerv(GL_MINOR_VERSION, &minor);

           // BE_CORE_ASSERT(major > 4 || (major == 4 && minor >= 5),
           //                "BeEngine requires at least OpenGL 4.5!");

           // BE_CORE_INFO("OpenGL {}.{} context created successfully", major, minor);
         }

         void OpenGLContext::SwapBuffers() {
           glfwSwapBuffers(m_WindowHandle);
         }

         void OpenGLContext::MakeCurrent() {
           glfwMakeContextCurrent(m_WindowHandle);
         }
}
