#pragma once
#include "../Core.hpp"
#include "LogConfig.hpp"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include <memory>
#include <mutex>
#include <regex>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace BeEngine {
class BE_API Log {
public:
  // Initialization
  static void init(const LogConfig &config = LogConfig::Debug());
  static void Shutdown();
  static bool IsInitialized() { return s_Initialized.load(); }

  // Logger Access
  static std::shared_ptr<spdlog::logger> &GetCoreLogger();
  static std::shared_ptr<spdlog::logger> &GetClientLogger();
  static std::shared_ptr<spdlog::logger> &GetLogger(LogCategory category);

  // Runtime Configuration
  static void SetGlobalLevel(spdlog::level::level_enum level);
  static void SetCategoryLevel(LogCategory category,
                               spdlog::level::level_enum level);

  // Crash Handling
  static void Flush();
  static void FlushOn(spdlog::level::level_enum level);

  // Performance and Stats
  static LogStats GetStats();
  static void ResetStats();

  // Privacy and Security
  static void
  AddRedactionPattern(const std::regex &pattern,
                      const std::string &replacement = "[REDACTED]");
  static void SetPrivacyMode(bool enabled);

  static void FallbackLog(spdlog::level::level_enum level, const char *message);

private:
  // Internal Helpers
  static void CreateSinks(const LogConfig &config,
                          std::vector<spdlog::sink_ptr> &coreSinks,
                          std::vector<spdlog::sink_ptr> &clientSinks);
  static void CreateConsoleSinks(std::vector<spdlog::sink_ptr> &sinks,
                                 const LogConfig &config);
  static void CreateFileSinks(std::vector<spdlog::sink_ptr> &coreSinks,
                              std::vector<spdlog::sink_ptr> &clientSinks,
                              const LogConfig &config);
  static std::string GetTimestamps();
  static std::string RedactMessage(const std::string &message);
  static std::string CategoryToString(LogCategory category);

  // Static Members
  static std::atomic<bool> s_Initialized;
  static std::shared_ptr<spdlog::logger> s_CoreLogger;
  static std::shared_ptr<spdlog::logger> s_ClientLogger;
  static std::unordered_map<LogCategory, std::shared_ptr<spdlog::logger>>
      s_CategoryLogger;

  static LogConfig s_Config;
  static LogStats s_Stats;
  static std::mutex s_StatsMutex;

  static std::vector<std::pair<std::regex, std::string>> s_RedactionPatterns;
  static std::mutex s_RedactionMutex;
  static bool s_PrivacyMode;
};

} // namespace BeEngine

// Conditional Compilation
#ifdef BE_DEBUG
#define BE_LOGGING_ENABLED 1
#define BE_TRACE_ENABLED 1
#elif BE_RELEASE
#define BE_LOGGING_ENABLED 1
#define BE_TRACE_ENABLED 0
#else
#define BE_LOGGING_ENABLED 1
#define BE_TRACE_ENABLED 0
#endif

// CORE LOGGING MACROS
#if BE_LOGGING_ENABLED

#define BE_LOG_CHECK()                                                         \
  (::BeEngine::Log::IsInitialized()                                            \
       ? true                                                                  \
       : (::BeEngine::Log::FallbackLog(spdlog::level::err,                     \
                                       "Logging not initialized!"),            \
          false))

// Core Logger Macro
#define BE_CORE_TRACE(...)                                                     \
  do {                                                                         \
    if (BE_TRACE_ENABLED && BE_LOG_CHECK())                                    \
      ::BeEngine::Log::GetCoreLogger()->trace(__VA_ARGS__);                    \
  } while (0)

#define BE_CORE_INFO(...)                                                      \
  do {                                                                         \
    if (BE_LOG_CHECK())                                                        \
      ::BeEngine::Log::GetCoreLogger()->info(__VA_ARGS__);                     \
  } while (0)

#define BE_CORE_WARN(...)                                                      \
  do {                                                                         \
    if (BE_LOG_CHECK())                                                        \
      ::BeEngine::Log::GetCoreLogger()->warn(__VA_ARGS__);                     \
  } while (0);

#define BE_CORE_ERROR(...)                                                     \
  do {                                                                         \
    if (BE_LOG_CHECK())                                                        \
      ::BeEngine::Log::GetCoreLogger()->error(__VA_ARGS__);                    \
  } while (0)

#define BE_CORE_CRITICAL(...)                                                  \
  do {                                                                         \
    if (BE_LOG_CHECK()) {                                                      \
      ::BeEngine::Log::GetCoreLogger()->critical(__VA_ARGS__);                 \
      ::BeEngine::Log::Flush();                                                \
    }                                                                          \
  } while (0)

// Client Logger Macro
#define BE_TRACE(...)                                                          \
  do {                                                                         \
    if (BE_TRACE_ENABLED && BE_LOG_CHECK())                                    \
      ::BeEngine::Log::GetClientLogger()->trace(__VA_ARGS__);                  \
  } while (0)

#define BE_INFO(...)                                                           \
  do {                                                                         \
    if (BE_LOG_CHECK())                                                        \
      ::BeEngine::Log::GetClientLogger()->info(__VA_ARGS__);                   \
  } while (0)

#define BE_WARN(...)                                                           \
  do {                                                                         \
    if (BE_LOG_CHECK())                                                        \
      ::BeEngine::Log::GetClientLogger()->warn(__VA_ARGS__);                   \
  } while (0)

#define BE_ERROR(...)                                                          \
  do {                                                                         \
    if (BE_LOG_CHECK())                                                        \
      ::BeEngine::Log::GetClientLogger()->error(__VA_ARGS__);                  \
  } while (0)

#define BE_CRITICAL(...)                                                       \
  do {                                                                         \
    if (BE_LOG_CHECK()) {                                                      \
      ::BeEngine::Log::GetClientLogger()->critical(__VA_ARGS__);               \
      ::BeEngine::Log::Flush();                                                \
    }                                                                          \
  } while (0)

// Category-based logging
#define BE_LOG_CATEGORY(category, level, ...)                                  \
  do {                                                                         \
    if (BE_LOG_CHECK())                                                        \
      ::BeEngine::Log::GetLogger(category)->level(__VA_ARGS__);                \
  } while (0)

// Event logging (uses Events category)
#define BE_LOG_EVENT(event)                                                    \
  BE_LOG_CATEGORY(::BeEngine::LogCategory::Events, trace, "Event: {}",         \
                  (event).ToString())

#else
  // Logging disabled
#define BE_CORE_TRACE(...) ((void)0)
#define BE_CORE_INFO(...) ((void)0)
#define BE_CORE_WARN(...) ((void)0)
#define BE_CORE_ERROR(...) ((void)0)
#define BE_CORE_CRITICAL(...) ((void)0)
#define BE_TRACE(...) ((void)0)
#define BE_INFO(...) ((void)0)
#define BE_WARN(...) ((void)0)
#define BE_ERROR(...) ((void)0)
#define BE_CRITICAL(...) ((void)0)
#define BE_LOG_CATEGORY(category, level, ...) ((void)0)
#define BE_LOG_EVENT(event) ((void)0)
#endif
