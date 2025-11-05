#pragma once
#include "Application.hpp"

#if defined(_WIN32) || defined(__APPLE__) || defined(__linux__)
int main() {
  auto *app = BeEngine::CreateApplication();
  app->Run();
  delete app;
  return 0;
}
#endif
