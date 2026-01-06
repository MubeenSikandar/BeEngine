# BeEngine - Complete Technical Documentation

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Core Systems](#2-core-systems)
3. [Rendering System](#3-rendering-system)
4. [Camera System](#4-camera-system)
5. [Event System](#5-event-system)
6. [Input System](#6-input-system)
7. [Layer System](#7-layer-system)
8. [Time System](#8-time-system)
9. [Math Library](#9-math-library)
10. [Window System](#10-window-system)
11. [ImGui Integration](#11-imgui-integration)
12. [Logging System](#12-logging-system)
13. [Build System](#13-build-system)
14. [Design Patterns Used](#14-design-patterns-used)
15. [Future Roadmap](#15-future-roadmap)

---

## 1. Architecture Overview

### 1.1 Engine Philosophy

BeEngine is designed as a **hybrid game/simulation engine** built with modern C++23 standards. The architecture follows these core principles:

- **API Agnostic**: Abstract interfaces allow swapping rendering backends (OpenGL, Vulkan, DirectX, Metal)
- **Factory Pattern**: All GPU resources are created through factory methods
- **RAII**: Resources are automatically cleaned up using smart pointers
- **Event-Driven**: Loose coupling through an event system
- **Layer-Based**: Modular architecture through stackable layers

### 1.2 Directory Structure

```
BeEngine/
├── BeEngine/                    # Engine Core Library
│   ├── include/                 # Public Headers
│   │   ├── Application.hpp      # Main application class
│   │   ├── Core.hpp             # Platform macros & utilities
│   │   ├── EntryPoint.hpp       # Cross-platform entry point
│   │   ├── Window.hpp           # Window abstraction
│   │   ├── Camera/              # Camera system
│   │   ├── Codes/               # Input code definitions
│   │   ├── Events/              # Event system
│   │   ├── ImGUI/               # ImGui layer
│   │   ├── Input/               # Input handling
│   │   ├── Layers/              # Layer system
│   │   ├── Log/                 # Logging system
│   │   ├── Math/                # Math utilities (Transform)
│   │   ├── PCH/                 # Precompiled headers
│   │   ├── Renderer/            # Rendering abstraction
│   │   └── Time/                # Time management
│   ├── src/                     # Implementation files
│   └── vendor/                  # Third-party libraries
│       ├── glad/                # OpenGL loader
│       ├── GLFW/                # Windowing
│       ├── glm/                 # Math library
│       ├── imgui/               # Dear ImGui
│       └── spdlog/              # Logging
├── Sandbox/                     # Test application
└── build/                       # Build output
```

### 1.3 Dependency Graph

```
┌─────────────────────────────────────────────────────────────┐
│                      Application                            │
├─────────────────────────────────────────────────────────────┤
│  Window  │  LayerStack  │  EventQueue  │  Renderer  │ Time │
├─────────────────────────────────────────────────────────────┤
│     GLFW     │    ImGui    │    OpenGL/Glad    │   spdlog  │
├─────────────────────────────────────────────────────────────┤
│                    Operating System                          │
└─────────────────────────────────────────────────────────────┘
```

---

## 2. Core Systems

### 2.1 Core.hpp - Platform Abstraction

The `Core.hpp` file provides essential platform detection and utility macros.

#### Platform Detection

```cpp
// Compiler Detection
#if defined(_MSC_VER)
    #define BE_COMPILER_MSVC
#elif defined(__GNUC__)
    #define BE_COMPILER_GCC
#elif defined(__clang__)
    #define BE_COMPILER_CLANG
#endif

// Platform Detection
#if defined(_WIN32) || defined(_WIN64)
    #define BE_PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #define BE_PLATFORM_APPLE
#elif defined(__linux__)
    #define BE_PLATFORM_LINUX
#endif

// Architecture Detection
#if defined(__x86_64__) || defined(_M_X64)
    #define BE_ARCH_X64
#elif defined(__i386__) || defined(_M_IX86)
    #define BE_ARCH_X86
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define BE_ARCH_ARM64
#endif
```

#### DLL Export/Import Macros

```cpp
#ifdef BE_PLATFORM_WINDOWS
    #ifdef BE_BUILD_DLL
        #define BE_API __declspec(dllexport)
    #else
        #define BE_API __declspec(dllimport)
    #endif
#else
    #define BE_API __attribute__((visibility("default")))
#endif
```

#### Utility Macros

```cpp
// Attribute for nodiscard (compiler warns if return value is ignored)
#define NODISCARD [[nodiscard]]

// Debug assertions (disabled in release)
#ifdef BE_DEBUG
    #define BE_CORE_ASSERT(x, msg) \
        if (!(x)) { \
            BE_CORE_CRITICAL("Assertion Failed: {}", msg); \
            __debugbreak(); \
        }
#else
    #define BE_CORE_ASSERT(x, msg)
#endif
```

### 2.2 EntryPoint.hpp - Application Entry

The entry point abstracts the `main()` function across platforms:

```cpp
// Platform-specific entry point
extern BeEngine::Application* BeEngine::CreateApplication();

int main(int argc, char** argv) {
    // Initialize logging
    BeEngine::Log::Init();
    BE_CORE_INFO("BeEngine Initialized");
    
    // Create and run application
    auto app = BeEngine::CreateApplication();
    app->Run();
    delete app;
    
    BE_CORE_INFO("Application finished Normally");
    return 0;
}
```

**Why This Design?**
- Client code only needs to implement `CreateApplication()`
- Engine controls initialization order
- Platform-specific entry points (WinMain, etc.) handled internally

### 2.3 Application.hpp - Main Application Class

The `Application` class is the heart of the engine:

```cpp
class BE_API Application {
public:
    Application();
    virtual ~Application();
    
    void Run();                                    // Main loop
    virtual void OnEvent(Event& event);            // Event handler
    
    void PushLayer(std::shared_ptr<Layer> layer);  // Add layer
    void PushOverlay(std::shared_ptr<Layer> overlay); // Add overlay
    void PopLayer(const std::shared_ptr<Layer>& layer);
    void PopOverlay(const std::shared_ptr<Layer>& overlay);
    
    Window& GetWindow() { return *m_Window; }
    LayerStack& GetLayerStack() { return m_LayerStack; }
    EventQueue& GetEventQueue() { return m_EventQueue; }
    
    static Application& Get() { return *s_Instance; }
    
    void SetBlockEvents(bool block) { m_BlockEvents = block; }
    bool IsBlockingEvents() const { return m_BlockEvents; }
    void Close() { m_Running = false; }

private:
    std::unique_ptr<Window> m_Window;
    LayerStack m_LayerStack;
    EventQueue m_EventQueue;
    std::shared_ptr<ImGuiLayer> m_ImGuiLayer;
    
    bool m_Running = true;
    bool m_Minimized = false;
    bool m_BlockEvents = true;
    
    static Application* s_Instance;  // Singleton
};
```

#### Main Game Loop

```cpp
void Application::Run() {
    while (m_Running && !m_Window->shouldClose()) {
        // 1. Update time system
        Time::Update();
        Input::Update();
        
        // 2. Process event queue
        ProcessEvents();
        
        if (!m_Minimized) {
            // 3. Fixed timestep updates (physics/simulation)
            while (Time::ShouldRunFixedUpdate()) {
                FixedUpdateLayers(Time::GetFixedDeltaTime());
                Time::ConsumeFixedTime();
            }
            
            // 4. Variable timestep updates (game logic)
            UpdateLayers(Timestep(Time::GetDeltaTime()));
            
            // 5. Render phase
            Renderer::BeginFrame();
            
            // Render layers to framebuffers
            for (auto& layer : m_LayerStack) {
                if (layer && layer->IsEnabled()) {
                    layer->OnRender();
                }
            }
            
            // Render ImGui
            m_ImGuiLayer->Begin();
            m_ImGuiLayer->BeginDockspace();
            for (auto& layer : m_LayerStack) {
                if (layer && layer->IsEnabled()) {
                    layer->OnImGuiRender();
                }
            }
            m_ImGuiLayer->EndDockspace();
            m_ImGuiLayer->End();
            
            Renderer::EndFrame();
            Input::EndFrame();
        }
        
        // 6. Swap buffers
        m_Window->OnUpdate();
    }
}
```

---

## 3. Rendering System

### 3.1 Architecture Overview

The rendering system uses an **abstract factory pattern** to support multiple graphics APIs:

```
                    Abstract Interfaces
┌─────────────────┬─────────────────┬──────────────────┐
│  VertexBuffer   │   IndexBuffer   │   VertexArray    │
│  Shader         │   Framebuffer   │   Renderer       │
│  RendererAPI    │   Texture       │   GraphicsContext│
└────────┬────────┴────────┬────────┴─────────┬────────┘
         │                 │                  │
         ▼                 ▼                  ▼
┌─────────────────┬─────────────────┬──────────────────┐
│ OpenGLVertex    │ OpenGLIndex     │ OpenGLVertex     │
│ Buffer          │ Buffer          │ Array            │
│ OpenGLShader    │ OpenGLFrame     │ OpenGLRenderer   │
│                 │ buffer          │                  │
└─────────────────┴─────────────────┴──────────────────┘
```

### 3.2 RendererAPI - Graphics API Abstraction

```cpp
enum class RenderAPI : uint8_t {
    None = 0,
    OpenGL,
    Vulkan,
    DirectX11,
    DirectX12,
    Metal
};

class BE_API RendererAPI {
public:
    virtual ~RendererAPI() = default;
    
    virtual void Init() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, 
                             uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const glm::vec4& color) = 0;
    virtual void Clear() = 0;
    
    virtual void DrawIndexed(const std::shared_ptr<VertexArray>& va,
                             uint32_t indexCount = 0) = 0;
    virtual void DrawLines(const std::shared_ptr<VertexArray>& va,
                           uint32_t vertexCount) = 0;
    
    static void SetAPI(RenderAPI api) { s_API = api; }
    static RenderAPI GetAPI() { return s_API; }
    
private:
    static RenderAPI s_API;
};
```

### 3.3 BufferLayout - Vertex Attribute Description

The BufferLayout system describes how vertex data is structured:

```cpp
enum class ShaderDataType : uint8_t {
    None = 0,
    Float, Float2, Float3, Float4,
    Int, Int2, Int3, Int4,
    Mat3, Mat4,
    Bool
};

struct BufferElement {
    std::string Name;
    ShaderDataType Type;
    uint32_t Size;
    uint32_t Offset;
    bool Normalized;
    
    uint32_t GetComponentCount() const {
        switch (Type) {
            case ShaderDataType::Float:  return 1;
            case ShaderDataType::Float2: return 2;
            case ShaderDataType::Float3: return 3;
            case ShaderDataType::Float4: return 4;
            // ... etc
        }
    }
};

class BufferLayout {
public:
    BufferLayout(std::initializer_list<BufferElement> elements)
        : m_Elements(elements) {
        CalculateOffsetsAndStride();
    }
    
    const std::vector<BufferElement>& GetElements() const;
    uint32_t GetStride() const { return m_Stride; }
    
private:
    void CalculateOffsetsAndStride() {
        uint32_t offset = 0;
        for (auto& element : m_Elements) {
            element.Offset = offset;
            offset += element.Size;
            m_Stride += element.Size;
        }
    }
    
    std::vector<BufferElement> m_Elements;
    uint32_t m_Stride = 0;
};
```

**Usage Example:**
```cpp
auto vb = VertexBuffer::Create(vertices, sizeof(vertices));
vb->SetLayout({
    { ShaderDataType::Float3, "a_Position" },
    { ShaderDataType::Float3, "a_Color" },
    { ShaderDataType::Float2, "a_TexCoord" }
});
```

### 3.4 VertexBuffer

```cpp
// Abstract Interface
class BE_API VertexBuffer {
public:
    virtual ~VertexBuffer() = default;
    
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    
    virtual void SetLayout(const BufferLayout& layout) = 0;
    virtual const BufferLayout& GetLayout() const = 0;
    
    static std::shared_ptr<VertexBuffer> Create(const void* vertices,
                                                 uint32_t size);
};

// OpenGL Implementation
class OpenGLVertexBuffer : public VertexBuffer {
public:
    OpenGLVertexBuffer(const void* vertices, uint32_t size) {
        // Use DSA if available (OpenGL 4.5+)
        if (GLAD_GL_VERSION_4_5) {
            glCreateBuffers(1, &m_RendererID);
            glNamedBufferData(m_RendererID, size, vertices, GL_STATIC_DRAW);
        } else {
            glGenBuffers(1, &m_RendererID);
            glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
            glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
        }
    }
    
    ~OpenGLVertexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }
    
    void Bind() const override {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }
    
private:
    uint32_t m_RendererID = 0;
    BufferLayout m_Layout;
};
```

### 3.5 IndexBuffer

```cpp
class BE_API IndexBuffer {
public:
    virtual ~IndexBuffer() = default;
    
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    virtual uint32_t GetCount() const = 0;
    
    static std::shared_ptr<IndexBuffer> Create(uint32_t* indices,
                                                uint32_t count);
};
```

### 3.6 VertexArray (VAO)

The VertexArray combines vertex buffers and their layouts:

```cpp
class BE_API VertexArray {
public:
    virtual ~VertexArray() = default;
    
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    
    virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vb) = 0;
    virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& ib) = 0;
    
    virtual const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const = 0;
    
    static std::shared_ptr<VertexArray> Create();
};
```

**OpenGL Implementation Key Code:**

```cpp
void OpenGLVertexArray::AddVertexBuffer(
    const std::shared_ptr<VertexBuffer>& vertexBuffer) {
    
    glBindVertexArray(m_RendererID);
    vertexBuffer->Bind();
    
    const auto& layout = vertexBuffer->GetLayout();
    uint32_t index = 0;
    
    for (const auto& element : layout) {
        switch (element.Type) {
            case ShaderDataType::Float:
            case ShaderDataType::Float2:
            case ShaderDataType::Float3:
            case ShaderDataType::Float4: {
                glEnableVertexAttribArray(index);
                glVertexAttribPointer(
                    index,
                    element.GetComponentCount(),
                    GL_FLOAT,
                    element.Normalized ? GL_TRUE : GL_FALSE,
                    layout.GetStride(),
                    (const void*)(uintptr_t)element.Offset
                );
                index++;
                break;
            }
            case ShaderDataType::Int:
            case ShaderDataType::Int2:
            case ShaderDataType::Int3:
            case ShaderDataType::Int4: {
                glEnableVertexAttribArray(index);
                glVertexAttribIPointer(  // Note: IPointer for integers
                    index,
                    element.GetComponentCount(),
                    GL_INT,
                    layout.GetStride(),
                    (const void*)(uintptr_t)element.Offset
                );
                index++;
                break;
            }
        }
    }
    
    m_VertexBuffers.push_back(vertexBuffer);
}
```

### 3.7 Shader

```cpp
class BE_API Shader {
public:
    virtual ~Shader() = default;
    
    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;
    
    virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;
    virtual void SetVec3(const std::string& name, const glm::vec3& value) = 0;
    virtual void SetVec4(const std::string& name, const glm::vec4& value) = 0;
    virtual void SetInt(const std::string& name, int value) = 0;
    virtual void SetFloat(const std::string& name, float value) = 0;
    
    static std::shared_ptr<Shader> Create(const std::string& vertexSrc,
                                           const std::string& fragmentSrc);
};
```

**OpenGL Shader Compilation:**

```cpp
uint32_t OpenGLShader::CompileShader(uint32_t type, const std::string& source) {
    uint32_t shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    // Check for errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        BE_CORE_ERROR("Shader compilation failed: {}", infoLog);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}
```

### 3.8 Framebuffer

The framebuffer allows off-screen rendering, essential for:
- Editor viewports (render scene to ImGui window)
- Post-processing effects
- Shadow mapping
- Entity picking

```cpp
enum class FramebufferTextureFormat : uint8_t {
    None = 0,
    
    // Color formats
    RGBA8,           // Standard 8-bit RGBA
    RGBA16F,         // HDR 16-bit float
    RGBA32F,         // HDR 32-bit float
    RED_INTEGER,     // For entity picking (stores IDs)
    
    // Depth formats
    DEPTH24STENCIL8, // Combined depth + stencil
    DEPTH32F,        // 32-bit float depth
    
    Depth = DEPTH24STENCIL8
};

struct FramebufferSpecification {
    uint32_t Width = 1280;
    uint32_t Height = 720;
    uint32_t Samples = 1;  // MSAA samples
    
    FramebufferAttachmentSpecification Attachments;
    bool SwapChainTarget = false;
};

class BE_API Framebuffer {
public:
    virtual ~Framebuffer() = default;
    
    virtual void Bind() = 0;
    virtual void Unbind() = 0;
    virtual void Resize(uint32_t width, uint32_t height) = 0;
    
    virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;
    virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;
    
    virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;
    virtual const FramebufferSpecification& GetSpecification() const = 0;
    
    static std::shared_ptr<Framebuffer> Create(
        const FramebufferSpecification& spec);
};
```

**OpenGL Framebuffer Creation:**

```cpp
void OpenGLFramebuffer::Invalidate() {
    if (m_RendererID) {
        // Cleanup existing resources
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);
    }
    
    glCreateFramebuffers(1, &m_RendererID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
    
    // Create color attachments
    for (size_t i = 0; i < m_ColorAttachmentSpecs.size(); i++) {
        auto& spec = m_ColorAttachmentSpecs[i];
        
        if (spec.TextureFormat == FramebufferTextureFormat::RGBA8) {
            glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachments[i]);
            glBindTexture(GL_TEXTURE_2D, m_ColorAttachments[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
                        m_Specification.Width, m_Specification.Height,
                        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
                                   GL_TEXTURE_2D, m_ColorAttachments[i], 0);
        }
    }
    
    // Create depth attachment
    if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None) {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_DepthAttachment);
        glBindTexture(GL_TEXTURE_2D, m_DepthAttachment);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
                    m_Specification.Width, m_Specification.Height,
                    0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                               GL_TEXTURE_2D, m_DepthAttachment, 0);
    }
    
    // Verify framebuffer is complete
    BE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == 
                   GL_FRAMEBUFFER_COMPLETE,
                   "Framebuffer is incomplete!");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
```

### 3.9 GraphicsContext

```cpp
class BE_API GraphicsContext {
public:
    virtual ~GraphicsContext() = default;
    
    virtual void Init() = 0;
    virtual void SwapBuffers() = 0;
    
    virtual std::string GetRendererInfo() const = 0;
    virtual std::string GetVersionInfo() const = 0;
    
    static std::unique_ptr<GraphicsContext> Create(void* window);
};

// OpenGL Implementation
class OpenGLContext : public GraphicsContext {
public:
    OpenGLContext(GLFWwindow* windowHandle)
        : m_WindowHandle(windowHandle) {}
    
    void Init() override {
        glfwMakeContextCurrent(m_WindowHandle);
        
        // Load OpenGL functions via GLAD
        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        BE_CORE_ASSERT(status, "Failed to initialize GLAD!");
        
        BE_CORE_INFO("OpenGL Info:");
        BE_CORE_INFO("  Vendor: {}", (const char*)glGetString(GL_VENDOR));
        BE_CORE_INFO("  Renderer: {}", (const char*)glGetString(GL_RENDERER));
        BE_CORE_INFO("  Version: {}", (const char*)glGetString(GL_VERSION));
    }
    
    void SwapBuffers() override {
        glfwSwapBuffers(m_WindowHandle);
    }
    
private:
    GLFWwindow* m_WindowHandle;
};
```

---

## 4. Camera System

### 4.1 Architecture

```
                     Camera (Abstract Base)
                           │
         ┌─────────────────┴─────────────────┐
         │                                   │
OrthographicCamera                   PerspectiveCamera
         │                                   │
         │                                   │
OrthographicCameraController    PerspectiveCameraController
```

### 4.2 Camera Base Class

```cpp
class BE_API Camera {
public:
    enum class ProjectionType : uint8_t {
        Orthographic = 0,
        Perspective = 1
    };

    virtual ~Camera() = default;

    // Matrices
    const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
    const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
    const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

    // Transform
    const glm::vec3& GetPosition() const { return m_Position; }
    virtual void SetPosition(const glm::vec3& position);
    
    float GetRotation() const { return m_Rotation; }
    virtual void SetRotation(float rotation);

    // Coordinate conversion
    virtual glm::vec3 ScreenToWorld(const glm::vec2& screenPos,
                                     const glm::vec2& screenSize) const;
    virtual glm::vec2 WorldToScreen(const glm::vec3& worldPos,
                                     const glm::vec2& screenSize) const;

protected:
    Camera(ProjectionType type) : m_ProjectionType(type) {}

    virtual void RecalculateViewMatrix();
    virtual void RecalculateProjectionMatrix() = 0;
    
    void UpdateViewProjectionMatrix() {
        m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
    }

protected:
    glm::mat4 m_ViewMatrix{1.0f};
    glm::mat4 m_ProjectionMatrix{1.0f};
    glm::mat4 m_ViewProjectionMatrix{1.0f};

    glm::vec3 m_Position{0.0f};
    float m_Rotation{0.0f};  // Z-axis rotation (radians)
    float m_NearClip{-1.0f};
    float m_FarClip{1.0f};

    ProjectionType m_ProjectionType;
};
```

### 4.3 Orthographic Camera (2D)

Perfect for:
- 2D games (platformers, top-down)
- UI rendering
- Scientific visualization
- CAD applications

```cpp
class BE_API OrthographicCamera : public Camera {
public:
    // Construct with explicit bounds
    OrthographicCamera(float left, float right, float bottom, float top);
    
    // Construct with aspect ratio and size
    OrthographicCamera(float aspectRatio, float size = 1.0f);

    void SetProjection(float left, float right, float bottom, float top);
    void SetProjectionByAspect(float aspectRatio, float size);
    
    // Zoom control
    float GetZoom() const { return m_Zoom; }
    void SetZoom(float zoom);  // 1.0 = default, 2.0 = 2x zoom
    
    float GetSize() const { return m_Size; }  // Half-height of view
    void SetSize(float size);
    
    void OnViewportResize(float width, float height);

protected:
    void RecalculateProjectionMatrix() override {
        m_ProjectionMatrix = glm::ortho(m_Left, m_Right, m_Bottom, m_Top,
                                        m_NearClip, m_FarClip);
        UpdateViewProjectionMatrix();
    }

private:
    float m_Left{-1.0f}, m_Right{1.0f};
    float m_Bottom{-1.0f}, m_Top{1.0f};
    float m_AspectRatio{16.0f / 9.0f};
    float m_Size{1.0f};
    float m_Zoom{1.0f};
};
```

### 4.4 Perspective Camera (3D)

Perfect for:
- 3D games
- First/Third person views
- 3D visualization

```cpp
class BE_API PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(float fov = 45.0f, float aspectRatio = 16.0f / 9.0f,
                      float nearClip = 0.1f, float farClip = 1000.0f);

    void SetFOV(float fov);
    float GetFOV() const { return m_FOV; }
    
    void SetAspectRatio(float aspectRatio);
    
    // 3D orientation
    void SetEulerAngles(const glm::vec3& euler);  // Pitch, Yaw, Roll (radians)
    void SetOrientation(const glm::quat& orientation);
    void LookAt(const glm::vec3& target, const glm::vec3& up);
    
    // Direction vectors
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    glm::vec3 GetUp() const;

protected:
    void RecalculateViewMatrix() override {
        glm::mat4 rotation = glm::mat4_cast(glm::conjugate(m_Orientation));
        glm::mat4 translation = glm::translate(glm::mat4(1.0f), -m_Position);
        m_ViewMatrix = rotation * translation;
        UpdateViewProjectionMatrix();
    }

    void RecalculateProjectionMatrix() override {
        m_ProjectionMatrix = glm::perspective(
            glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
        UpdateViewProjectionMatrix();
    }

private:
    float m_FOV{45.0f};
    float m_AspectRatio{16.0f / 9.0f};
    glm::vec3 m_EulerAngles{0.0f};
    glm::quat m_Orientation{glm::identity<glm::quat>()};
    glm::vec3 m_Forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_Right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_Up{0.0f, 1.0f, 0.0f};
};
```

### 4.5 Orthographic Camera Controller

Handles input for 2D camera:

```cpp
class BE_API OrthographicCameraController {
public:
    OrthographicCameraController(float aspectRatio, float size = 1.0f,
                                  bool rotation = true);

    void OnUpdate(Timestep ts);
    void OnEvent(Event& event);

    OrthographicCamera& GetCamera() { return m_Camera; }
    
    void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }
    void SetRotationSpeed(float speed) { m_RotationSpeed = speed; }
    void SetZoomSpeed(float speed) { m_ZoomSpeed = speed; }
    void SetZoomLimits(float min, float max);
    
    void OnViewportResize(float width, float height);

private:
    bool OnMouseScrolled(MouseScrolledEvent& event);
    bool OnWindowResized(WindowResizeEvent& event);

    OrthographicCamera m_Camera;
    
    float m_MoveSpeed{5.0f};
    float m_RotationSpeed{180.0f};  // Degrees/second
    float m_ZoomSpeed{0.1f};
    float m_MinZoom{0.1f}, m_MaxZoom{10.0f};
    
    glm::vec3 m_Position{0.0f};
    float m_Rotation{0.0f};
    float m_Zoom{1.0f};
    bool m_RotationEnabled{true};
};
```

**Input Processing:**

```cpp
void OrthographicCameraController::OnUpdate(Timestep ts) {
    float dt = ts.GetSeconds();
    
    // Calculate movement direction based on camera rotation
    float rad = glm::radians(m_Rotation);
    float cos_r = glm::cos(rad);
    float sin_r = glm::sin(rad);
    
    // Speed scales with zoom (zoomed out = move faster)
    float effectiveSpeed = m_MoveSpeed / m_Zoom;

    // WASD Movement (rotated by camera angle)
    if (Input::IsKeyPressed(KeyCode::W)) {
        m_Position.x -= sin_r * effectiveSpeed * dt;
        m_Position.y += cos_r * effectiveSpeed * dt;
    }
    if (Input::IsKeyPressed(KeyCode::S)) {
        m_Position.x += sin_r * effectiveSpeed * dt;
        m_Position.y -= cos_r * effectiveSpeed * dt;
    }
    // ... A, D similar
    
    // Q/E Rotation
    if (m_RotationEnabled) {
        if (Input::IsKeyPressed(KeyCode::Q)) {
            m_Rotation += m_RotationSpeed * dt;
        }
        if (Input::IsKeyPressed(KeyCode::E)) {
            m_Rotation -= m_RotationSpeed * dt;
        }
    }
    
    m_Camera.SetPosition(m_Position);
    m_Camera.SetRotation(glm::radians(m_Rotation));
}
```

### 4.6 Perspective Camera Controller

Handles input for 3D camera with FPS-style controls:

```cpp
class BE_API PerspectiveCameraController {
public:
    enum class Mode : uint8_t {
        Fly = 0,  // Free movement in all directions
        FPS = 1   // Locked to XZ plane
    };

    PerspectiveCameraController(float aspectRatio, float fov = 45.0f,
                                 float nearClip = 0.1f, float farClip = 1000.0f);

    void OnUpdate(Timestep ts);
    void OnEvent(Event& event);

    PerspectiveCamera& GetCamera() { return m_Camera; }
    
    void SetMoveSpeed(float speed) { m_MoveSpeed = speed; }
    void SetSprintMultiplier(float mult) { m_SprintMultiplier = mult; }
    void SetMouseSensitivity(float sens) { m_MouseSensitivity = sens; }
    void SetMode(Mode mode) { m_Mode = mode; }
    
    void SetPosition(const glm::vec3& position);
    void SetYaw(float yaw);
    void SetPitch(float pitch);
    
    float GetYaw() const { return m_Yaw; }
    float GetPitch() const { return m_Pitch; }
    bool IsSprinting() const { return m_IsSprinting; }

private:
    void ProcessKeyboardInput(float dt);
    void ProcessMouseMovement(float xOffset, float yOffset);
    void UpdateCameraVectors();

    PerspectiveCamera m_Camera;
    Mode m_Mode{Mode::Fly};

    glm::vec3 m_Position{0.0f, 0.0f, 3.0f};
    float m_Yaw{-90.0f};    // Start looking down -Z
    float m_Pitch{0.0f};
    
    glm::vec3 m_Front{0.0f, 0.0f, -1.0f};
    glm::vec3 m_Right{1.0f, 0.0f, 0.0f};
    glm::vec3 m_Up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_WorldUp{0.0f, 1.0f, 0.0f};
    
    float m_MoveSpeed{5.0f};
    float m_SprintMultiplier{2.5f};
    float m_MouseSensitivity{0.1f};
    float m_MinPitch{-89.0f}, m_MaxPitch{89.0f};
    
    bool m_RightMouseDown{false};
    bool m_IsSprinting{false};
};
```

**Direction Vector Calculation:**

```cpp
void PerspectiveCameraController::UpdateCameraVectors() {
    // Calculate front vector from Euler angles
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    
    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}
```

---

## 5. Event System

### 5.1 Event Base Class

```cpp
enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

enum EventCategory {
    None = 0,
    EventCategoryApplication = BIT(0),
    EventCategoryInput       = BIT(1),
    EventCategoryKeyboard    = BIT(2),
    EventCategoryMouse       = BIT(3),
    EventCategoryMouseButton = BIT(4)
};

class BE_API Event {
public:
    virtual ~Event() = default;
    
    bool IsHandled = false;
    
    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }
    
    bool IsInCategory(EventCategory category) const {
        return GetCategoryFlags() & category;
    }
};

// Macros to reduce boilerplate
#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; } \
    virtual EventType GetEventType() const override { return GetStaticType(); } \
    virtual const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    virtual int GetCategoryFlags() const override { return category; }
```

### 5.2 Event Dispatcher

Type-safe event dispatching:

```cpp
class EventDispatcher {
public:
    EventDispatcher(Event& event) : m_Event(event) {}

    template<typename T, typename F>
    bool Dispatch(const F& func) {
        if (m_Event.GetEventType() == T::GetStaticType()) {
            m_Event.IsHandled |= func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};
```

**Usage:**

```cpp
void OnEvent(Event& event) {
    EventDispatcher dispatcher(event);
    
    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) {
        if (e.GetKeyCode() == KeyCode::Escape) {
            Close();
            return true;  // Consumed
        }
        return false;
    });
    
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) {
        OnResize(e.GetWidth(), e.GetHeight());
        return false;  // Allow propagation
    });
}
```

### 5.3 Event Queue

Priority-based, thread-safe event queue:

```cpp
struct EventQueueConfig {
    size_t maxQueue = 1000;
    bool dropOnOverflow = true;
    bool enableProfiling = true;
    bool enableLogging = false;
};

class BE_API EventQueue {
public:
    EventQueue(const Config& config = {});
    
    // Queue events
    void QueueEvent(std::unique_ptr<Event> event);
    void QueueImmediate(std::unique_ptr<Event> event);  // Process immediately
    
    // Process events
    void ProcessEvents();                              // Process all
    void ProcessEvents(size_t maxEvents);              // Process N events
    void ProcessEventsWithBudget(double maxTimeMs);    // Time-limited
    
    // Subscriptions
    uint64_t Subscribe(EventType type, EventListener listener, int priority = 0);
    uint64_t Subscribe(EventListener listener, int priority = 0);  // Global
    void Unsubscribe(uint64_t listenerID);
    
    // Filtering
    void BlockEventType(EventType type);
    void UnBlockEventType(EventType type);
    void BlockCategory(EventCategory category);
    
    // Statistics
    EventStats GetStats() const;
    void ResetStats();

private:
    void ProcessEvent(Event& event);
    void NotifyListener(Event& event);
    
    // Priority queue for events
    std::priority_queue<QueuedEvent> m_EventQueue;
    
    // Listeners organized by type
    std::unordered_map<EventType, std::vector<EventListenerHandle>> m_Listeners;
    std::vector<EventListenerHandle> m_GlobalListeners;
    
    // Blocked events/categories
    std::unordered_set<EventType> m_BlockedEvents;
    uint32_t m_BlockedCategories = 0;
    
    // Thread safety
    mutable std::mutex m_QueueMutex;
    mutable std::mutex m_ListenersMutex;
};
```

### 5.4 Specific Event Types

**Keyboard Events:**

```cpp
class KeyEvent : public Event {
public:
    KeyCode GetKeyCode() const { return m_KeyCode; }
    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
    
protected:
    KeyEvent(KeyCode keycode) : m_KeyCode(keycode) {}
    KeyCode m_KeyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(KeyCode keycode, bool isRepeat)
        : KeyEvent(keycode), m_IsRepeat(isRepeat) {}
    
    bool IsRepeat() const { return m_IsRepeat; }
    
    EVENT_CLASS_TYPE(KeyPressed)
    
private:
    bool m_IsRepeat;
};

class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}
    EVENT_CLASS_TYPE(KeyReleased)
};
```

**Mouse Events:**

```cpp
class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y, float deltaX, float deltaY)
        : m_MouseX(x), m_MouseY(y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}
    
    float GetX() const { return m_MouseX; }
    float GetY() const { return m_MouseY; }
    float GetDeltaX() const { return m_DeltaX; }
    float GetDeltaY() const { return m_DeltaY; }
    
    EVENT_CLASS_TYPE(MouseMoved)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
    
private:
    float m_MouseX, m_MouseY;
    float m_DeltaX, m_DeltaY;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(float xOffset, float yOffset)
        : m_XOffset(xOffset), m_YOffset(yOffset) {}
    
    float GetXOffset() const { return m_XOffset; }
    float GetYOffset() const { return m_YOffset; }
    
    EVENT_CLASS_TYPE(MouseScrolled)
    EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)
    
private:
    float m_XOffset, m_YOffset;
};
```

---

## 6. Input System

### 6.1 Input Class

Polling-based input with frame tracking:

```cpp
class BE_API Input {
public:
    static void Init();
    static void Update();      // Call at start of frame
    static void EndFrame();    // Call at end of frame
    
    // Keyboard
    static bool IsKeyPressed(KeyCode key);
    static bool IsKeyJustPressed(KeyCode key);   // First frame only
    static bool IsKeyJustReleased(KeyCode key);
    
    // Mouse
    static bool IsMouseButtonPressed(MouseButton button);
    static bool IsMouseButtonJustPressed(MouseButton button);
    static bool IsMouseButtonJustReleased(MouseButton button);
    
    static glm::vec2 GetMousePosition();
    static float GetMouseX();
    static float GetMouseY();
    static glm::vec2 GetMouseDelta();
    
    // Scroll
    static float GetScrollDelta();

private:
    static std::unordered_map<KeyCode, bool> s_KeyStates;
    static std::unordered_map<KeyCode, bool> s_PreviousKeyStates;
    static std::unordered_map<MouseButton, bool> s_MouseStates;
    static std::unordered_map<MouseButton, bool> s_PreviousMouseStates;
    
    static glm::vec2 s_MousePosition;
    static glm::vec2 s_PreviousMousePosition;
    static float s_ScrollDelta;
};
```

### 6.2 Key Codes

```cpp
enum class KeyCode : uint16_t {
    // Letters
    A = 65, B = 66, C = 67, /* ... */ Z = 90,
    
    // Numbers
    D0 = 48, D1 = 49, /* ... */ D9 = 57,
    
    // Function keys
    F1 = 290, F2 = 291, /* ... */ F12 = 301,
    
    // Navigation
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    
    // Modifiers
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    
    Space = 32
};
```

### 6.3 Mouse Codes

```cpp
enum class MouseButton : uint16_t {
    Left = 0,
    Right = 1,
    Middle = 2,
    Button4 = 3,
    Button5 = 4,
    Button6 = 5,
    Button7 = 6,
    Button8 = 7
};
```

---

## 7. Layer System

### 7.1 Layer Base Class

```cpp
class BE_API Layer {
public:
    Layer(const std::string& name = "Layer") : m_DebugName(name) {}
    virtual ~Layer() = default;

    virtual void OnAttach() {}          // Called when layer is pushed
    virtual void OnDetach() {}          // Called when layer is popped
    virtual void OnUpdate(Timestep ts) {}        // Variable timestep
    virtual void OnFixedUpdate(float fixedDt) {} // Fixed timestep
    virtual void OnRender() {}          // Render to framebuffer
    virtual void OnImGuiRender() {}     // Render ImGui
    virtual void OnEvent(Event& event) {}

    void Enable() { m_Enabled = true; }
    void Disable() { m_Enabled = false; }
    bool IsEnabled() const { return m_Enabled; }

    const std::string& GetName() const { return m_DebugName; }

protected:
    std::string m_DebugName;
    bool m_Enabled = true;
};
```

### 7.2 Layer Stack

```cpp
class BE_API LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();

    void PushLayer(std::shared_ptr<Layer> layer);
    void PushOverlay(std::shared_ptr<Layer> overlay);
    void PopLayer(const std::shared_ptr<Layer>& layer);
    void PopOverlay(const std::shared_ptr<Layer>& overlay);

    // Iteration support
    auto begin() { return m_Layers.begin(); }
    auto end() { return m_Layers.end(); }
    auto rbegin() { return m_Layers.rbegin(); }
    auto rend() { return m_Layers.rend(); }

private:
    std::vector<std::shared_ptr<Layer>> m_Layers;
    unsigned int m_LayerInsertIndex = 0;
};
```

**Stack Organization:**

```
┌─────────────────────────────────────────┐
│              Overlays                   │  ← Events first, Render last
│  ┌─────────────────────────────────┐    │
│  │         ImGuiLayer              │    │
│  └─────────────────────────────────┘    │
│  ┌─────────────────────────────────┐    │
│  │       DebugOverlay              │    │
│  └─────────────────────────────────┘    │
├─────────────────────────────────────────┤
│               Layers                    │  ← Events last, Render first
│  ┌─────────────────────────────────┐    │
│  │         GameLayer               │    │
│  └─────────────────────────────────┘    │
│  ┌─────────────────────────────────┐    │
│  │         UILayer                 │    │
│  └─────────────────────────────────┘    │
└─────────────────────────────────────────┘
        m_LayerInsertIndex ↑
```

---

## 8. Time System

### 8.1 Time Management

Supports both variable and fixed timestep for games and simulations:

```cpp
class BE_API Time {
public:
    static void Update();                    // Call once per frame
    
    // Variable timestep (for game logic)
    static float GetDeltaTime();             // Scaled
    static float GetUnscaledDeltaTime();     // Raw
    
    // Total time
    static float GetTime();                  // Scaled total
    static float GetUnscaledTime();          // Raw total
    
    // Fixed timestep (for physics/simulation)
    static float GetFixedDeltaTime();        // Default 1/60
    static void SetFixedDeltaTime(float dt);
    static bool ShouldRunFixedUpdate();      // Accumulator check
    static void ConsumeFixedTime();          // After fixed update
    
    // Time control
    static void SetTimeScale(float scale);   // 1.0 = normal, 0.5 = slow-mo
    static float GetTimeScale();
    static void SetPaused(bool paused);
    static bool IsPaused();
    
    // Stats
    static uint64_t GetFrameCount();
    static float GetFPS();

private:
    static std::chrono::high_resolution_clock::time_point s_StartTime;
    static std::chrono::high_resolution_clock::time_point s_LastFrameTime;
    
    static float s_DeltaTime;
    static float s_UnscaledDeltaTime;
    static float s_Time;
    static float s_UnscaledTime;
    static float s_TimeScale;
    static bool s_Paused;
    
    static float s_FixedDeltaTime;
    static float s_FixedTimeAccumulator;
    
    static uint64_t s_FrameCount;
    static float s_FPS;
    static float s_FPSTimer;
    static uint32_t s_FrameCounter;
};
```

### 8.2 Implementation Details

```cpp
void Time::Update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    // Calculate delta time
    s_UnscaledDeltaTime = std::chrono::duration<float>(
        currentTime - s_LastFrameTime).count();
    
    // Clamp to prevent "spiral of death" (lag spikes)
    s_UnscaledDeltaTime = std::min(s_UnscaledDeltaTime, 0.25f);
    
    // Apply time scale and pause
    s_DeltaTime = s_Paused ? 0.0f : s_UnscaledDeltaTime * s_TimeScale;
    
    // Accumulate total time
    s_UnscaledTime += s_UnscaledDeltaTime;
    s_Time += s_DeltaTime;
    
    // Fixed timestep accumulator
    if (!s_Paused) {
        s_FixedTimeAccumulator += s_UnscaledDeltaTime;
    }
    
    // FPS calculation (1-second window)
    s_FPSTimer += s_UnscaledDeltaTime;
    s_FrameCounter++;
    if (s_FPSTimer >= 1.0f) {
        s_FPS = static_cast<float>(s_FrameCounter) / s_FPSTimer;
        s_FrameCounter = 0;
        s_FPSTimer = 0.0f;
    }
    
    s_FrameCount++;
    s_LastFrameTime = currentTime;
}

bool Time::ShouldRunFixedUpdate() {
    return s_FixedTimeAccumulator >= s_FixedDeltaTime;
}

void Time::ConsumeFixedTime() {
    s_FixedTimeAccumulator -= s_FixedDeltaTime;
}
```

### 8.3 Timestep Wrapper

```cpp
class Timestep {
public:
    Timestep(float time = 0.0f) : m_Time(time) {}
    
    operator float() const { return m_Time; }
    
    float GetSeconds() const { return m_Time; }
    float GetMilliseconds() const { return m_Time * 1000.0f; }
    
private:
    float m_Time;
};
```

### 8.4 Usage in Game Loop

```cpp
void Application::Run() {
    while (m_Running) {
        Time::Update();
        
        // Fixed timestep for physics/simulation
        while (Time::ShouldRunFixedUpdate()) {
            FixedUpdateLayers(Time::GetFixedDeltaTime());
            Time::ConsumeFixedTime();
        }
        
        // Variable timestep for game logic
        UpdateLayers(Timestep(Time::GetDeltaTime()));
        
        // Render...
    }
}
```

---

## 9. Math Library

### 9.1 Transform Component

Full 3D transform with hierarchy support:

```cpp
class BE_API Transform {
public:
    Transform();
    Transform(const glm::vec3& position);
    Transform(const glm::vec3& position, const glm::vec3& rotation,
              const glm::vec3& scale);

    // Position
    void SetPosition(const glm::vec3& position);
    void Translate(const glm::vec3& delta);
    const glm::vec3& GetPosition() const { return m_Position; }
    glm::vec3 GetWorldPosition() const;

    // Rotation (euler degrees or quaternion)
    void SetRotation(const glm::vec3& eulerDegrees);
    void SetRotation(const glm::quat& quaternion);
    void Rotate(const glm::vec3& eulerDegrees);
    void RotateAround(const glm::vec3& axis, float angleDegrees);
    const glm::quat& GetRotation() const { return m_Rotation; }
    glm::vec3 GetEulerAngles() const;
    glm::quat GetWorldRotation() const;

    // Scale
    void SetScale(const glm::vec3& scale);
    void SetScale(float uniformScale);
    const glm::vec3& GetScale() const { return m_Scale; }
    glm::vec3 GetWorldScale() const;

    // Direction vectors
    glm::vec3 GetForward() const;  // Local -Z
    glm::vec3 GetRight() const;    // Local +X
    glm::vec3 GetUp() const;       // Local +Y

    // Matrices (lazy evaluation with dirty flags)
    const glm::mat4& GetLocalMatrix();
    const glm::mat4& GetWorldMatrix();
    glm::mat4 GetInverseWorldMatrix();

    // Hierarchy
    void SetParent(Transform* parent);
    Transform* GetParent() const { return m_Parent; }
    const std::vector<Transform*>& GetChildren() const { return m_Children; }
    void AddChild(Transform* child);
    void RemoveChild(Transform* child);

    // Utility
    void LookAt(const glm::vec3& target, const glm::vec3& up);
    void Reset();

    // 2D convenience
    void SetPosition2D(float x, float y);
    glm::vec2 GetPosition2D() const;
    void SetRotation2D(float angleDegrees);
    float GetRotation2D() const;

private:
    void MarkDirty();
    void RecalculateLocalMatrix();
    void RecalculateWorldMatrix();

    glm::vec3 m_Position{0.0f};
    glm::quat m_Rotation{glm::identity<glm::quat>()};
    glm::vec3 m_Scale{1.0f};

    glm::mat4 m_LocalMatrix{1.0f};
    glm::mat4 m_WorldMatrix{1.0f};

    bool m_LocalDirty{true};
    bool m_WorldDirty{true};

    Transform* m_Parent{nullptr};
    std::vector<Transform*> m_Children;
};
```

### 9.2 Matrix Calculation

```cpp
void Transform::RecalculateLocalMatrix() {
    // Order: Scale -> Rotate -> Translate
    // M = T * R * S
    m_LocalMatrix = glm::translate(glm::mat4(1.0f), m_Position) *
                    glm::mat4_cast(m_Rotation) *
                    glm::scale(glm::mat4(1.0f), m_Scale);
    m_LocalDirty = false;
}

void Transform::RecalculateWorldMatrix() {
    if (m_LocalDirty) {
        RecalculateLocalMatrix();
    }
    
    if (m_Parent) {
        m_WorldMatrix = m_Parent->GetWorldMatrix() * m_LocalMatrix;
    } else {
        m_WorldMatrix = m_LocalMatrix;
    }
    m_WorldDirty = false;
}

void Transform::MarkDirty() {
    m_LocalDirty = true;
    m_WorldDirty = true;
    
    // Propagate to children
    for (auto* child : m_Children) {
        if (child) {
            child->m_WorldDirty = true;
            child->MarkDirty();
        }
    }
}
```

---

## 10. Window System

### 10.1 Window Abstraction

```cpp
struct WindowProps {
    std::string title;
    Width width;
    Height height;
    bool vSync = true;
};

class BE_API Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    Window(const WindowProps& props);
    ~Window();

    void OnUpdate();  // Poll events, swap buffers

    uint32_t GetWidth() const { return m_Data.width; }
    uint32_t GetHeight() const { return m_Data.height; }
    float GetAspectRatio() const;

    void SetEventCallback(const EventCallbackFn& callback);
    void SetVSync(bool enabled);
    bool IsVSync() const { return m_Data.vSync; }
    void SetTitle(const std::string& title);

    void* GetNativeWindow() const { return m_Window; }
    GraphicsContext* GetContext() const { return m_Context.get(); }

    static std::unique_ptr<Window> Create(const WindowProps& props);

private:
    void Init(const WindowProps& props);
    void ShutDown();
    void SetupCallbacks();

    GLFWwindow* m_Window;
    std::unique_ptr<GraphicsContext> m_Context;

    struct WindowData {
        std::string title;
        uint32_t width, height;
        bool vSync;
        EventCallbackFn eventCallback;
        double lastMouseX, lastMouseY;
    };
    WindowData m_Data;
};
```

### 10.2 GLFW Callbacks

```cpp
void Window::SetupCallbacks() {
    // Window resize
    glfwSetWindowSizeCallback(m_Window,
        [](GLFWwindow* window, int width, int height) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.width = width;
            data.height = height;
            
            WindowResizeEvent event(width, height);
            data.eventCallback(event);
        });
    
    // Window close
    glfwSetWindowCloseCallback(m_Window,
        [](GLFWwindow* window) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            data.eventCallback(event);
        });
    
    // Keyboard
    glfwSetKeyCallback(m_Window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            
            switch (action) {
                case GLFW_PRESS: {
                    KeyPressedEvent event(static_cast<KeyCode>(key), false);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    KeyReleasedEvent event(static_cast<KeyCode>(key));
                    data.eventCallback(event);
                    break;
                }
                case GLFW_REPEAT: {
                    KeyPressedEvent event(static_cast<KeyCode>(key), true);
                    data.eventCallback(event);
                    break;
                }
            }
        });
    
    // Mouse scroll
    glfwSetScrollCallback(m_Window,
        [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            data.eventCallback(event);
        });
    
    // Mouse movement (with delta)
    glfwSetCursorPosCallback(m_Window,
        [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            
            float deltaX = (float)(xPos - data.lastMouseX);
            float deltaY = (float)(yPos - data.lastMouseY);
            data.lastMouseX = xPos;
            data.lastMouseY = yPos;
            
            MouseMovedEvent event((float)xPos, (float)yPos, deltaX, deltaY);
            data.eventCallback(event);
        });
    
    // Mouse buttons
    glfwSetMouseButtonCallback(m_Window,
        [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            
            switch (action) {
                case GLFW_PRESS: {
                    MouseButtonPressedEvent event((MouseButton)button);
                    data.eventCallback(event);
                    break;
                }
                case GLFW_RELEASE: {
                    MouseButtonReleasedEvent event((MouseButton)button);
                    data.eventCallback(event);
                    break;
                }
            }
        });
}
```

---

## 11. ImGui Integration

### 11.1 ImGuiLayer

```cpp
class BE_API ImGuiLayer : public Layer {
public:
    ImGuiLayer();
    ~ImGuiLayer() = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnEvent(Event& event) override;

    void Begin();           // Start ImGui frame
    void End();             // End ImGui frame & render
    
    void BeginDockspace();  // Enable docking
    void EndDockspace();
    
    void EnableDockspace() { m_DockspaceEnabled = true; }
    void DisableDockspace() { m_DockspaceEnabled = false; }
    
    bool WantCaptureMouse() const;
    bool WantCaptureKeyboard() const;

private:
    bool m_DockspaceEnabled = true;
};
```

### 11.2 Initialization

```cpp
void ImGuiLayer::OnAttach() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    ImGui::StyleColorsDark();
    SetDarkThemeColors();  // Custom theme
    
    // Platform backends
    Application& app = Application::Get();
    GLFWwindow* window = (GLFWwindow*)app.GetWindow().GetNativeWindow();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}
```

### 11.3 Event Handling (Blocking)

```cpp
void ImGuiLayer::OnEvent(Event& event) {
    if (Application::Get().IsBlockingEvents()) {
        ImGuiIO& io = ImGui::GetIO();
        
        // Block mouse events if ImGui wants mouse
        if (event.IsInCategory(EventCategoryMouse) && io.WantCaptureMouse) {
            event.IsHandled = true;
        }
        
        // Block keyboard events if ImGui wants keyboard
        if (event.IsInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard) {
            event.IsHandled = true;
        }
    }
}
```

### 11.4 Dockspace Setup

```cpp
void ImGuiLayer::BeginDockspace() {
    ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
}
```

---

## 12. Logging System

### 12.1 Logger Setup

```cpp
class BE_API Log {
public:
    static void Init();
    
    static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
        return s_CoreLogger;
    }
    static std::shared_ptr<spdlog::logger>& GetClientLogger() {
        return s_ClientLogger;
    }

private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
};

void Log::Init() {
    // Pattern: [timestamp] [level] [logger] [thread] message
    auto pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [thread %t] %v";
    
    // Console sink with colors
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern(pattern);
    
    // File sink with rotation
    auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        "logs/BeEngine.log", 5 * 1024 * 1024, 3);
    fileSink->set_pattern(pattern);
    
    std::vector<spdlog::sink_ptr> sinks = { consoleSink, fileSink };
    
    s_CoreLogger = std::make_shared<spdlog::logger>("BeEngine", sinks.begin(), sinks.end());
    s_CoreLogger->set_level(spdlog::level::trace);
    
    s_ClientLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
    s_ClientLogger->set_level(spdlog::level::trace);
    
    spdlog::register_logger(s_CoreLogger);
    spdlog::register_logger(s_ClientLogger);
}
```

### 12.2 Logging Macros

```cpp
// Core logging (engine internals)
#define BE_CORE_TRACE(...)    ::BeEngine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define BE_CORE_INFO(...)     ::BeEngine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define BE_CORE_WARN(...)     ::BeEngine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define BE_CORE_ERROR(...)    ::BeEngine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define BE_CORE_CRITICAL(...) ::BeEngine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client logging (game/application)
#define BE_TRACE(...)         ::BeEngine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define BE_INFO(...)          ::BeEngine::Log::GetClientLogger()->info(__VA_ARGS__)
#define BE_WARN(...)          ::BeEngine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define BE_ERROR(...)         ::BeEngine::Log::GetClientLogger()->error(__VA_ARGS__)
#define BE_CRITICAL(...)      ::BeEngine::Log::GetClientLogger()->critical(__VA_ARGS__)

// Disabled in distribution builds
#ifdef BE_DIST
    #define BE_CORE_TRACE(...)
    #define BE_TRACE(...)
#endif
```

---

## 13. Build System

### 13.1 CMake Configuration

```cmake
cmake_minimum_required(VERSION 3.31)
project(BeEngine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build type configuration
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(BE_DEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(BE_RELEASE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Dist")
    add_compile_definitions(BE_DIST)
endif()

# Platform configuration
if(WIN32)
    add_compile_definitions(BE_PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(BE_PLATFORM_APPLE)
elseif(UNIX)
    add_compile_definitions(BE_PLATFORM_LINUX)
endif()

# Engine library
add_library(BeEngine SHARED ${BEENGINE_SOURCES})

target_compile_definitions(BeEngine PRIVATE BE_BUILD_DLL)

target_include_directories(BeEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/vendor/glm
    ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/vendor/spdlog/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/vendor/GLFW/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/vendor/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/BeEngine/vendor/imgui
)

target_link_libraries(BeEngine PUBLIC glfw glad spdlog imgui)

# Sandbox application
add_executable(Sandbox ${SANDBOX_SOURCES})
target_link_libraries(Sandbox PRIVATE BeEngine)
```

### 13.2 CMake Presets

```json
{
  "version": 6,
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "binaryDir": "${sourceDir}/build",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "debug",
      "inherits": "base",
      "displayName": "Debug",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    },
    {
      "name": "release",
      "inherits": "base",
      "displayName": "Release",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "debug",
      "configurePreset": "debug"
    },
    {
      "name": "release",
      "configurePreset": "release"
    }
  ]
}
```

---

## 14. Design Patterns Used

### 14.1 Abstract Factory Pattern

Used for all GPU resources (VertexBuffer, Shader, Framebuffer, etc.):

```cpp
// Abstract interface
class Shader {
    static std::shared_ptr<Shader> Create(const std::string& vs, const std::string& fs);
};

// Implementation selection at runtime
std::shared_ptr<Shader> Shader::Create(const std::string& vs, const std::string& fs) {
    switch (RendererAPI::GetAPI()) {
        case RenderAPI::OpenGL: return std::make_shared<OpenGLShader>(vs, fs);
        case RenderAPI::Vulkan: return std::make_shared<VulkanShader>(vs, fs);
        // etc.
    }
}
```

### 14.2 Singleton Pattern

Used for Application and static utility classes:

```cpp
class Application {
public:
    static Application& Get() { return *s_Instance; }
private:
    static Application* s_Instance;
};
```

### 14.3 Observer Pattern

Event system with subscribers:

```cpp
// Subscribe
eventQueue.Subscribe(EventType::KeyPressed, [](Event& e) {
    // Handle event
    return false;
});

// Notify
eventQueue.QueueEvent(std::make_unique<KeyPressedEvent>(key, repeat));
```

### 14.4 RAII (Resource Acquisition Is Initialization)

Smart pointers for automatic resource cleanup:

```cpp
class OpenGLVertexBuffer {
public:
    OpenGLVertexBuffer() {
        glGenBuffers(1, &m_RendererID);  // Acquire
    }
    ~OpenGLVertexBuffer() {
        glDeleteBuffers(1, &m_RendererID);  // Release
    }
private:
    uint32_t m_RendererID;
};
```

### 14.5 Dirty Flag Pattern

Used in Transform for lazy matrix recalculation:

```cpp
class Transform {
    void SetPosition(const glm::vec3& pos) {
        m_Position = pos;
        m_Dirty = true;  // Mark dirty
    }
    
    const glm::mat4& GetMatrix() {
        if (m_Dirty) {
            RecalculateMatrix();  // Lazy recalculation
            m_Dirty = false;
        }
        return m_Matrix;
    }
};
```

---

## 15. Future Roadmap

### Phase 1: Core Rendering (Current)
- ✅ Camera System (2D & 3D)
- ✅ Buffer Layout System
- ✅ Transform System
- ⬜ Texture Loading
- ⬜ 2D Batch Renderer
- ⬜ Sprite System

### Phase 2: Scene Management
- ⬜ Entity-Component System (ECS)
- ⬜ Scene Graph
- ⬜ Serialization (Save/Load)
- ⬜ Editor Tools

### Phase 3: Game Features
- ⬜ Physics (Box2D for 2D, Bullet/PhysX for 3D)
- ⬜ Audio System
- ⬜ Animation System
- ⬜ Particle System

### Phase 4: Simulation Features
- ⬜ Data Recorder (CSV export)
- ⬜ Headless Mode
- ⬜ Python Bindings
- ⬜ Numerical Integrators (Euler, RK4, Verlet)

### Phase 5: Advanced Features
- ⬜ Networking
- ⬜ Scripting (Lua/Python)
- ⬜ Blockchain Integration
- ⬜ Quantum Computing APIs

---

## Appendix A: Complete File List

```
BeEngine/
├── include/
│   ├── Application.hpp
│   ├── Core.hpp
│   ├── EntryPoint.hpp
│   ├── Window.hpp
│   ├── Camera/
│   │   ├── Camera.hpp
│   │   ├── OrthographicCamera.hpp
│   │   ├── OrthographicCameraController.hpp
│   │   ├── PerspectiveCamera.hpp
│   │   └── PerspectiveCameraController.hpp
│   ├── Codes/
│   │   ├── KeyCodes.hpp
│   │   └── MouseCodes.hpp
│   ├── Events/
│   │   ├── Event.hpp
│   │   ├── EventQueue.hpp
│   │   ├── ApplicationEvent.hpp
│   │   ├── KeyEvent.hpp
│   │   └── MouseEvent.hpp
│   ├── ImGUI/
│   │   └── ImGuiLayer.hpp
│   ├── Input/
│   │   └── Input.hpp
│   ├── Layers/
│   │   ├── Layer.hpp
│   │   └── LayerStack.hpp
│   ├── Log/
│   │   ├── Log.hpp
│   │   └── LogConfig.hpp
│   ├── Math/
│   │   └── Transform.hpp
│   ├── PCH/
│   │   └── BeEnginePCH.hpp
│   ├── Renderer/
│   │   ├── BufferLayout.hpp
│   │   ├── Framebuffer.hpp
│   │   ├── GraphicsContext.hpp
│   │   ├── IndexBuffer.hpp
│   │   ├── Renderer.hpp
│   │   ├── RendererAPI.hpp
│   │   ├── Shader.hpp
│   │   ├── VertexArray.hpp
│   │   ├── VertexBuffer.hpp
│   │   └── OpenGL/
│   │       ├── OpenGLContext.hpp
│   │       ├── OpenGLFramebuffer.hpp
│   │       ├── OpenGLIndexBuffer.hpp
│   │       ├── OpenGLRendererAPI.hpp
│   │       ├── OpenGLShader.hpp
│   │       ├── OpenGLVertexArray.hpp
│   │       └── OpenGLVertexBuffer.hpp
│   └── Time/
│       ├── Time.hpp
│       └── Timestep.hpp
└── src/
    ├── Application.cpp
    ├── Window.cpp
    ├── Camera/
    │   ├── Camera.cpp
    │   ├── OrthographicCamera.cpp
    │   ├── OrthographicCameraController.cpp
    │   ├── PerspectiveCamera.cpp
    │   └── PerspectiveCameraController.cpp
    ├── Events/
    │   ├── Event.cpp
    │   └── EventQueue.cpp
    ├── ImGUI/
    │   └── ImGuiLayer.cpp
    ├── Input/
    │   └── Input.cpp
    ├── Layers/
    │   ├── Layer.cpp
    │   └── LayerStack.cpp
    ├── Log/
    │   └── Log.cpp
    ├── Math/
    │   └── Transform.cpp
    ├── Renderer/
    │   ├── Framebuffer.cpp
    │   ├── Renderer.cpp
    │   ├── RendererAPI.cpp
    │   ├── Shader.cpp
    │   ├── VertexArray.cpp
    │   ├── VertexBuffer.cpp
    │   └── OpenGL/
    │       ├── OpenGLContext.cpp
    │       ├── OpenGLFramebuffer.cpp
    │       ├── OpenGLIndexBuffer.cpp
    │       ├── OpenGLRendererAPI.cpp
    │       ├── OpenGLShader.cpp
    │       ├── OpenGLVertexArray.cpp
    │       └── OpenGLVertexBuffer.cpp
    └── Time/
        └── Time.cpp
```

---
*Last updated: January 2026*
