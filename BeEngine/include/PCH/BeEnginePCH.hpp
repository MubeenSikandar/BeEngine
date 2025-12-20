#pragma once

#include <glad/glad.h>   // must be first OpenGL header
#include <GLFW/glfw3.h>  // GLFW after Glad

#include <glm/glm.hpp>
#include <imgui.h>

#include "Core.hpp"
#include "Events/ApplicationEvent.hpp"
#include "Events/Event.hpp"
#include "Events/EventQueue.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"
#include "KeyCodes.hpp"
#include "Layers/Layer.hpp"
#include "Layers/LayerStack.hpp"
#include "Logs/Log.hpp"
#include "MouseCodes.hpp"
#include "Timestep.hpp"
#include "Window.hpp"
#include "Renderer/OpenGLRendererAPI.hpp"
#include "Renderer/Renderer.hpp"
#include "../../Sandbox/src/TriangleLayer.hpp"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/common.h"
#include "spdlog/logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <cstdint>
#include <format>
#include <string>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <exception>
#include <memory>
#include <mutex>
#include <ratio>
#include <atomic>
#include <ranges>
#include <iostream>
#include <iomanip>
