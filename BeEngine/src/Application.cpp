// src/Application.cpp
#include "Application.hpp"

namespace BeEngine {

Application *Application::s_Instance = nullptr;

// Initialize m_EventQueue in member initializer list
Application::Application()
    : m_EventQueue(Config{
          .maxQueue = 1000,
          .dropOnOverflow = true,
          .enableProfiling = true,
          .enableLogging = false,
      }) {
  BE_CORE_ASSERT(!s_Instance, "Application already exists!");
  s_Instance = this;

  BE_CORE_INFO("Creating Application...");

  m_Window = Window::Create(WindowProps("BeEngine", Width{1280}, Height{720}));

  // Initialize renderer (AFTER window creation!)
  RendererAPI::SetAPI(RenderAPI::OpenGL);
  Renderer::Init();

  // Initialize Input System
  Input::Init();

  // Set initial viewport
  Renderer::SetViewport(0, 0, 1280, 720);

  // Create and push ImGui layer
  m_ImGuiLayer = std::make_shared<ImGuiLayer>();
  PushOverlay(m_ImGuiLayer);

  m_Window->SetEventCallback([this](Event &e) {
    EventDispatcher dispatcher(e);

    dispatcher.Dispatch<WindowCloseEvent>(
        [this](WindowCloseEvent &we) { return OnWindowClose(we); });

    dispatcher.Dispatch<WindowResizeEvent>(
        [this](WindowResizeEvent &we) { return OnWindowResize(we); });

    if (!e.IsHandled) {
      DispatchEventToLayers(e);
    }

    if (!e.IsHandled) {
      OnEvent(e);
    }
  });

  BE_CORE_INFO("Application Created Successfully");
}

Application::~Application() {
  BE_CORE_INFO("Cleaning Up application...");

  // Shutdown renderer
  Renderer::Shutdown();

  auto stats = m_EventQueue.GetStats();
  BE_CORE_INFO("Event Statistics:");
  BE_CORE_INFO("  Total Queued: {}", stats.totalEventsQueued);
  BE_CORE_INFO("  Total Processed: {}", stats.totalEventsProcessed);
  BE_CORE_INFO("  Total Dropped: {}", stats.totalEventsDropped);
  BE_CORE_INFO("  Max Queue Size: {}", stats.maxQueueSize);
  if (stats.totalEventsProcessed > 0) {
    BE_CORE_INFO("  Avg Process Time: {:.3f}ms", stats.averageProcessTimeMs);
  }
}

void Application::Run() {
  BE_CORE_INFO("Application started");

  while (m_Running && !m_Window->shouldClose()) {
    // Calculate delta time
    auto time = static_cast<float>(glfwGetTime());
    Timestep timestep{time - m_LastFrameTime};
    m_LastFrameTime = time;

    Input::Update();

    // Process events from queue
    ProcessEvents();

    // Update layers (if not minimized)
    if (!m_Minimized) {
      UpdateLayers(timestep);
    }

    if (!m_Minimized) {
      Renderer::BeginFrame();
      Renderer::Clear(0.1F, 0.1F, 0.1F, 1.0F); // Dark gray background

      // Render all layers
      for (auto &layer : m_LayerStack) {
        if (layer && layer->IsEnabled()) {
          layer->OnRender();
        }
      }

      // Render ImGui
      if (m_ImGuiLayer) {
        m_ImGuiLayer->Begin();

        // Begin dockspace (full-window docking area)
        m_ImGuiLayer->BeginDockspace();

        for (auto &layer : m_LayerStack) {
          if (layer && layer->IsEnabled()) {
            layer->OnImGuiRender();
          }
        }

        // End dockspace
        m_ImGuiLayer->EndDockspace();

        m_ImGuiLayer->End();
      }

      Renderer::EndFrame();
      BeEngine::Input::EndFrame();
    }

    // Swap buffers
    m_Window->OnUpdate();
  }

  BE_CORE_INFO("Application loop ended");
  BE_CORE_INFO("Application finished normally");
}

void Application::OnEvent(Event &event) {
  BE_CORE_TRACE("Event received: {}", event.ToString());

  // Default implementation does nothing
  // Derived classes can override this
}

void Application::PushLayer(std::shared_ptr<Layer> layer) {
  m_LayerStack.PushLayer(std::move(layer));
}

void Application::PushOverlay(std::shared_ptr<Layer> overlay) {
  m_LayerStack.PushOverlay(std::move(overlay));
}

void Application::PopLayer(const std::shared_ptr<Layer> &layer) {
  m_LayerStack.PopLayer(layer);
}

void Application::PopOverlay(const std::shared_ptr<Layer> &overlay) {
  m_LayerStack.PopOverlay(overlay);
}

bool Application::OnWindowClose(WindowCloseEvent &e) {
  BE_CORE_INFO("Window Close Requested");
  m_Running = false;
  return true;
}

bool Application::OnWindowResize(WindowResizeEvent &e) {
  if (e.GetWidth() == 0 || e.GetHeight() == 0) {
    BE_CORE_INFO("Window minimized");
    m_Minimized = true;
    return false;
  }

  BE_CORE_INFO("Window resized to {}x{}", e.GetWidth(), e.GetHeight());
  m_Minimized = false;

  // TODO: Update renderer viewport
  Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

  return false;
}

void Application::ProcessEvents() {
  // Process events with a time budget of 5ms per frame
  m_EventQueue.ProcessEventsWithBudget(5.0);
}

void Application::UpdateLayers(Timestep ts) {
  // Update all layers in forward order
  for (auto &layer : m_LayerStack) {
    if (layer && layer->IsEnabled()) {
      layer->OnUpdate(ts);
    }
  }
}

void Application::DispatchEventToLayers(Event &event) {
  // Dispatch events to layers in REVERSE order (overlays first)
  for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it) {
    if (event.IsHandled) {
      break; // Stop if event was handled
    }

    if (*it && (*it)->IsEnabled()) {
      (*it)->OnEvent(event);
    }
  }
}

} // namespace BeEngine
