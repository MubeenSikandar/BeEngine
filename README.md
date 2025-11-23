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

## � Building the Engine

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

## 💻 Creating a Client Application

```cpp
#include "Application.hpp"
#include "EntryPoint.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Logs/Log.hpp"

class MyGame : public BeEngine::Application {
public:
    MyGame() {
        BE_INFO("MyGame Started!");

        // Subscribe to events
        GetEventQueue().Subscribe(
            BeEngine::EventType::KeyPressed,
            [](BeEngine::Event& e) {
                auto& keyEvent = static_cast<BeEngine::KeyPressedEvent&>(e);
                BE_INFO("Key pressed: {}", BeEngine::ToString(keyEvent.GetKeyCode()));
                return false;
            }
        );
    }

    void OnEvent(BeEngine::Event& event) override {
        // Handle events
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
- Event queue management
- Configurable frame rate
- Automatic cleanup

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

## 📊 Performance Considerations

- **Async logging** minimizes I/O impact on frame time
- **Event time budgeting** prevents event processing from blocking rendering
- **Priority queues** ensure critical events are processed first
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
