#pragma once
#include "Core.hpp"

namespace BeEngine {
class BE_API Application {
public:
  Application();
  virtual ~Application();
  static void Run();
};

// To be defined in CLIENT
Application *CreateApplication();
} // namespace BeEngine
