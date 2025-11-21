// Log.cpp
#include "../include/Logs/Log.hpp"
#include "../include/Logs/LogConfig.hpp"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

namespace BeEngine {

// Static Member Initialization
std::atomic<bool> Log::s_Initialized{false};
std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
std::unordered_map<LogCategory, std::shared_ptr<spdlog::logger>>
    Log::s_CategoryLogger;

LogConfig Log::s_Config;
LogStats Log::s_Stats;
std::mutex Log::s_StatsMutex;

std::vector<std::pair<std::regex, std::string>> Log::s_RedactionPatterns;
std::mutex Log::s_RedactionMutex;
bool Log::s_PrivacyMode = false;

// LogConfig Factory Methods
LogConfig LogConfig::Debug() {
  LogConfig config;
  config.enableConsole = true;
  config.enableFile = true;
  config.consoleLevel = spdlog::level::trace;
  config.fileLevel = spdlog::level::trace;
  config.minLevel = spdlog::level::trace;
  config.enableAsync = true;
  config.enablePerformanceTracking = true;
  return config;
}

LogConfig LogConfig::Release() {
  LogConfig config;
  config.enableConsole = true;
  config.enableFile = true;
  config.consoleLevel = spdlog::level::info;
  config.fileLevel = spdlog::level::trace;
  config.minLevel = spdlog::level::info;
  config.enableAsync = true;
  config.enablePerformanceTracking = false;
  return config;
}

LogConfig LogConfig::Distribution() {
  LogConfig config;
  config.enableConsole = false;
  config.enableFile = true;
  config.consoleLevel = spdlog::level::off;
  config.fileLevel = spdlog::level::err;
  config.minLevel = spdlog::level::err;
  config.enableAsync = true;
  config.enablePerformanceTracking = false;
  return config;
}

// Initialization
void Log::init(const LogConfig &config) {
  if (s_Initialized.exchange(true)) {
    std::cerr << "Warning: Log system already initialized\n";
    return;
  }

  try {
    s_Config = config;

    if (config.enableAsync) {
      spdlog::init_thread_pool(LogConfig::ASYNC_QUEUE_SIZE,
                               LogConfig::ASYNC_THREAD_COUNT);
    }

    // Create Sinks
    std::vector<spdlog::sink_ptr> coreSinks;
    std::vector<spdlog::sink_ptr> clientSinks;
    CreateSinks(config, coreSinks, clientSinks);

    // Create Core Logger
    if (config.enableAsync) {
      s_CoreLogger = std::make_shared<spdlog::async_logger>(
          "BeEngine", coreSinks.begin(), coreSinks.end(), spdlog::thread_pool(),
          spdlog::async_overflow_policy::block);
    } else {
      s_CoreLogger = std::make_shared<spdlog::logger>(
          "BeEngine", coreSinks.begin(), coreSinks.end());
    }

    s_CoreLogger->set_level(config.minLevel);
    s_CoreLogger->set_pattern(config.pattern);
    spdlog::register_logger(s_CoreLogger);

    // Create Client Logger
    if (config.enableAsync) {
      s_ClientLogger = std::make_shared<spdlog::async_logger>(
          "APP", clientSinks.begin(), clientSinks.end(), spdlog::thread_pool(),
          spdlog::async_overflow_policy::block);
    } else {
      s_ClientLogger = std::make_shared<spdlog::logger>(
          "APP", clientSinks.begin(), clientSinks.end());
    }

    s_ClientLogger->set_level(config.minLevel);
    s_ClientLogger->set_pattern(config.pattern);
    spdlog::register_logger(s_ClientLogger);

    // Create Category Logger
    const std::vector<LogCategory> categories = {LogCategory::Core,
                                                 LogCategory::Events};

    for (auto category : categories) {
      auto logger = std::make_shared<spdlog::logger>(
          CategoryToString(category), coreSinks.begin(), coreSinks.end());

      logger->set_level(config.minLevel);
      logger->set_pattern(config.pattern);
      spdlog::register_logger(logger);

      s_CategoryLogger[category] = logger;
    }

    FlushOn(spdlog::level::err);
    s_CoreLogger->info("BeEngine Logging System Initialized!");

  } catch (const spdlog::spdlog_ex &ex) {
    std::cerr << "Log initialization failed: \n" << ex.what();
    s_Initialized = false;
    throw;
  }
}

void Log::Shutdown() {
  if (!s_Initialized.exchange(false)) {
    return;
  }

  if (s_CoreLogger) {
    s_CoreLogger->info("Shutting down logging system");
  }

  Flush();
  spdlog::drop_all();
  spdlog::shutdown();

  s_CoreLogger.reset();
  s_ClientLogger.reset();
  s_CategoryLogger.clear();
}

// LOGGER ACCESS
std::shared_ptr<spdlog::logger> &Log::GetCoreLogger() {
  if (!s_Initialized.load()) {
    FallbackLog(spdlog::level::err, "GetCoreLogger called before init()");
    static std::shared_ptr<spdlog::logger> fallback =
        spdlog::stdout_color_mt("FALLBACK");
    return fallback;
  }
  return s_CoreLogger;
}

std::shared_ptr<spdlog::logger> &Log::GetClientLogger() {
  if (!s_Initialized.load()) {
    FallbackLog(spdlog::level::err, "GetClientLogger called before init()");
    static std::shared_ptr<spdlog::logger> fallback =
        spdlog::stdout_color_mt("FALLBACK");
    return fallback;
  }
  return s_ClientLogger;
}

std::shared_ptr<spdlog::logger> &Log::GetLogger(LogCategory category) {
  if (!s_Initialized.load()) {
    return GetCoreLogger();
  }

  auto it = s_CategoryLogger.find(category);
  if (it != s_CategoryLogger.end()) {
    return it->second;
  }
  return s_CoreLogger;
}

// RUNTIME CONFIGURATION
void Log::SetGlobalLevel(spdlog::level::level_enum level) {
  if (s_CoreLogger) {
    s_CoreLogger->set_level(level);
  }
  if (s_ClientLogger) {
    s_ClientLogger->set_level(level);
  }

  for (auto &[category, logger] : s_CategoryLogger) {
    logger->set_level(level);
  }
}

void Log::SetCategoryLevel(LogCategory category,
                           spdlog::level::level_enum level) {
  auto &logger = GetLogger(category);
  logger->set_level(level);
}

// CRASH HANDLING
void Log::Flush() {
  if (s_CoreLogger) {
    s_CoreLogger->flush();
  }

  if (s_ClientLogger) {
    s_ClientLogger->flush();
  }

  for (auto &[category, logger] : s_CategoryLogger) {
    logger->flush();
  }
}

void Log::FlushOn(spdlog::level::level_enum level) { spdlog::flush_on(level); }

// PERFORMANCE & STATS

LogStats Log::GetStats() {
  std::lock_guard<std::mutex> lock(s_StatsMutex);
  return s_Stats;
}

void Log::ResetStats() {
  std::lock_guard<std::mutex> lock(s_StatsMutex);
  s_Stats = LogStats();
}

// PRIVACY AND SECURITY
void Log::AddRedactionPattern(const std::regex &pattern,
                              const std::string &replacement) {
  std::lock_guard<std::mutex> lock(s_RedactionMutex);
  s_RedactionPatterns.emplace_back(pattern, replacement);
}

void Log::SetPrivacyMode(bool enabled) { s_PrivacyMode = enabled; }

// INTERNAL HELPERS
void Log::CreateSinks(const LogConfig &config,
                      std::vector<spdlog::sink_ptr> &coreSinks,
                      std::vector<spdlog::sink_ptr> &clientSinks) {

  if (config.enableConsole) {
    CreateConsoleSinks(coreSinks, config);
    CreateConsoleSinks(clientSinks, config);
  }

  if (config.enableFile) {
    CreateFileSinks(coreSinks, clientSinks, config);
  }
}

void Log::CreateConsoleSinks(std::vector<spdlog::sink_ptr> &sinks,
                             const LogConfig &config) {
  auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  consoleSink->set_level(config.consoleLevel);
  sinks.push_back(consoleSink);
}

void Log::CreateFileSinks(std::vector<spdlog::sink_ptr> &coreSinks,
                          std::vector<spdlog::sink_ptr> &clientSinks,
                          const LogConfig &config) {

  std::string timestamp = GetTimestamps();

  if (config.enableRotating) {
    auto coreFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        config.logDirectory + "BeEngine_" + timestamp + ".log",
        config.maxFileSize, config.maxFiles);
    coreFileSink->set_level(config.fileLevel);
    coreSinks.push_back(coreFileSink);

    auto clientFileSink =
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            config.logDirectory + "App_" + timestamp + ".log",
            config.maxFileSize, config.maxFiles);
    clientFileSink->set_level(config.fileLevel);
    clientSinks.push_back(clientFileSink);
  } else {
    auto coreFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        config.logDirectory + "BeEngine_" + timestamp + ".log");
    coreFileSink->set_level(config.fileLevel);
    coreSinks.push_back(coreFileSink);

    auto clientFileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        config.logDirectory + "App_" + timestamp + ".log");
    clientFileSink->set_level(config.fileLevel);
    clientSinks.push_back(clientFileSink);
  }
}

std::string Log::GetTimestamps() {
  auto now = std::chrono::system_clock::now();
  auto time = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S");
  return ss.str();
}

std::string Log::RedactMessage(const std::string &message) {
  if (!s_PrivacyMode) {
    return message;
  }

  std::lock_guard<std::mutex> lock(s_RedactionMutex);
  std::string result = message;

  for (const auto &[pattern, replacement] : s_RedactionPatterns) {
    result = std::regex_replace(result, pattern, replacement);
  }

  return result;
}

void Log::FallbackLog(spdlog::level::level_enum level, const char *message) {
  std::cerr << "[FALLBACK LOG] \n" << message;
}

std::string Log::CategoryToString(LogCategory category) {
  switch (category) {
  case LogCategory::Core:
    return "Core";
  case LogCategory::Events:
    return "Events";
  default:
    return "Unknown";
  }
}

} // namespace BeEngine
