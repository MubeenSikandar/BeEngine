#include "Application.hpp"
#include "imgui_impl_opengl3.h"
#include <PCH/BeEnginePCH.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>

namespace BeEngine {

ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

// Helper function to set dark theme colors
static void SetDarkThemeColors() {
  auto &colors = ImGui::GetStyle().Colors;

  colors[ImGuiCol_WindowBg] = ImVec4{0.1F, 0.105F, 0.11F, 1.0F};

  // Headers
  colors[ImGuiCol_Header] = ImVec4{0.2F, 0.205F, 0.21F, 1.0F};
  colors[ImGuiCol_HeaderHovered] = ImVec4{0.3F, 0.305F, 0.31F, 1.0F};
  colors[ImGuiCol_HeaderActive] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};

  // Buttons
  colors[ImGuiCol_Button] = ImVec4{0.2F, 0.205F, 0.21F, 1.0F};
  colors[ImGuiCol_ButtonHovered] = ImVec4{0.3F, 0.305F, 0.31F, 1.0F};
  colors[ImGuiCol_ButtonActive] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};

  // Frame BG
  colors[ImGuiCol_FrameBg] = ImVec4{0.2F, 0.205F, 0.21F, 1.0F};
  colors[ImGuiCol_FrameBgHovered] = ImVec4{0.3F, 0.305F, 0.31F, 1.0F};
  colors[ImGuiCol_FrameBgActive] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};

  // Tabs
  colors[ImGuiCol_Tab] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};
  colors[ImGuiCol_TabHovered] = ImVec4{0.38F, 0.3805F, 0.381F, 1.0F};
  colors[ImGuiCol_TabActive] = ImVec4{0.28F, 0.2805F, 0.281F, 1.0F};
  colors[ImGuiCol_TabUnfocused] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};
  colors[ImGuiCol_TabUnfocusedActive] = ImVec4{0.2F, 0.205F, 0.21F, 1.0F};

  // Title
  colors[ImGuiCol_TitleBg] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};
  colors[ImGuiCol_TitleBgActive] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4{0.15F, 0.1505F, 0.151F, 1.0F};
}

void ImGuiLayer::OnAttach() {
  BE_CORE_INFO("Initializing ImGui...");
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();

  // Enable features
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

  // When viewports are enabled, tweak WindowRounding/WindowBg
  // so platform windows can look identical to regular ones.
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0F;
    style.Colors[ImGuiCol_WindowBg].w = 1.0F;
  }

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Customize colors (optional)
  SetDarkThemeColors();

  // Setup Platform/Renderer backends
  Application &app = Application::Get();
  auto *window = static_cast<GLFWwindow *>(app.GetWindow().GetNativeWindow());

  // Initialize GLFW backend
  ImGui_ImplGlfw_InitForOpenGL(window, true);

  // Initialize OpenGL3 backend

#if defined(__APPLE__)
  const char *glsl_version = "#version 410";
#elif defined(WIN32)
  const char *glsl_version = "#version 450";
#else
  const char *glsl_version = "#version 330";
#endif

  ImGui_ImplOpenGL3_Init(glsl_version);

  BE_CORE_INFO("ImGui initialization successfull");
}

void ImGuiLayer::OnDetach() {
  BE_CORE_INFO("Shutting down ImGui...");

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  BE_CORE_INFO("ImGui Shut down");
}

void ImGuiLayer::OnEvent(Event &e) {
  // Use Application's block setting instead of local m_BlockEvents
  if (Application::Get().IsBlockingEvents()) {
    ImGuiIO &io = ImGui::GetIO();

    if (e.IsInCategory(EventCategory::EventCategoryMouse) &&
        io.WantCaptureMouse) {
      e.IsHandled = true;
    }

    if (e.IsInCategory(EventCategory::EventCategoryKeyboard) &&
        io.WantCaptureKeyboard) {
      e.IsHandled = true;
    }

    if (e.IsInCategory(EventCategory::EventCategoryGamepad) &&
        io.WantCaptureKeyboard) {
      e.IsHandled = true;
    }
  }
}

void ImGuiLayer::Begin() {
  // Start the Dear ImGui frame
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

void ImGuiLayer::End() {
  // Start the Dear ImGui frame
  ImGuiIO &io = ImGui::GetIO();
  Application &app = Application::Get();
  io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()),
                          static_cast<float>(app.GetWindow().GetHeight()));

  // Rendering
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

  // Update and Render additional Platform Windows
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();
    glfwMakeContextCurrent(backup_current_context);
  }
}

bool ImGuiLayer::WantCaptureMouse() const {
  return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiLayer::WantCaptureKeyboard() const {
  return ImGui::GetIO().WantCaptureKeyboard;
}

void ImGuiLayer::BeginDockspace() {
  if (!m_DockspaceEnabled) {
    return;
  }

  // Setup dockspace
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window
  // not dockable
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  // Get the main viewport
  const ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  // Transparent window settings
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |=
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render
  // our background
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
    window_flags |= ImGuiWindowFlags_NoBackground;
  }

  // Important: note that we proceed even if Begin() returns false (closed
  // window). This is because we want to keep our DockSpace() active.
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace", nullptr, window_flags);
  ImGui::PopStyleVar();
  ImGui::PopStyleVar(2);

  // Submit the DockSpace
  ImGuiIO &io = ImGui::GetIO();
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }
}

void ImGuiLayer::EndDockspace() {
  if (!m_DockspaceEnabled) {
    return;
  }

  ImGui::End(); // End DockSpace window
}

} // namespace BeEngine
