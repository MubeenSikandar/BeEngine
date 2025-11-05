#include "../BeEngine/include/Application.hpp"
#include "../BeEngine/include/EntryPoint.hpp"

class Sandbox : public BeEngine::Application {
public:
  Sandbox() = default;
  ~Sandbox() override = default;
};

BeEngine::Application *BeEngine::CreateApplication() { return new Sandbox(); }
