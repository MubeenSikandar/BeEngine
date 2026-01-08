# BeEngine

A modern, cross-platform C++23 game engine with event-driven architecture, comprehensive logging system, and OpenGL rendering pipeline.

## Overview

BeEngine is a feature-rich game engine built with modern C++ standards, featuring a robust event system, flexible logging infrastructure, OpenGL-based rendering pipeline, and cross-platform support. The engine follows a modular architecture with both shared and static library options, making it easy to integrate into client applications.

## 🚀 Technology Stack

- **C++ Standard**: C++23 (ISO/IEC 14882:2024)
- **Build System**: CMake 3.31+
- **Graphics API**: OpenGL 4.1+
- **Windowing**: GLFW 3.x
- **Mathematics**: GLM (OpenGL Mathematics)
- **UI Framework**: Dear ImGui (docking branch)
- **Image Loading**: STB Image
- **OpenGL Loading**: Glad
- **Logging**: spdlog (integrated as submodule)
- **Architecture**: Shared/Static library (DLL/dylib/so/lib)

### Platform Support

| Platform    | SDK/Compiler                           | Deployment Target |
| ----------- | -------------------------------------- | ----------------- |
| **macOS**   | macOS 15 Sequoia, Clang 18+            | 26.1              |
| **Windows** | Windows 11 SDK 10.0.26100, MSVC 19.40+ | Windows 11 24H2   |
| **Linux**   | GCC 14+ or Clang 18+                   | Kernel 6.x        |

## 🎯 Core Features

### Rendering System

- **OpenGL 4.1+ pipeline** with modern shader support
- **Vertex Array Objects (VAO)** and buffer management
- **Shader compilation and linking** with error handling
- **Texture loading and management** via STB Image
- **Frame buffer objects** for render-to-texture
- **Camera system** with orthographic and perspective projections
- **Transform system** for 3D object positioning and scaling

### Event System

- **Priority-based event queue** with configurable capacity
- **Type-safe event dispatching** using templates
- **Event categories** (Application, Input, Keyboard, Mouse, Window, etc.)
- **Subscriber pattern** with priority levels
- **Event filtering** and blocking by type or category
- **Performance profiling** with statistics tracking
- **Time-budgeted processing** to prevent frame drops

### UI System (ImGui Integration)

- **Dear ImGui integration** with docking support
- **ImGui layer management** with event handling
- **Dockspace support** for editor-style interfaces
- **Input capture management** (mouse/keyboard)
- **Custom styling and theming** support

### Input System

- **Cross-platform input handling** via GLFW
- **Keyboard and mouse event processing**
- **Key code and mouse button abstractions**
- **Input state management** and polling

### Material System

- **Texture management** with OpenGL backend
- **2D texture loading** from various formats
- **Texture binding and sampling** in shaders
- **Material property management**

### Camera System

- **Abstract camera base class** with common functionality
- **Orthographic camera** for 2D rendering and UI
- **Perspective camera** for 3D rendering
- **Camera controllers** with input handling
- **View and projection matrix management**
- **Screen-to-world coordinate conversion**

### Logging System

- **Dual-logger architecture** (Core and Client loggers)
- **Multiple output sinks** (console, file, rotating files)
- **Async logging** for performance
- **Category-based logging** (Core, Events, etc.)
- **Configurable log levels** per category
- **Privacy mode** with redaction patterns
- **Build-specific configurations** (Debug, Release, Distribution)

### Window System

- **GLFW-based windowing** with cross-platform support
- **Event-driven input handling** (keyboard, mouse, window events)
- **Configurable window properties** (size, title, VSync)
- **Native window handle access** for advanced integration
- **Automatic event callback setup** with the event system
- **OpenGL context management**

### Layer System

- **Hierarchical layer management** with LayerStack
- **Overlay support** for UI and debug tools
- **Forward update order** (layers first, overlays last)
- **Reverse event order** (overlays first, layers last)
- **Per-layer enable/disable** functionality
- **Timestep-based updates** for frame-rate independence

### Platform Abstraction

- **Cross-platform macros** for DLL export/import
- **Compiler detection** (MSVC, GCC, Clang)
- **Architecture detection** (x86, x64, ARM, ARM64)
- **Platform-specific optimizations**
- **Smart pointer utilities** (Scope, Ref)

### Build Configurations

- **Debug**: Full logging, assertions enabled, no optimization
- **Release**: Optimized with debug info, reduced logging
- **Distribution**: Maximum optimization, minimal logging, LTO enabled
- **Static/Shared library options** for different deployment needs

## 📁 Project Structure

```
BeEngine/
├── BeEngine/                    # Engine library
│   ├── include/
│   │   ├── Application.hpp      # Main application class
│   │   ├── Core.hpp            # Platform detection & macros
│   │   ├── EntryPoint.hpp      # Application entry point
│   │   ├── KeyCodes.hpp        # Keyboard input codes
│   │   ├── MouseCodes.hpp      # Mouse input codes
│   │   ├── Timestep.hpp        # Frame timing utilities
│   │   ├── Window.hpp          # GLFW window abstraction
│   │   ├── Events/             # Event system
│   │   │   ├── Event.hpp       # Base event class
│   │   │   ├── EventQueue.hpp  # Priority-based event queue
│   │   │   ├── ApplicationEvent.hpp
│   │   │   ├── KeyEvent.hpp
│   │   │   └── MouseEvent.hpp
│   │   ├── Layers/             # Layer system
│   │   │   ├── Layer.hpp       # Base layer class
│   │   │   └── LayerStack.hpp  # Layer management
│   │   ├── Renderer/           # Rendering system
│   │   │   ├── Renderer.hpp    # High-level renderer
│   │   │   ├── RendererAPI.hpp # Abstract rendering API
│   │   │   ├── Shader.hpp      # Shader management
│   │   │   ├── VertexArray.hpp # VAO abstraction
│   │   │   ├── VertexBuffer.hpp# VBO abstraction
│   │   │   ├── IndexBuffer.hpp # EBO abstraction
│   │   │   ├── FrameBuffer.hpp # FBO abstraction
│   │   │   └── OpenGL/         # OpenGL implementation
│   │   ├── Camera/             # Camera system
│   │   │   ├── Camera.hpp      # Base camera class
│   │   │   ├── OrthographicCamera.hpp
│   │   │   ├── PerspectiveCamera.hpp
│   │   │   └── *CameraController.hpp
│   │   ├── ImGui/              # ImGui integration
│   │   │   └── ImGuiLayer.hpp  # ImGui layer
│   │   ├── Input/              # Input handling
│   │   │   └── Input.hpp       # Input abstraction
│   │   ├── MaterialSystem/     # Material and texture system
│   │   │   ├── Texture.hpp     # Texture abstraction
│   │   │   └── OpenGL/         # OpenGL texture implementation
│   │   ├── Math/               # Mathematics utilities
│   │   │   └── Transform.hpp   # 3D transforms
│   │   └── Logs/               # Logging system
│   │       ├── Log.hpp         # Logger interface
│   │       └── LogConfig.hpp   # Configuration
│   ├── src/                    # Implementation files
│   │   ├── Application.cpp
│   │   ├── Window.cpp
│   │   ├── Events/             # Event implementations
│   │   ├── Layers/             # Layer implementations
│   │   ├── Renderer/           # Rendering implementations
│   │   ├── Camera/             # Camera implementations
│   │   ├── ImGui/              # ImGui implementations
│   │   ├── Input/              # Input implementations
│   │   ├── MaterialSystem/     # Material implementations
│   │   ├── Math/               # Math implementations
│   │   └── Logs/               # Logging implementations
│   └── vendor/                 # Third-party dependencies
│       ├── spdlog/            # Logging library (submodule)
│       ├── GLFW/              # Window/input library (submodule)
│       ├── GLM/               # Mathematics library (submodule)
│       ├── imgui/             # UI library (submodule)
│       ├── STB/               # Image loading (submodule)
│       └── Glad/              # OpenGL loader
├── Sandbox/                    # Example application
│   ├── SandBoxApp.cpp         # Main demo application
│   ├── SandboxLayer.cpp       # 2D rendering demo
│   └── SandboxLayer3D.cpp     # 3D rendering demo
├── .vscode/                   # VS Code configuration
├── .zed/                      # Zed editor configuration
├── build/                     # Build output directory
├── logs/                      # Runtime log files
├── CMakeLists.txt             # Build configuration
├── build.sh                   # Unix build script
├── build.bat                  # Windows build script
└── .gitmodules               # Git submodule configuration
```

## 🔧 Building the Engine

### Quick Build (macOS/Linux)

```bash
./build.sh
./build/bin/Sandbox
```

### Manual Build

#### macOS/Linux

```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/Sandbox
```

#### Windows (Visual Studio)

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
.\bin\Release\Sandbox.exe
```

#### Windows (MinGW)

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
.\bin\Sandbox.exe
```

### Build Targets

```bash
# Build everything
cmake --build build

# Run the sandbox application
cmake --build build --target run

# Clean logs directory
cmake --build build --target clean-logs

# Full rebuild
cmake --build build --target rebuild
```

## 📦 Output Structure

```
build/
├── bin/
│   ├── Debug/          # Debug builds
│   │   ├── BeEngine.dll/dylib/so (or .lib/.a for static)
│   │   ├── Sandbox
│   │   └── logs/       # Runtime logs
│   └── Release/        # Release builds
│       ├── BeEngine.dll/dylib/so (or .lib/.a for static)
│       ├── Sandbox
│       └── logs/
└── lib/                # Static libraries
    ├── libglfw3.a
    ├── libspdlogd.a
    ├── libimgui.a
    ├── libstb.a
    └── libglad.a
```

### Platform-Specific Output

| Platform    | Shared Library      | Static Library  | Executable    |
| ----------- | ------------------- | --------------- | ------------- |
| **Windows** | `BeEngine.dll`      | `BeEngine.lib`  | `Sandbox.exe` |
| **macOS**   | `libBeEngine.dylib` | `libBeEngine.a` | `Sandbox`     |
| **Linux**   | `libBeEngine.so`    | `libBeEngine.a` | `Sandbox`     |

## 💻 Creating a Client Application

```cpp
#include "Application.hpp"
#include "EntryPoint.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Layers/Layer.hpp"
#include "Logs/Log.hpp"
#include "Renderer/Renderer.hpp"
#include "Camera/OrthographicCamera.hpp"

// Custom rendering layer
class GameLayer : public BeEngine::Layer {
public:
    GameLayer() : Layer("GameLayer") {
        // Initialize camera
        m_Camera = std::make_unique<BeEngine::OrthographicCamera>(
            -1.6f, 1.6f, -0.9f, 0.9f
        );
    }

    void OnAttach() override {
        BE_INFO("Game layer attached!");

        // Initialize renderer
        BeEngine::Renderer::Init();

        // Setup your geometry, shaders, textures here
        SetupScene();
    }

    void OnUpdate(BeEngine::Timestep ts) override {
        // Update game logic
        UpdateCamera(ts);

        // Render
        BeEngine::Renderer::Clear(0.1f, 0.1f, 0.1f, 1.0f);
        BeEngine::Renderer::BeginFrame();

        // Render your objects here
        RenderScene();

        BeEngine::Renderer::EndFrame();
    }

    void OnEvent(BeEngine::Event& event) override {
        BeEngine::EventDispatcher dispatcher(event);
        dispatcher.Dispatch<BeEngine::KeyPressedEvent>(
            [](BeEngine::KeyPressedEvent& e) {
                BE_INFO("Key pressed: {}", BeEngine::ToString(e.GetKeyCode()));
                return false; // Don't consume the event
            }
        );
    }

    void OnImGuiRender() override {
        // Render debug UI
        ImGui::Begin("Debug Info");
        ImGui::Text("FPS: %.1f", 1.0f / GetTimestep().GetSeconds());
        ImGui::End();
    }

private:
    void SetupScene() {
        // Setup your vertex arrays, shaders, textures
    }

    void UpdateCamera(BeEngine::Timestep ts) {
        // Update camera position, rotation, etc.
    }

    void RenderScene() {
        // Render your geometry
    }

    std::unique_ptr<BeEngine::OrthographicCamera> m_Camera;
};

class MyGame : public BeEngine::Application {
public:
    MyGame() {
        BE_INFO("MyGame Started!");

        // Push your game layer
        PushLayer(std::make_shared<GameLayer>());

        // Push ImGui layer for debug UI
        PushOverlay(std::make_shared<BeEngine::ImGuiLayer>());
    }
};

// Entry point
BeEngine::Application* BeEngine::CreateApplication() {
    return new MyGame();
}
```

## 🔍 Key Components

### Application Class

The base application class provides:

- Main game loop with event processing
- Window creation and management
- Layer stack management
- Event queue management
- Renderer initialization
- Configurable frame rate
- Automatic cleanup

### Rendering System

Modern OpenGL-based rendering featuring:

- **Renderer**: High-level rendering API with state management
- **Shader Management**: Compilation, linking, and uniform handling
- **Vertex Array Objects**: Efficient geometry organization
- **Buffer Management**: Vertex, index, and frame buffer abstractions
- **Texture System**: 2D texture loading and management via STB
- **OpenGL Context**: Cross-platform OpenGL setup and management

### Camera System

Flexible camera system with:

- **Abstract Camera Base**: Common functionality for all camera types
- **Orthographic Camera**: Perfect for 2D games and UI rendering
- **Perspective Camera**: 3D rendering with proper depth perception
- **Camera Controllers**: Input-driven camera movement and rotation
- **Matrix Management**: Automatic view/projection matrix calculation
- **Coordinate Conversion**: Screen-to-world and world-to-screen utilities

### ImGui Integration

Comprehensive UI system featuring:

- **ImGui Layer**: Seamless integration with the layer system
- **Docking Support**: Editor-style dockable windows
- **Event Handling**: Proper input capture and event filtering
- **Styling Support**: Custom themes and styling options
- **Debug Tools**: Built-in debugging and profiling interfaces

### Material System

Advanced material and texture management:

- **Texture Abstraction**: Platform-independent texture interface
- **STB Integration**: Support for PNG, JPG, BMP, TGA, and more formats
- **OpenGL Backend**: Optimized OpenGL texture implementation
- **Binding Management**: Automatic texture unit management
- **Format Support**: Various pixel formats and compression options

### Input System

Cross-platform input handling:

- **GLFW Integration**: Robust keyboard and mouse input
- **Event Generation**: Automatic conversion to engine events
- **Key Mapping**: Platform-independent key code system
- **Mouse Support**: Button, movement, and scroll wheel handling
- **Input Polling**: Both event-driven and polling-based input

### Window System

GLFW-based window abstraction featuring:

- Cross-platform window creation
- Event callback integration
- VSync control
- Window property management
- Native handle access for advanced use
- OpenGL context management

### Layer System

Hierarchical layer management with:

- **LayerStack**: Manages layer lifecycle and execution order
- **Layer**: Base class for game logic, UI, debug tools
- **Forward updates**: Layers update in push order
- **Reverse events**: Overlays receive events first
- **Timestep integration**: Frame-rate independent updates

### Event Queue

Features include:

- Priority-based processing
- Time-budgeted event handling
- Event statistics and profiling
- Overflow handling (drop or block)
- Thread-safe operations

### Logging Macros

```cpp
// Core engine logging
BE_CORE_TRACE("Detailed trace info");
BE_CORE_INFO("General information");
BE_CORE_WARN("Warning message");
BE_CORE_ERROR("Error occurred");
BE_CORE_CRITICAL("Critical failure");

// Client application logging
BE_TRACE("Client trace");
BE_INFO("Client info");
BE_WARN("Client warning");
BE_ERROR("Client error");
BE_CRITICAL("Client critical");

// Category-based logging
BE_LOG_CATEGORY(LogCategory::Events, info, "Event logged");
```

## 🛠️ Development Tools

### IDE Support

#### VS Code

- Install **C/C++** and **CMake Tools** extensions
- Configuration files included in `.vscode/`
- Press `Cmd+Shift+B` (macOS) or `Ctrl+Shift+B` (Windows/Linux) to build

#### Zed Editor

- Configuration in `.zed/settings.json` and `.zed/tasks.json`
- Use Command Palette → "Tasks: Run Task" → "Build Project"

### Language Server

- **clangd** configuration included (`.clangd`)
- Automatic compile commands generation
- Full IntelliSense support

## 🔗 Dependencies

BeEngine uses the following third-party libraries as git submodules:

- **[spdlog](https://github.com/gabime/spdlog)**: Fast C++ logging library
- **[GLFW](https://github.com/glfw/glfw)**: Cross-platform window and input handling
- **[GLM](https://github.com/g-truc/glm)**: OpenGL Mathematics library
- **[Dear ImGui](https://github.com/ocornut/imgui)**: Immediate mode GUI (docking branch)
- **[STB](https://github.com/nothings/stb)**: Single-file public domain libraries for image loading
- **[Glad](https://glad.dav1d.de/)**: OpenGL function loader

### Cloning with Submodules

```bash
# Clone with submodules
git clone --recursive https://github.com/MubeenSikandar/BeEngine.git

# Or if already cloned, initialize submodules
git submodule update --init --recursive
```

### Build Options

BeEngine supports both static and shared library builds:

```bash
# Build as static library (default)
cmake -DBE_BUILD_SHARED=OFF ..

# Build as shared library (DLL/dylib/so)
cmake -DBE_BUILD_SHARED=ON ..
```

## 📋 Requirements

### macOS

- macOS 15 Sequoia or later
- Xcode Command Line Tools
- CMake 3.31+
- Clang 18+ with C++23 support
- OpenGL 4.1+ (built into macOS)

### Windows

- Windows 11 (24H2 recommended)
- Visual Studio 2022 (v17.10+) or MinGW-w64
- Windows SDK 10.0.26100+
- CMake 3.31+
- OpenGL 4.1+ drivers (usually included with graphics drivers)

### Linux

- Modern distribution (Ubuntu 24.04 LTS, Fedora 40+)
- GCC 14+ or Clang 18+
- CMake 3.31+
- Build essentials and pthread
- OpenGL 4.1+ drivers
- X11 development libraries (`libx11-dev` on Ubuntu)

## 🎨 C++23 Features Used

- `std::format` for string formatting
- Enhanced `constexpr` support
- Improved template deduction
- Modern smart pointer utilities
- Atomic operations
- Chrono library for timing

## 🎮 Graphics Features

### OpenGL Support

- **OpenGL 4.1+** core profile for maximum compatibility
- **Glad loader** for modern OpenGL function loading
- **Vertex Array Objects** for efficient geometry management
- **Shader compilation** with error reporting and validation
- **Texture management** with multiple format support
- **Frame buffer objects** for render-to-texture capabilities

### Rendering Pipeline

- **Forward rendering** with depth testing
- **Immediate mode rendering** for simple geometry
- **Batch rendering** support for performance optimization
- **Wireframe mode** for debugging and visualization
- **Viewport management** with automatic resize handling

### Logging System

BeEngine features a comprehensive logging system built on spdlog:

### Features

- **Dual-logger architecture**: Separate Core and Client loggers
- **Multiple sinks**: Console, file, and rotating file output
- **Async logging**: Non-blocking I/O for performance
- **Category-based logging**: Organize logs by system (Core, Events, etc.)
- **Privacy mode**: Automatic redaction of sensitive data
- **Build-specific configs**: Different verbosity for Debug/Release/Distribution

### Log Files

Runtime logs are automatically created in the `logs/` directory:

- `BeEngine_YYYYMMDD_HHMMSS.log`: Core engine logs
- `App_YYYYMMDD_HHMMSS.log`: Client application logs

### Usage Examples

```cpp
// Engine logging (use in BeEngine code)
BE_CORE_TRACE("Detailed debug information");
BE_CORE_INFO("General information");
BE_CORE_WARN("Warning message");
BE_CORE_ERROR("Error occurred");
BE_CORE_CRITICAL("Critical failure");

// Client logging (use in your application)
BE_TRACE("Application trace");
BE_INFO("Application info");
BE_WARN("Application warning");
BE_ERROR("Application error");
BE_CRITICAL("Application critical");

// Category-based logging
BE_LOG_CATEGORY(LogCategory::Events, info, "Event processed: {}", event.ToString());
```

## 📊 Performance Considerations

- **Async logging** minimizes I/O impact on frame time
- **Event time budgeting** prevents event processing from blocking rendering
- **Priority queues** ensure critical events are processed first
- **Layer system** allows efficient update and event handling order
- **Timestep-based updates** provide frame-rate independent game logic
- **OpenGL state caching** reduces redundant state changes
- **Vertex Array Objects** minimize draw call overhead
- **Texture binding optimization** reduces GPU state changes
- **ImGui integration** with minimal performance impact
- **Compile-time optimizations** with LTO in Distribution builds
- **Platform-specific optimizations** (SIMD, cache alignment)

## 🏗️ Architecture Overview

### Rendering Architecture

```
Application Layer
    ↓
Layer System (Game Logic, UI, Debug)
    ↓
Renderer (High-level API)
    ↓
RendererAPI (Platform abstraction)
    ↓
OpenGL Implementation
    ↓
GLFW + OpenGL Context
```

### Event Flow

```
GLFW Input → Window Events → Event Queue → Layer Stack → Application
                                ↓
                         Event Subscribers
```

### Layer Stack Organization

```
┌─────────────────┐ ← Overlays (UI, Debug) - Events first
├─────────────────┤
├─────────────────┤ ← Regular Layers (Game Logic)
├─────────────────┤
└─────────────────┘ ← Base Layer - Updates first
```

## 🔐 Build Macros

```cpp
BE_PLATFORM_WINDOWS    // Windows platform
BE_PLATFORM_APPLE      // macOS platform
BE_PLATFORM_LINUX      // Linux platform

BE_DEBUG               // Debug build
BE_RELEASE             // Release build
BE_DIST                // Distribution build

BE_COMPILER_MSVC       // MSVC compiler
BE_COMPILER_GCC        // GCC compiler
BE_COMPILER_CLANG      // Clang compiler
```

## 📚 Additional Resources

- [C++23 Standard Reference](https://en.cppreference.com/w/cpp/23)
- [CMake Documentation](https://cmake.org/documentation/)
- [spdlog Documentation](https://github.com/gabime/spdlog)
- [Game Engine Architecture](https://www.gameenginebook.com/)

## 📄 License

See `LICENSE` file for details.

## 🔄 Version

Current version: **1.0.0** (See `VERSION_INFO.md` for detailed technology stack information)
