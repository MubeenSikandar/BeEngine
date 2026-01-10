#pragma once
#include <Core.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <spdlog/common.h>
#include <string>
#include <unordered_map>

namespace BeEngine {
enum class LogCategory : std::uint8_t { Core, Events };

struct BE_API LogConfig {

  // Console Output
  bool enableConsole{true};
  spdlog::level::level_enum consoleLevel{spdlog::level::trace};

  // File Output
  bool enableFile{true};
  spdlog::level::level_enum fileLevel{spdlog::level::trace};
  std::string logDirectory{"logs/"};

  // Rotating File System
  static constexpr size_t BYTES_PER_KB{1024};
  static constexpr size_t MAX_FILE_SIZE_MB{5};
  static constexpr size_t MAX_FILES{10};

  bool enableRotating{true};
  size_t maxFileSize{BYTES_PER_KB * BYTES_PER_KB * MAX_FILE_SIZE_MB};
  size_t maxFiles{MAX_FILES};

  // Async Settings
  bool enableAsync{true};
  static constexpr size_t ASYNC_QUEUE_SIZE{8192};
  static constexpr size_t ASYNC_THREAD_COUNT{1};

  // Pattern
  std::string pattern{"[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] [thread %t] %v"};

  // Global Level
  spdlog::level::level_enum minLevel{spdlog::level::trace};

  // Platform Specific
  bool enablePlatformDebugger{true};

  // Performance
  static constexpr size_t MAX_MEMORY{50};

  bool enablePerformanceTracking{false};
  size_t maxMemoryUsage{BYTES_PER_KB * BYTES_PER_KB * MAX_MEMORY};

  static LogConfig Debug();
  static LogConfig Release();
  static LogConfig Distribution();
};

// Telemetry Event Structure
struct BE_API LogEvent {
  spdlog::level::level_enum level;
  std::string message;
  LogCategory category;
  std::chrono::system_clock::time_point timestamp;
  std::unordered_map<std::string, std::string> metadata;
  uint32_t threadId;
};

// Performance Statistics
struct BE_API LogStats {
  size_t totalLogsWritten{};
  size_t droppedLogs{};
  size_t bytesWritten{};
  double averageLogTimeMs{};
  size_t currentMemoryUsage{};
};

} // namespace BeEngine
