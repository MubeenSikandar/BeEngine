#include "../include/Application.hpp"
#include <iostream>

namespace BeEngine {
Application::Application() = default;
Application::~Application() {}

void Application::Run() {
  while (true) {
    std::cout << "Run Like the Wind!\n"; // Added: \n for newline
  }
}
} // namespace BeEngine
