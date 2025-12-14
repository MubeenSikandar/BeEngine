#pragma once

#include <csignal>

// PLATFORM DETECTION

// Operating System Detection
#if defined(_WIN32) || defined(_WIN64)
#define BE_PLATFORM_WINDOWS
#ifdef _WIN64
#define BE_PLATFORM_WIN64
#else
#define BE_PLATFORM_WIN32
#endif
#elif defined(__APPLE__) || defined(__MACH__)
#define BE_PLATFORM_APPLE
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define BE_PLATFORM_IOS
#elif TARGET_OS_MAC
#define BE_PLATFROM_MACOS
#endif
#elif defined(__ANDROID__)
#define BE_PLATFORM_ANDROID
#elif defined(__linux__)
#define BE_PLATFORM_LINUX
#elif defined(__unix__)
#define BE_PLATFORM_UNIX
#else
#error "Unknown Platform! Add Platform Support!"
#endif

// Compiler Detection
#if defined(_MSC_VER)
#define BE_COMPILER_MSVC
#define BE_COMPILER_VERSION _MSC_VER
#elif defined(__clang__)
#define BE_COMPILER_CLANG
#define BE_COMPILER_VERSION                                                    \
  (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#elif defined(__GNUC__) || defined(__GNUG__)
#define BE_COMPILER_GCC
#define BE_COMPILER_VERSION                                                    \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#warning "Unknown compiler"
#endif

// Build Configuration

// Debug Build Detection
#if defined(_DEBUG) || defined(DEBUG) || (!defined(NDEBUG))
#define BE_DEBUG 1
#define BE_BUILD_TYPE ("Debug")
#else
#define BE_RELEASE
#define BE_RELEASE_TYPE ("Release")
#endif

// Distribution Build (No Debugging, Optimized)
#ifdef BE_DIST
#undef BE_DEBUG
#undef BE_RELEASE
#define BE_BUILD_TYPE ("Distribution")
#endif

// DLL EXPORT/IMPORT
#ifdef BE_PLATFORM_WINDOWS
#ifdef BE_BUILD_DLL
#define BE_API __declspec(dllexport)
#else
#define BE_API __declspec(dllimport)
#endif
#else
#define BE_API __attribute__((visibility("default")))
#endif

// Architecture Detection
#if defined(_M_X64) || defined(__x86_64__) || defined(__amd64__)
#define BE_ARCH_X64
#define BE_ARCH_64BIT
#elif defined(_M_IX86) || defined(__i386__)
#define BE_ARCH_X86
#define BE_ARCH_32BIT
#elif defined(__arm__) || defined(_M_ARM)
#define BE_ARCH_ARM
#define BE_ARCH_32BIT
#elif defined(__aarch64__) || defined(_M_ARM64)
#define BE_ARCH_ARM64
#define BE_ARCH_64BIT
#else
#warning "Unknown Architecture"
#endif

// UTILITY MACROS
//  Bit Manipulation
#define BIT(x) (1 << (x))
#define BIT64(x) (1ULL << (x))

// Bind member functions for callbacks
#define BIND_EVENT_FN(fn)                                                      \
  [this](auto &&...args) -> decltype(auto) {                                   \
    return this->fn(std::forward<decltype(args)> args...);                     \
  }

// Force inline
#ifdef BE_COMPILER_MSVC
#define BE_FORCE_INLINE __forceinline
#elif defined(BE_COMPILER_CLANG) || defined(BE_COMPILER_GCC)
#define BE_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define BE_FORCE_INLINE inline
#endif

// No Inline
#ifdef BE_COMPILER_MSVC
#define BE_NO_INLINE __declspec(noinline)
#elif defined(BE_COMPILER_CLANG) || defined(BE_COMPILER_GCC)
#define BE_NO_INLINE __attribute__((noinline))
#else
#define BE_NO_INLINE
#endif

// Unreachable code hint
#ifdef BE_COMPILER_MSVC
#define BE_UNREACHABLE() __assume(0)
#elif defined(BE_COMPILER_CLANG) || defined(BE_COMPILER_GCC)
#define BE_UNREACHABLE() __builtin_unreachable()
#else
#define BE_UNREACHABLE()
#endif

// Likely/Unlikely branch predictions
#if defined(BE_COMPILER_CLANG) || defined(BE_COMPILER_GCC)
#define BE_LIKELY(x) __builtin_expect(!!(x), 1)
#define BE_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define BE_LIKELY(x) (x)
#define BE_UNLIKELY(x) (x)
#endif

// Deprecated warnings
#ifdef BE_COMPILER_MSVC
#define BE_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined(BE_COMPILER_CLANG) || defined(BE_COMPILER_GCC)
#define BE_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
#define BE_DEPRECATED(msg)
#endif

// Disable specific warnings
#ifdef BE_COMPILER_MSVC
#define BE_DISABLE_WARNING_PUSH __pragma(warning(push))
#define BE_DISABLE_WARNING_POP __pragma(warning(pop))
#define BE_DISABLE_WARNING(warningNumber)                                      \
  __pragma(warning(disable : warningNumber))

// Common warnings
#define BE_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER                       \
  BE_DISABLE_WARNING(4100)
#define BE_DISABLE_WARNING_UNREFERENCED_FUNCTION BE_DISABLE_WARNING(4505)
#elif defined(BE_COMPILER_CLANG) || defined(BE_COMPILER_GCC)
#define BE_DISABLE_WARNING_PUSH _Pragma("GCC diagnostic push")
#define BE_DISABLE_WARNING_POP _Pragma("GCC diagnostic pop")
#define BE_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER                       \
  _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"")
#define BE_DISABLE_WARNING_UNREFERENCED_FUNCTION                               \
  _Pragma("GCC diagnostic ignored \"-Wunused-function\"")
#else
#define BE_DISABLE_WARNING_PUSH
#define BE_DISABLE_WARNING_POP
#define BE_DISABLE_WARNING_UNREFERENCED_FORMAL_PARAMETER
#define BE_DISABLE_WARNING_UNREFERENCED_FUNCTION
#endif

// Assertions
#ifdef BE_DEBUG
#ifdef BE_PLATFORM_WINDOWS
#define BE_DEBUGBREAK() __debugbreak()
#elif defined(BE_PLATFORM_LINUX) || defined(BE_PLATFORM_APPLE)
#define BE_DEBUGBREAK() raise(SIGTRAP)
#else
#define BE_DEBUGBREAK()
#endif

// Enable assertions in debug
#define BE_ENABLE_ASSERTS
#else
#define BE_DEBUGBREAK()
#endif

// Assertions (defined in Log.hpp, declared here)
#ifdef BE_ENABLE_ASSERTS
#define BE_ASSERT(condition, ...)                                              \
  do {                                                                         \
    if (!(condition)) {                                                        \
      BE_CRITICAL("Assertion failed: " #condition " - " __VA_ARGS__);          \
      BE_DEBUGBREAK();                                                         \
    }                                                                          \
  } while (0)

#define BE_CORE_ASSERT(condition, ...)                                         \
  do {                                                                         \
    if (!(condition)) {                                                        \
      BE_CORE_CRITICAL("Assertion failed: " #condition " - " __VA_ARGS__);     \
      BE_DEBUGBREAK();                                                         \
    }                                                                          \
  } while (0)
#else
#define BE_ASSERT(condition, ...) ((void)0)
#define BE_CORE_ASSERT(condition, ...) ((void)0)
#endif

// Verify (always enabled, even in release)
#define BE_VERIFY(condition, ...)                                              \
  do {                                                                         \
    if (!(condition)) {                                                        \
      BE_CRITICAL("Verification failed: " #condition " - " __VA_ARGS__);       \
    }                                                                          \
  } while (0)

#define BE_CORE_VERIFY(condition, ...)                                         \
  do {                                                                         \
    if (!(condition)) {                                                        \
      BE_CORE_CRITICAL("Verification failed: " #condition " - " __VA_ARGS__);  \
    }                                                                          \
  } while (0)

// Memory Management
// Memory alignment
#define BE_ALIGN(x) __declspec(align(x))
#define BE_ALIGN_OF(x) __alignof(x)

// Cache line size (typically 64 bytes on modern CPUs)
#define BE_CACHE_LINE_SIZE 64
#define BE_CACHE_ALIGNED BE_ALIGN(BE_CACHE_LINE_SIZE)

// SMART POINTER HELPER
#include <memory>

namespace BeEngine {
// Scope - unique_ptr alias
template <typename T> using Scope = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr Scope<T> CreateScope(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

// Ref - shared_ptr alias
template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, typename... Args>
constexpr Ref<T> CreateRef(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}
} // namespace BeEngine

// BYTE SIZE LITERALS
constexpr size_t BYTE_SIZE{1024};
constexpr size_t operator"" _KB(unsigned long long kb) {
  return kb * BYTE_SIZE;
}

constexpr size_t operator"" _MB(unsigned long long mb) {
  return mb * BYTE_SIZE * BYTE_SIZE;
}

constexpr size_t operator"" _GB(unsigned long long gb) {
  return gb * BYTE_SIZE * BYTE_SIZE * BYTE_SIZE;
}

// VERSION INFORMATION

#define BE_VERSION_MAJOR 1
#define BE_VERSION_MINOR 0
#define BE_VERSION_PATCH 0
#define BE_VERSION_STRING "1.0.0"

// Profiling Helper
#ifdef BE_ENABLE_PROFILING
#define BE_PROFILE_SCOPE(name) /* Implement with your profiler */
#define BE_PROFILE_FUNCTION() BE_PROFILE_SCOPE(__FUNCTION__)
#else
#define BE_PROFILE_SCOPE(name)
#define BE_PROFILE_FUNCTION()
#endif

// Static Analysis
// Null pointer annotation
#ifdef BE_COMPILER_MSVC
#include <sal.h>
#define BE_NULLABLE _Maybenull_
#define BE_NONNULL _Notnull_
#else
#define BE_NULLABLE
#define BE_NONNULL
#endif

// DISALLOW COPY/MOVE
#define BE_DISALLOW_COPY(TypeName)                                             \
  TypeName(const TypeName &) = delete;                                         \
  (TypeName) &operator=(const TypeName &) = delete;

#define BE_DISALLOW_MOVE(TypeName)                                             \
  TypeName((TypeName) &&) = delete;                                            \
  (TypeName) &operator=((TypeName) &&) = delete;

#define BE_DISALLOW_COPY_AND_MOVE(TypeName)                                    \
  BE_DISALLOW_COPY(TypeName)                                                   \
  BE_DISALLOW_MOVE(TypeName)

// ARRAY SIZE
template <typename T, size_t N>
constexpr size_t BE_ARRAY_SIZE(const std::array<T, N> & /* arr */) {
  return N;
}

// STRINGIFY
#define BE_STRINGIFY_IMPL(x) #x
#define BE_STRINGIFY(x) BE_STRINGIFY_IMPL(x)

// CONCAT
#define BE_CONCAT_IMPL(x, y) x##y
#define BE_CONCAT(x, y) BE_CONCAT_IMPL(x, y)

// UNIQUE NAME GENERATOR
#define BE_UNIQUE_NAME(base) BE_CONCAT(base, __LINE__)

// BIT OPERATION
#define BIT(x) (1 << (x))

// NO DISCARD
#define NODISCARD [[nodiscard]]

// PLATFORM-SPECIFIC INCLUDES
#ifdef BE_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif
