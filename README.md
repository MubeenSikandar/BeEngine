# BeEngine

A modern, cross-platform C++23 game engine with event-driven architecture and comprehensive logging system.

## Overview

BeEngine is a lightweight game engine built with modern C++ standards, featuring a robust event system, flexible logging infrastructure, and cross-platform support. The engine follows a shared library architecture, making it easy to integrate into client applications.

## 🚀 Technology Stack

- **C++ Standard**: C++23 (ISO/IEC 14882:2024)
- **Build System**: CMake 3.31+
- **Logging**: spdlog (integrated as submodule)
- **Architecture**: Shared library (DLL/dylib/so)

### Platform Support

| Platform    | SDK/Compiler                           | Deployment Target |
| ----------- | -------------------------------------- | ----------------- |
| **macOS**   | macOS 15 Sequoia, Clang 18+            | 26.1              |
| **Windows** | Windows 11 SDK 10.0.26100, MSVC 19.40+ | Windows 11 24H2   |
| **Linux**   | GCC 14+ or Clang 18+                   | Kernel 6.x        |

## 🎯 Core Features

### Event System

- **Priority-based event queue** with configurable capacity
- **Type-safe event dispatching** using templates
- **Event categories** (Application, Input, Keyboard, Mouse, Window, etc.)
- **Subscriber pattern** with priority levels
- **Event filtering** and blocking by type or category
- **Performance profiling** with statistics tracking
- **Time-budgeted processing** to prevent frame drops

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
│   │   └── Logs/               # Logging system
│   │       ├── Log.hpp         # Logger interface
│   │       └── LogConfig.hpp   # Configuration
│   ├── src/                    # Implementation files
│   │   ├── Application.cpp
│   │   ├── Event.cpp
│   │   ├── EventQueue.cpp
│   │   ├── Layer.cpp
│   │   ├── LayerStack.cpp
│   │   ├── Log.cpp
│   │   └── Window.cpp
│   └── vendor/                 # Third-party dependencies
│       ├── spdlog/            # Logging library (submodule)
│       └── GLFW/              # Window/input library (submodule)
├── Sandbox/                    # Example application
│   └── SandBoxApp.cpp         # Demo client
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
│   │   ├── BeEngine.dll/dylib/so
│   │   ├── Sandbox
│   │   └── logs/       # Runtime logs
│   └── Release/        # Release builds
│       ├── BeEngine.dll/dylib/so
│       ├── Sandbox
│       └── logs/
└── lib/                # Static libraries
    ├── libglfw3.a
    └── libspdlogd.a
```

### Platform-Specific Output

| Platform    | Library             | Executable    |
| ----------- | ------------------- | ------------- |
| **Windows** | `BeEngine.dll`      | `Sandbox.exe` |
| **macOS**   | `libBeEngine.dylib` | `Sandbox`     |
| **Linux**   | `libBeEngine.so`    | `Sandbox`     |

## 💻 Creating a Client Application

```cpp
#include "Application.hpp"
#include "EntryPoint.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Layers/Layer.hpp"
#include "Logs/Log.hpp"

// Custom game layer
class GameLayer : public BeEngine::Layer {
public:
    GameLayer() : Layer("GameLayer") {}

    void onAttach() override {
        BE_INFO("Game layer attached!");
    }

    void onUpdate(BeEngine::Timestep ts) override {
        // Update game logic here
        // Use ts.GetSeconds() for frame-rate independent updates
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
};

class MyGame : public BeEngine::Application {
public:
    MyGame() {
        BE_INFO("MyGame Started!");

        // Push your game layer
        PushLayer(std::make_shared<GameLayer>());

        // Subscribe to global events
        GetEventQueue().Subscribe(
            BeEngine::EventType::KeyPressed,
            [](BeEngine::Event& e) {
                auto& keyEvent = static_cast<BeEngine::KeyPressedEvent&>(e);
                if (keyEvent.GetKeyCode() == BeEngine::KeyCode::Escape) {
                    BE_WARN("Escape pressed - shutting down");
                    return true; // Consume the event
                }
                return false;
            }
        );
    }

    void OnEvent(BeEngine::Event& event) override {
        // Handle application-level events
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
- Configurable frame rate
- Automatic cleanup

### Window System

GLFW-based window abstraction featuring:

- Cross-platform window creation
- Event callback integration
- VSync control
- Window property management
- Native handle access for advanced use

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

## � Deqpendencies

BeEngine uses the following third-party libraries as git submodules:

- **[spdlog](https://github.com/gabime/spdlog)**: Fast C++ logging library
- **[GLFW](https://github.com/glfw/glfw)**: Cross-platform window and input handling

### Cloning with Submodules

```bash
# Clone with submodules
git clone --recursive https://github.com/MubeenSikandar/BeEngine.git

# Or if already cloned, initialize submodules
git submodule update --init --recursive
```

### CMake for imgui

```bash
cmake_minimum_required(VERSION 3.20)
project(imgui)

# ImGui core files
set(IMGUI_SOURCES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
)

# ImGui backends (GLFW + OpenGL3)
set(IMGUI_BACKENDS
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

# Create static library
add_library(imgui STATIC
    ${IMGUI_SOURCES}
    ${IMGUI_BACKENDS}
)

# Include directories
target_include_directories(imgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

# Link dependencies
find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC
    glfw
)

# Enable C++17 (ImGui requirement)
target_compile_features(imgui PUBLIC cxx_std_17)
```

## 📋 Requirements

### macOS

- macOS 15 Sequoia or later
- Xcode Command Line Tools
- CMake 3.31+
- Clang 18+ with C++23 support

### Windows

- Windows 11 (24H2 recommended)
- Visual Studio 2022 (v17.10+) or MinGW-w64
- Windows SDK 10.0.26100+
- CMake 3.31+

### Linux

- Modern distribution (Ubuntu 24.04 LTS, Fedora 40+)
- GCC 14+ or Clang 18+
- CMake 3.31+
- Build essentials and pthread

## 🎨 C++23 Features Used

- `std::format` for string formatting
- Enhanced `constexpr` support
- Improved template deduction
- Modern smart pointer utilities
- Atomic operations
- Chrono library for timing

## � Logging System

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
- **Compile-time optimizations** with LTO in Distribution builds
- **Platform-specific optimizations** (SIMD, cache alignment)

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
