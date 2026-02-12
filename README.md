# BeEngine

A modern, cross-platform C++23 game engine with event-driven architecture, Entity-Component-System (ECS), comprehensive rendering pipeline, and advanced material system.

## Overview

BeEngine is a feature-rich game engine built with cutting-edge C++23 standards, featuring a robust ECS architecture, advanced rendering system with PBR materials, comprehensive scripting support via Lua, and cross-platform compatibility. The engine supports both 2D and 3D development with modern OpenGL 4.1+ rendering, glTF model loading, and a complete scene management system.

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
- **Model Loading**: tinygltf (glTF 2.0 support)
- **Scripting**: LuaJIT (embedded scripting engine)
- **JSON**: nlohmann/json (scene serialization)
- **Architecture**: Entity-Component-System (ECS) with Scene Management

### Platform Support

| Platform    | SDK/Compiler                           | Deployment Target |
| ----------- | -------------------------------------- | ----------------- |
| **macOS**   | macOS 15 Sequoia, Clang 18+            | 26.1              |
| **Windows** | Windows 11 SDK 10.0.26100, MSVC 19.40+ | Windows 11 24H2   |
| **Linux**   | GCC 14+ or Clang 18+                   | Kernel 6.x        |

## 🎯 Core Features

### Entity-Component-System (ECS)

- **Modern ECS Architecture** with efficient component storage
- **Scene Management** with hierarchical entity organization
- **Component System** including Transform, Tag, and custom components
- **Scene Serialization** with JSON-based save/load functionality
- **Entity Inspector** with real-time component editing via ImGui

### Advanced Rendering System

- **OpenGL 4.1+ pipeline** with modern shader support
- **PBR Material System** (Physically Based Rendering)
- **Multiple Material Types**: Unlit, Phong, and PBR materials
- **Advanced Lighting**: Directional, Point, and Spot lights with shadows
- **glTF 2.0 Model Loading** with full material and animation support
- **Mesh System** with built-in primitives (cube, sphere, plane, etc.)
- **Framebuffer System** for render-to-texture and post-processing
- **Vertex Array Objects (VAO)** and efficient buffer management
- **Shader Library** with hot-reloading and material binding

### Camera System

- **Dual Camera Support**: Orthographic (2D) and Perspective (3D)
- **Camera Controllers** with smooth input handling
- **Viewport Management** with automatic resize handling
- **Multiple projection modes** with seamless switching

### Scripting System

- **LuaJIT Integration** for high-performance scripting
- **Hot Reloading** of Lua scripts during development
- **Comprehensive API** exposing engine functionality to scripts
- **Script Registry** for organized script management
- **Type Definitions** for Lua with full engine binding

### Material System

- **Texture Management** with OpenGL backend
- **Material Library** for organized material storage
- **PBR Workflow** with metallic-roughness and specular-glossiness
- **Shader Uniforms** with automatic binding and validation
- **Material Inspector** with real-time property editing

### Event System

- **Priority-based event queue** with configurable capacity
- **Type-safe event dispatching** using templates
- **Event categories** (Application, Input, Keyboard, Mouse, Gamepad, etc.)
- **Subscriber pattern** with priority levels
- **Event filtering** and blocking by type or category
- **Gamepad Support** with full controller input handling

### Scene Management

- **Scene Graph** with hierarchical transforms
- **Scene Renderer** with optimized draw calls
- **Scene Serialization** to JSON format
- **Multiple Scene Support** with scene switching
- **Asset Management** integrated with scene system

### Input System

- **Cross-platform input handling** via GLFW
- **Keyboard, Mouse, and Gamepad** event processing
- **Input state management** with frame-based tracking
- **Gamepad Hot-plugging** with automatic detection
- **Input mapping** and customizable controls

### UI System (ImGui Integration)

- **Dear ImGui integration** with docking support
- **Editor-style interface** with dockable windows
- **Scene Inspector** for real-time entity/component editing
- **Material Editor** with live property updates
- **Performance Profiler** with frame time analysis
- **Input capture management** (mouse/keyboard)
- **Custom styling and theming** support

### Time System

- **Dual timestep support**: Variable (game logic) and Fixed (physics)
- **Time scaling** for slow-motion and pause effects
- **Frame rate independent updates** with delta time
- **Performance monitoring** with FPS tracking
- **Time accumulator** for consistent physics simulation

## 📁 Project Structure

```
BeEngine/
├── BeEngine/                    # Engine library
│   ├── include/
│   │   ├── Application.hpp      # Main application class
│   │   ├── Core.hpp            # Platform detection & macros
│   │   ├── EntryPoint.hpp      # Application entry point
│   │   ├── Window.hpp          # GLFW window abstraction
│   │   ├── BuiltInShaders/     # Shader library and lighting
│   │   │   ├── LightManager.hpp
│   │   │   ├── PBRMaterial.hpp
│   │   │   └── PhongMaterial.hpp
│   │   ├── Camera/             # Camera system
│   │   │   ├── Camera.hpp      # Base camera class
│   │   │   ├── OrthographicCamera.hpp
│   │   │   ├── PerspectiveCamera.hpp
│   │   │   └── *CameraController.hpp
│   │   ├── Codes/              # Input code definitions
│   │   │   ├── KeyCodes.hpp
│   │   │   ├── MouseCodes.hpp
│   │   │   └── GamepadCodes.hpp
│   │   ├── Events/             # Event system
│   │   │   ├── Event.hpp       # Base event class
│   │   │   ├── EventQueue.hpp  # Priority-based event queue
│   │   │   ├── ApplicationEvent.hpp
│   │   │   ├── KeyEvent.hpp
│   │   │   ├── MouseEvent.hpp
│   │   │   └── GamepadEvent.hpp
│   │   ├── Scene/              # ECS and Scene Management
│   │   │   ├── Scene.hpp       # Scene container
│   │   │   ├── SceneManager.hpp
│   │   │   ├── SceneRenderer.hpp
│   │   │   ├── SceneSerializer.hpp
│   │   │   └── ECS/            # Entity-Component-System
│   │   ├── Renderer/           # Rendering system
│   │   │   ├── Renderer.hpp    # High-level renderer
│   │   │   ├── RendererAPI.hpp # Abstract rendering API
│   │   │   ├── Shader.hpp      # Shader management
│   │   │   ├── VertexArray.hpp # VAO abstraction
│   │   │   ├── FrameBuffer.hpp # FBO abstraction
│   │   │   └── OpenGL/         # OpenGL implementation
│   │   ├── MaterialSystem/     # Material and texture system
│   │   │   ├── Material.hpp    # Base material class
│   │   │   ├── MaterialLibrary.hpp
│   │   │   ├── Texture.hpp     # Texture abstraction
│   │   │   └── OpenGL/         # OpenGL implementations
│   │   ├── MeshSystem/         # Mesh and geometry
│   │   │   ├── Mesh.hpp        # Mesh container
│   │   │   ├── MeshFactory.hpp # Built-in primitives
│   │   │   └── Vertex.hpp      # Vertex definitions
│   │   ├── ModelLoader/        # 3D model loading
│   │   │   ├── GLTFLoader.hpp  # glTF 2.0 loader
│   │   │   ├── Model.hpp       # Model container
│   │   │   └── ModelRenderer.hpp
│   │   ├── Scripting/          # Lua scripting system
│   │   │   ├── API/            # Engine API bindings
│   │   │   ├── Core/           # Script management
│   │   │   └── Lua/            # Lua-specific code
│   │   ├── Layers/             # Layer system
│   │   │   ├── Layer.hpp       # Base layer class
│   │   │   └── LayerStack.hpp  # Layer management
│   │   ├── ImGui/              # ImGui integration
│   │   │   └── ImGuiLayer.hpp  # ImGui layer
│   │   ├── Input/              # Input handling
│   │   │   └── Input.hpp       # Input abstraction
│   │   ├── Math/               # Mathematics utilities
│   │   │   └── Transform.hpp   # 3D transforms
│   │   ├── Time/               # Time management
│   │   │   ├── Time.hpp        # Time utilities
│   │   │   └── Timestep.hpp    # Frame timing
│   │   └── Logs/               # Logging system
│   │       ├── Log.hpp         # Logger interface
│   │       └── LogConfig.hpp   # Configuration
│   ├── src/                    # Implementation files
│   │   ├── Application.cpp
│   │   ├── Window.cpp
│   │   ├── Scene/              # Scene implementations
│   │   ├── Renderer/           # Rendering implementations
│   │   ├── MaterialSystem/     # Material implementations
│   │   ├── MeshSystem/         # Mesh implementations
│   │   ├── ModelLoader/        # Model loading implementations
│   │   ├── Scripting/          # Scripting implementations
│   │   └── [other systems]/    # Other implementations
│   └── vendor/                 # Third-party dependencies
│       ├── spdlog/            # Logging library (submodule)
│       ├── GLFW/              # Window/input library (submodule)
│       ├── GLM/               # Mathematics library (submodule)
│       ├── imgui/             # UI library (submodule)
│       ├── STB/               # Image loading (submodule)
│       ├── tinygltf/          # glTF loader (submodule)
│       ├── nlohmann_json/     # JSON library (submodule)
│       ├── LuaJIT/            # Lua scripting (submodule)
│       └── Glad/              # OpenGL loader
├── Sandbox/                    # Example application
│   ├── SandBoxApp.cpp         # Main demo application
│   ├── SandboxLayer2D.hpp     # 2D rendering demo
│   └── SandboxLayer3D.cpp     # 3D rendering demo
├── Assets/                     # Runtime assets
│   └── Scenes/                # Scene files
├── .vscode/                   # VS Code configuration
├── .zed/                      # Zed editor configuration
├── build/                     # Build output directory
├── logs/                      # Runtime log files
├── CMakeLists.txt             # Build configuration
├── CMakePresets.json          # CMake presets
├── build.sh                   # Unix build script
├── build.bat                  # Windows build script
└── .gitmodules               # Git submodule configuration
```

## 🔧 Building the Engine

### Prerequisites

Ensure you have the required tools installed:

- **CMake 3.31+**
- **C++23 compatible compiler**:
  - GCC 14+ (Linux)
  - Clang 18+ (macOS/Linux)
  - MSVC 19.40+ / Visual Studio 2022 v17.10+ (Windows)
- **Git** (for submodules)

### Quick Build

#### macOS/Linux

```bash
git clone --recursive https://github.com/YourUsername/BeEngine.git
cd BeEngine
./build.sh
./build/bin/Sandbox
```

#### Windows

```cmd
git clone --recursive https://github.com/YourUsername/BeEngine.git
cd BeEngine
build.bat
.\build\bin\Release\Sandbox.exe
```

### Manual Build

#### All Platforms

```bash
# Clone with submodules
git clone --recursive https://github.com/YourUsername/BeEngine.git
cd BeEngine

# Create build directory
mkdir build && cd build

# Configure (Debug by default)
cmake ..

# Or configure for Release
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build .

# Run
./bin/Sandbox  # Linux/macOS
.\bin\Release\Sandbox.exe  # Windows
```

### Build Options

```bash
# Static library (default)
cmake -DBE_BUILD_SHARED=OFF ..

# Shared library (DLL/dylib/so)
cmake -DBE_BUILD_SHARED=ON ..

# Build types
cmake -DCMAKE_BUILD_TYPE=Debug ..      # Debug build
cmake -DCMAKE_BUILD_TYPE=Release ..    # Release build
cmake -DCMAKE_BUILD_TYPE=Dist ..       # Distribution build (max optimization)
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

### Basic Application Setup

```cpp
#include <Application.hpp>
#include <EntryPoint.hpp>
#include <Scene/Scene.hpp>
#include <Scene/SceneManager.hpp>
#include <Camera/PerspectiveCameraController.hpp>
#include <Logs/Log.hpp>

class GameLayer : public BeEngine::Layer {
public:
    GameLayer() : Layer("GameLayer") {}

    void OnAttach() override {
        BE_INFO("Game layer attached!");

        // Initialize camera
        m_CameraController = BeEngine::CreateScope<BeEngine::PerspectiveCameraController>(
            16.0f / 9.0f, 45.0f, 0.1f, 1000.0f
        );

        // Create scene
        m_Scene = &m_SceneManager.CreateScene("Game Scene");

        // Setup your scene
        SetupScene();
    }

    void OnUpdate(BeEngine::Timestep ts) override {
        // Update camera
        if (m_ViewportFocused) {
            m_CameraController->OnUpdate(ts);
        }

        // Update scene
        if (m_Scene) {
            m_Scene->OnUpdate(ts.GetSeconds());
        }
    }

    void OnRender() override {
        if (!m_Scene) return;

        // Render scene
        BeEngine::SceneRenderer::BeginScene(m_CameraController->GetCamera());
        BeEngine::SceneRenderer::RenderScene(*m_Scene);
        BeEngine::SceneRenderer::EndScene();
    }

    void OnEvent(BeEngine::Event& event) override {
        m_CameraController->OnEvent(event);

        BeEngine::EventDispatcher dispatcher(event);
        dispatcher.Dispatch<BeEngine::KeyPressedEvent>(
            [](BeEngine::KeyPressedEvent& e) {
                if (e.GetKeyCode() == BeEngine::KeyCode::Escape) {
                    BeEngine::Application::Get().Close();
                    return true;
                }
                return false;
            }
        );
    }

    void OnImGuiRender() override {
        // Scene inspector
        ImGui::Begin("Scene Inspector");
        if (m_Scene) {
            ImGui::Text("Scene: %s", m_Scene->GetName().c_str());
            ImGui::Text("Entities: %zu", m_Scene->GetEntityCount());

            // Entity list
            for (BeEngine::Entity entity : m_Scene->GetAllEntities()) {
                auto& tag = entity.GetComponent<BeEngine::TagComponent>();
                if (ImGui::TreeNode(tag.Tag.c_str())) {
                    // Transform component editor
                    if (entity.HasComponent<BeEngine::TransformComponent>()) {
                        auto& transform = entity.GetComponent<BeEngine::TransformComponent>();

                        glm::vec3 pos = transform.GetPosition();
                        if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) {
                            transform.SetPosition(pos);
                        }

                        glm::vec3 rot = glm::degrees(transform.GetEulerAngles());
                        if (ImGui::DragFloat3("Rotation", &rot.x, 1.0f)) {
                            transform.SetRotation(glm::radians(rot));
                        }

                        glm::vec3 scale = transform.GetScale();
                        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f)) {
                            transform.SetScale(scale);
                        }
                    }
                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();
    }

private:
    void SetupScene() {
        // Create a cube entity
        BeEngine::Entity cube = m_Scene->CreateEntity("Cube");
        auto& transform = cube.GetComponent<BeEngine::TransformComponent>();
        transform.SetPosition({0.0f, 0.0f, -5.0f});

        // Add mesh component (you would implement this)
        // cube.AddComponent<BeEngine::MeshComponent>(BeEngine::MeshFactory::CreateCube());

        // Add material component
        // cube.AddComponent<BeEngine::MaterialComponent>(someMaterial);
    }

    BeEngine::SceneManager m_SceneManager;
    BeEngine::Scene* m_Scene = nullptr;
    BeEngine::Scope<BeEngine::PerspectiveCameraController> m_CameraController;
    bool m_ViewportFocused = false;
};

class MyGame : public BeEngine::Application {
public:
    MyGame() {
        BE_INFO("MyGame Started!");

        // Push your game layer
        PushLayer(BeEngine::CreateRef<GameLayer>());

        // Push ImGui layer for debug UI
        PushOverlay(BeEngine::CreateRef<BeEngine::ImGuiLayer>());
    }
};

// Entry point
BeEngine::Application* BeEngine::CreateApplication() {
    return new MyGame();
}
```

### Advanced Features

#### Loading glTF Models

```cpp
#include <ModelLoader/GLTFLoader.hpp>
#include <ModelLoader/ModelRenderer.hpp>

void LoadModel() {
    BeEngine::GLTFLoader loader;
    auto model = loader.LoadModel("assets/models/character.gltf");

    if (model) {
        // Create entity for the model
        BeEngine::Entity modelEntity = m_Scene->CreateEntity("Character");

        // Add model component (you would implement this)
        // modelEntity.AddComponent<BeEngine::ModelComponent>(model);

        // Position the model
        auto& transform = modelEntity.GetComponent<BeEngine::TransformComponent>();
        transform.SetPosition({0.0f, 0.0f, 0.0f});
        transform.SetScale({1.0f, 1.0f, 1.0f});
    }
}
```

#### Using Materials

```cpp
#include <MaterialSystem/PBRMaterial.hpp>
#include <MaterialSystem/MaterialLibrary.hpp>

void SetupMaterials() {
    auto& materialLib = BeEngine::MaterialLibrary::Get();

    // Create PBR material
    auto pbrMaterial = BeEngine::CreateRef<BeEngine::PBRMaterial>();
    pbrMaterial->SetAlbedo({0.8f, 0.2f, 0.2f, 1.0f});
    pbrMaterial->SetMetallic(0.0f);
    pbrMaterial->SetRoughness(0.5f);

    materialLib.AddMaterial("RedPBR", pbrMaterial);
}
```

#### Lua Scripting

```cpp
#include <Scripting/Core/ScriptEngineManager.hpp>

void SetupScripting() {
    auto& scriptEngine = BeEngine::ScriptEngineManager::Get();

    // Load and execute a Lua script
    scriptEngine.LoadScript("player_controller.lua");

    // Call Lua function from C++
    scriptEngine.CallFunction("OnPlayerUpdate", deltaTime);
}
```

## 🔍 Key Components

### Application Class

The base application class provides:

- **Main game loop** with fixed and variable timestep support
- **Window creation and management** via GLFW
- **Layer stack management** with overlays
- **Event queue management** with priority handling
- **Scene management** integration
- **Automatic cleanup** and resource management

### Entity-Component-System (ECS)

Modern ECS architecture featuring:

- **Entity**: Lightweight ID with component storage
- **Components**: Data-only structures (Transform, Tag, Mesh, Material, etc.)
- **Systems**: Logic processors that operate on component data
- **Scene**: Container for entities with spatial organization
- **Serialization**: JSON-based scene save/load functionality

### Advanced Rendering Pipeline

Comprehensive rendering system with:

- **Scene Renderer**: High-level scene rendering with culling
- **Material System**: PBR, Phong, and Unlit material support
- **Lighting System**: Directional, Point, and Spot lights
- **Shadow Mapping**: Real-time shadow generation
- **Post-Processing**: Framebuffer-based effects pipeline
- **Mesh System**: Built-in primitives and custom mesh support
- **Model Loading**: Full glTF 2.0 support with animations

### Camera System

Flexible camera system with:

- **Dual Camera Types**: Orthographic (2D) and Perspective (3D)
- **Camera Controllers**: Input-driven movement with customizable controls
- **Smooth Interpolation**: Frame-rate independent camera movement
- **Viewport Management**: Automatic aspect ratio and resize handling
- **Coordinate Conversion**: Screen-to-world and world-to-screen utilities

### Scripting Integration

Powerful Lua scripting with:

- **LuaJIT Engine**: High-performance Lua execution
- **Hot Reloading**: Live script updates during development
- **Engine API**: Full access to engine functionality from Lua
- **Type Safety**: Comprehensive type definitions and error handling
- **Script Registry**: Organized script management and loading

### Material System

Advanced material pipeline featuring:

- **PBR Workflow**: Physically Based Rendering with metallic-roughness
- **Multiple Material Types**: Unlit, Phong, and PBR materials
- **Texture Management**: Efficient texture loading and binding
- **Shader Library**: Organized shader storage with hot-reloading
- **Material Editor**: Real-time property editing via ImGui
- **Uniform Binding**: Automatic shader uniform management

### Logging System

BeEngine features a comprehensive logging system built on spdlog:

#### Features

- **Dual-logger architecture**: Separate Core and Client loggers
- **Multiple sinks**: Console, file, and rotating file output
- **Async logging**: Non-blocking I/O for performance
- **Category-based logging**: Organize logs by system (Core, Events, etc.)
- **Privacy mode**: Automatic redaction of sensitive data
- **Build-specific configs**: Different verbosity for Debug/Release/Distribution

#### Log Files

Runtime logs are automatically created in the `logs/` directory:

- `BeEngine_YYYYMMDD_HHMMSS.log`: Core engine logs
- `App_YYYYMMDD_HHMMSS.log`: Client application logs

#### Usage Examples

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
- **[tinygltf](https://github.com/syoyo/tinygltf)**: Header-only glTF 2.0 loader and writer
- **[nlohmann/json](https://github.com/nlohmann/json)**: Modern C++ JSON library
- **[LuaJIT](https://luajit.org/)**: High-performance Lua scripting engine
- **[Glad](https://glad.dav1d.de/)**: OpenGL function loader

### Cloning with Submodules

```bash
# Clone with submodules
git clone --recursive https://github.com/YourUsername/BeEngine.git

# Or if already cloned, initialize submodules
git submodule update --init --recursive
```

### Library Configuration

BeEngine supports both static and shared library builds:

```bash
# Build as static library (default)
cmake -DBE_BUILD_SHARED=OFF ..

# Build as shared library (DLL/dylib/so)
cmake -DBE_BUILD_SHARED=ON ..
```

## 📋 Requirements

### macOS

- **macOS 15 Sequoia** or later
- **Xcode Command Line Tools** or Xcode 15+
- **CMake 3.31+**
- **Clang 18+** with C++23 support
- **OpenGL 4.1+** (built into macOS)

### Windows

- **Windows 11** (24H2 recommended)
- **Visual Studio 2022** (v17.10+) with C++23 support
- **Windows SDK 10.0.26100+**
- **CMake 3.31+**
- **OpenGL 4.1+** drivers (usually included with graphics drivers)

### Linux

- **Modern distribution** (Ubuntu 24.04 LTS, Fedora 40+)
- **GCC 14+** or **Clang 18+** with C++23 support
- **CMake 3.31+**
- **Build essentials** and pthread
- **OpenGL 4.1+** drivers
- **X11 development libraries** (`libx11-dev` on Ubuntu)

### Development Tools

- **Git** (for submodules)
- **VS Code** with C/C++ and CMake Tools extensions (recommended)
- **Zed Editor** with built-in C++ support (alternative)
- **clangd** for language server support

## 🎨 C++23 Features Used

BeEngine leverages modern C++23 features for performance and developer experience:

- **`std::print()` and `std::println()`** for formatted output
- **Enhanced `constexpr`** support for compile-time evaluation
- **`std::expected`** for error handling without exceptions
- **Improved template deduction** and concepts
- **Multidimensional subscript operator** for matrix operations
- **`if consteval`** for compile-time branching
- **Deducing `this`** for CRTP simplification
- **Modern smart pointer utilities** with automatic resource management
- **Atomic operations** for thread-safe event processing
- **Chrono library** enhancements for precise timing

## 🎮 Graphics Features

### OpenGL Support

- **OpenGL 4.1+** core profile for maximum compatibility
- **Glad loader** for modern OpenGL function loading
- **Vertex Array Objects** for efficient geometry management
- **Advanced shader pipeline** with uniform buffer objects
- **Texture management** with multiple format support
- **Framebuffer objects** for render-to-texture and post-processing

### Rendering Pipeline

- **Deferred rendering** support for complex lighting
- **Forward rendering** with depth testing and culling
- **PBR material workflow** with metallic-roughness model
- **Shadow mapping** with cascaded shadow maps
- **Post-processing effects** via framebuffer pipeline
- **Instanced rendering** for performance optimization
- **Wireframe and debug modes** for development

### Model Support

- **glTF 2.0** complete support including:
  - Meshes with multiple primitives
  - Materials (PBR metallic-roughness)
  - Textures and samplers
  - Animations and skinning
  - Scene hierarchy
- **Built-in primitives** (cube, sphere, plane, cylinder)
- **Custom mesh loading** with vertex attribute flexibility

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

- **ECS Architecture** provides cache-friendly component iteration
- **Scene Culling** reduces unnecessary draw calls
- **Instanced Rendering** for repeated geometry
- **Async logging** minimizes I/O impact on frame time
- **Event time budgeting** prevents event processing from blocking rendering
- **Priority queues** ensure critical events are processed first
- **Layer system** allows efficient update and event handling order
- **Timestep-based updates** provide frame-rate independent game logic
- **OpenGL state caching** reduces redundant state changes
- **Vertex Array Objects** minimize draw call overhead
- **Texture binding optimization** reduces GPU state changes
- **Material batching** groups similar materials for efficiency
- **Shader hot-reloading** without performance impact in release builds
- **Memory pooling** for frequent allocations (entities, components)
- **Spatial partitioning** for efficient collision detection and rendering
- **Level-of-detail (LOD)** system for distance-based optimization

## 🏗️ Architecture Overview

### Core Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      Application                            │
├─────────────────────────────────────────────────────────────┤
│  SceneManager │ LayerStack │ EventQueue │ Renderer │ Time  │
├─────────────────────────────────────────────────────────────┤
│     Scene     │    ECS     │   Camera   │ Material │ Input │
├─────────────────────────────────────────────────────────────┤
│     GLFW      │   ImGui    │  OpenGL    │  spdlog  │ Lua   │
├─────────────────────────────────────────────────────────────┤
│                    Operating System                          │
└─────────────────────────────────────────────────────────────┘
```

### ECS Data Flow

```
Entities ──┐
           ├──► Scene ──► Systems ──► Renderer ──► GPU
Components ┘
```

### Event Flow

```
Input (GLFW) → Events → Event Queue → Layers → Application
                 ↓
            Subscribers (Systems, UI, Scripts)
```

### Rendering Pipeline

```
Scene → Culling → Material Sorting → Draw Calls → Post-Processing → Present
```

## 🔐 Build Macros

```cpp
// Platform Detection
BE_PLATFORM_WINDOWS    // Windows platform
BE_PLATFORM_APPLE      // macOS platform
BE_PLATFORM_LINUX      // Linux platform

// Build Configuration
BE_DEBUG               // Debug build
BE_RELEASE             // Release build
BE_DIST                // Distribution build

// Compiler Detection
BE_COMPILER_MSVC       // MSVC compiler
BE_COMPILER_GCC        // GCC compiler
BE_COMPILER_CLANG      // Clang compiler

// Library Configuration
BE_BUILD_DLL           // Building as shared library
BE_DYNAMIC_LINK        // Using shared library
```

## 🚀 Getting Started

### 1. Clone and Build

```bash
git clone --recursive https://github.com/YourUsername/BeEngine.git
cd BeEngine
./build.sh  # or build.bat on Windows
```

### 2. Run the Demo

```bash
./build/bin/Sandbox  # Linux/macOS
.\build\bin\Release\Sandbox.exe  # Windows
```

### 3. Explore the Code

- Check out `Sandbox/SandBoxApp.cpp` for a complete example
- Look at `Sandbox/SandboxLayer3D.cpp` for 3D scene setup
- Examine `Sandbox/SandboxLayer2D.cpp` for 2D rendering

### 4. Create Your Own Project

Use the client application template above as a starting point for your own game or application.

## 📚 Additional Resources

- **[BeEngine Technical Documentation](BeEngine%20Technical%20Documentation.md)**: Comprehensive technical details
- **[VERSION_INFO.md](VERSION_INFO.md)**: Technology stack and version information
- **[C++23 Standard Reference](https://en.cppreference.com/w/cpp/23)**: C++23 language features
- **[CMake Documentation](https://cmake.org/documentation/)**: Build system reference
- **[OpenGL 4.1 Reference](https://www.khronos.org/opengl/wiki/OpenGL_4.1)**: Graphics API documentation
- **[glTF 2.0 Specification](https://github.com/KhronosGroup/glTF/tree/master/specification/2.0)**: 3D model format
- **[Dear ImGui Documentation](https://github.com/ocornut/imgui/wiki)**: UI framework guide

## 📄 License

See `LICENSE` file for details.

## 🔄 Version

Current version: **1.0.0** (See `VERSION_INFO.md` for detailed technology stack information)

---

**BeEngine** - Modern C++23 Game Engine with ECS, PBR Rendering, and Lua Scripting
