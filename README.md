# GameEngine - Minimal Setup

Bare minimum cross-platform game engine setup with **latest technology standards**.

## 🚀 Technology Stack

- **C++ Standard**: C++23 (ISO/IEC 14882:2024)
- **Build System**: CMake 4.1.2+
- **macOS**: macOS 15 Sequoia (or later)
- **Windows**: Windows 11 SDK (Build 10.0.26100)
- **Linux**: GCC 14+ or Clang 18+

## 📁 Structure
```
GameEngine/
├── BeEngine/
│   ├── test.hpp        # Engine header with DLL export macros
│   └── test.cpp        # Engine implementation
├── Sandbox/
│   └── main.cpp        # Test application
└── CMakeLists.txt      # Build configuration
```

## 🔧 Build Instructions

### macOS/Linux
```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/Sandbox
```

### Windows (Visual Studio)
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
.\bin\Release\Sandbox.exe
```

### Windows (MinGW)
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
.\bin\Sandbox.exe
```

## 📦 Output

Platform-specific library and executable:

| Platform | Library | Executable |
|----------|---------|------------|
| **Windows** | `BeEngine.dll` | `Sandbox.exe` |
| **macOS** | `libBeEngine.dylib` | `Sandbox` |
| **Linux** | `libBeEngine.so` | `Sandbox` |

All files in `build/bin/` directory.

## 💻 IDE Support

### VS Code
- Install: **C/C++** and **CMake Tools** extensions
- Press `Cmd+Shift+B` to build
- Press `F5` to debug

### Zed Editor
- Open project: `zed .`
- Press `Cmd+Shift+P` → "Tasks: Run Task" → "Build Project"
- Or use terminal: `./build.sh`

## 🎯 Features

✅ **Cross-platform** (Windows 11, macOS 15+, Linux)
✅ **Modern C++23** with latest language features
✅ **Shared library** architecture (.dll/.dylib/.so)
✅ **Latest SDKs** (Windows 11 SDK, macOS Sequoia)
✅ **Proper DLL export/import** macros
✅ **CMake 4.1+** build system
✅ **Clangd LSP** support for IntelliSense
✅ **Both VS Code and Zed** editor configurations

## 📋 Requirements

### macOS
- macOS 15 Sequoia or later
- Xcode Command Line Tools
- CMake 4.1.2+
- Clang with C++23 support

### Windows
- Windows 11 (recommended)
- Visual Studio 2022 or MinGW-w64
- Windows SDK 10.0.26100+
- CMake 4.1.2+

### Linux
- GCC 14+ or Clang 18+
- CMake 4.1.2+
- Build essentials

## 🔍 Compiler Support

- **Clang 18+**: Full C++23 support
- **GCC 14+**: Full C++23 support
- **MSVC 2022**: Partial C++23 support (use latest version)

## 📚 Learn More

- [C++23 Standard](https://en.cppreference.com/w/cpp/23)
- [CMake Documentation](https://cmake.org/documentation/)
- [Game Engine Architecture](https://www.gameenginebook.com/)
