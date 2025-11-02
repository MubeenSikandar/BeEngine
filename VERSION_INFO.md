# BeEngine Technology Stack - Latest Versions

Last Updated: November 2024

## Core Technologies

### C++ Standard
- **Version**: C++23 (ISO/IEC 14882:2024)
- **Published**: October 2024
- **Status**: Current stable standard
- **Macro**: `__cplusplus` = `202302L`
- **Next**: C++26 (expected 2026)

**Key C++23 Features Used**:
- `std::print()` and `std::println()`
- Improved `constexpr`
- `std::expected` for error handling
- Multidimensional subscript operator
- `if consteval`
- Deducing `this`
- Extended floating-point types

### Build System
- **CMake**: 4.1.2 (October 2025)
- **Minimum**: 3.31 (November 2024)
- **Download**: https://cmake.org/download/

### Platform SDKs

#### macOS
- **Version**: macOS 15 Sequoia
- **Released**: September 16, 2024
- **Deployment Target**: 15.0
- **Next**: macOS 26 Tahoe (2025)
- **Features**: Apple Intelligence, iPhone Mirroring

#### Windows
- **SDK Version**: 10.0.26100 (Windows 11 24H2)
- **Released**: September 2024
- **Target OS**: Windows 11 version 24H2
- **Backward Compatible**: Windows 10 1809+
- **Download**: https://developer.microsoft.com/windows/downloads/windows-sdk/

#### Linux
- **Kernel**: 6.x (stable)
- **Distribution**: Any modern distro (Ubuntu 24.04 LTS, Fedora 40+)

## Compiler Support

### GCC
- **Minimum**: GCC 14
- **C++23 Support**: Full (as of GCC 14)
- **Released**: May 2024
- **Flag**: `-std=c++23`

### Clang
- **Minimum**: Clang 18
- **C++23 Support**: Full (as of Clang 18)
- **Released**: March 2024
- **Flag**: `-std=c++23`

### MSVC (Microsoft Visual C++)
- **Version**: MSVC 19.40+ (Visual Studio 2022 v17.10+)
- **C++23 Support**: Partial (improving with each release)
- **Flag**: `/std:c++latest`
- **Status**: Most C++23 features available

## Development Tools

### Editors & IDEs
- **VS Code**: Latest (with C/C++ and CMake Tools extensions)
- **Zed Editor**: Latest (built-in C++ and CMake support)
- **Visual Studio**: 2022 v17.10+
- **CLion**: 2024.2+

### Language Server
- **Clangd**: Version 18+ (for IntelliSense)
- **Configuration**: `.clangd` file included

## Recommended Versions for Production

```yaml
minimum_requirements:
  cmake: "3.31"
  cpp_standard: "23"

  compilers:
    gcc: "14.0"
    clang: "18.0"
    msvc: "19.40"

  platforms:
    macos: "15.0"  # Sequoia
    windows_sdk: "10.0.26100"
    linux_kernel: "6.0"
```

## Version Checking Commands

```bash
# Check CMake version
cmake --version

# Check GCC version
g++ --version

# Check Clang version
clang++ --version

# Check C++ standard support
echo | g++ -dM -E -std=c++23 - | grep __cplusplus

# Check macOS version
sw_vers

# Check Windows SDK (PowerShell)
Get-Item "HKLM:\SOFTWARE\Microsoft\Microsoft SDKs\Windows" | Get-ItemProperty -Name CurrentVersion
```

## Migration from Older Versions

### From C++17 to C++23
- Update `CMAKE_CXX_STANDARD` to `23`
- Review new features: modules, ranges improvements, `std::print()`
- Update deprecated features

### From CMake 3.x to 4.x
- Minimum version now 3.5 (C++11 minimum removed)
- Review breaking changes in CMake 4.0
- Update generator expressions if needed

## Useful Links

- **C++23 Standard**: https://en.cppreference.com/w/cpp/23
- **CMake 4.x Docs**: https://cmake.org/cmake/help/latest/
- **GCC C++23**: https://gcc.gnu.org/projects/cxx-status.html
- **Clang C++23**: https://clang.llvm.org/cxx_status.html
- **MSVC C++23**: https://learn.microsoft.com/en-us/cpp/overview/visual-cpp-language-conformance

## Notes

- **C++23** is the current standard (finalized February 2023, published 2024)
- **C++26** is the next standard (expected 2026)
- All versions listed are the **latest stable releases** as of November 2024
- For maximum compatibility, test on all target platforms
- Keep compilers and tools updated for best C++23 support
