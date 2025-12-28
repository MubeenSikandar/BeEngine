#pragma once

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
// must be first OpenGL header

#include <glm/glm.hpp>
#include <imgui.h>

#include "Codes/GamepadCodes.hpp"
#include "Codes/KeyCodes.hpp"
#include "Codes/MouseCodes.hpp"
#include "Core.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/Event.hpp"
#include "Events/EventQueue.hpp"
#include "Events/GamepadEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "ImGui/ImGuiLayer.hpp"
#include "Input/Input.hpp"
#include "Layers/Layer.hpp"
#include "Layers/LayerStack.hpp"
#include "Logs/Log.hpp"
#include "Renderer/OpenGLRendererAPI.hpp"
#include "Renderer/Renderer.hpp"
#include "Timestep.hpp"
#include "Window.hpp"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <exception>
#include <format>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <ranges>
#include <ratio>
#include <string>
#include <unordered_map>
