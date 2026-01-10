// EntryPoint.hpp
#pragma once
#include <Application.hpp>
#include <Core.hpp>
#include <Logs/Log.hpp>
#include <Logs/LogConfig.hpp>
#include <cstddef>
#include <exception>

#if defined(_WIN32) || defined(__APPLE__) || defined(__linux__)

int main() {
// INITIALIZE LOGGING SYSTEM
#ifdef BE_DEBUG
  BeEngine::Log::init(BeEngine::LogConfig::Debug());
  BE_CORE_INFO("Starting in DEBUG mode");
#elif defined(BE_RELEASE)
  BeEngine::Log::init(BeEngine::LogConfig::Release());
  BE_CORE_INFO("Starting in RELEASE mode");
#elif defined(BE_DIST)
  BeEngine::Log::init(BeEngine::LogConfig::Distribution());
#else
  BeEngine::Log::init(BeEngine::LogConfig::Debug());
  BE_CORE_WARN("Build type not defined, defaulting to Debug");
#endif

  // LOG SYSTEM INFORMATION
  BE_CORE_INFO("========================================");
  BE_CORE_INFO("BeEngine v.{}.{}.{}", BE_VERSION_MAJOR, BE_VERSION_MINOR,
               BE_VERSION_PATCH);
  BE_CORE_INFO("Build: {}", BE_BUILD_TYPE);

#ifdef BE_PLATFORM_WINDOWS
  BE_CORE_INFO("Platform: Windows");
#elif defined(BE_PLATFORM_APPLE)
  BE_CORE_INFO("Platform: macOS");
#elif defined(BE_PLATFORM_LINUX)
  BE_CORE_INFO("Platform: Linux");
#endif

  BE_CORE_INFO("========================================");

  // CREATE AND RUN APPLICATIONS
  BeEngine::Application *app = nullptr;
  int exitCode{};

  try {
    BE_CORE_INFO("Creating Application... ");
    app = BeEngine::CreateApplication();

    if (!app) {
      BE_CORE_CRITICAL("Failed to create application");
      return -1;
    }

    BE_CORE_INFO("Application Created Successfully");
    BE_INFO("Welcome to BeEngine!");

    // Run Application
    app->Run();

    BE_CORE_INFO("Applictaion finished Normally");

  } catch (const std::exception &ex) {
    BE_CORE_CRITICAL("Unhandled exception: {}", ex.what());
    exitCode = -1;
  } catch (...) {
    BE_CORE_CRITICAL("Unknown exception occured!");
    exitCode = -1;
  }

  // CleanUp
  if (app) {
    BE_CORE_INFO("Cleaning Up application... ");
    delete app;
    app = nullptr;
  }

  BE_CORE_INFO("Shutting Down...");
  BeEngine::Log::Shutdown();

  return exitCode;
}
#else
#error "Unsupported platform! Only Windows, macOS, and Linux are supported."
#endif
