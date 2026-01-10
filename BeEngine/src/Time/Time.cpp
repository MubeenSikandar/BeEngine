// Time/Time.cpp
#include <PCH/BeEnginePCH.hpp>

namespace BeEngine {

// Static member initialization
float Time::s_DeltaTime = 0.0F;
float Time::s_Time = 0.0F;
float Time::s_UnscaledTime = 0.0F;
float Time::s_TimeScale = 1.0F;
float Time::s_FixedDeltaTime = 1.0F / 60.0F; // 60 Hz default
float Time::s_FixedAccumulator = 0.0F;
float Time::s_LastFrameTime = 0.0F;
float Time::s_FPS = 0.0F;
uint64_t Time::s_FrameCount = 0;
bool Time::s_Paused = false;
float Time::s_FPSAccumulator = 0.0F;
uint32_t Time::s_FPSFrameCount = 0;

void Time::Update() {
  float currentTime = static_cast<float>(glfwGetTime());
  s_DeltaTime = currentTime - s_LastFrameTime;
  s_LastFrameTime = currentTime;

  // Clamp delta time to avoid spiral of death
  // (e.g., when debugging or window is dragged)
  s_DeltaTime = std::clamp(s_DeltaTime, 0.0F, 0.25F);

  // Update unscaled time (always increments)
  s_UnscaledTime += s_DeltaTime;

  // Update scaled time and fixed accumulator (respects pause)
  if (!s_Paused) {
    s_Time += s_DeltaTime * s_TimeScale;
    s_FixedAccumulator += s_DeltaTime * s_TimeScale;
  }

  s_FrameCount++;

  // FPS calculation (update every second)
  s_FPSAccumulator += s_DeltaTime;
  s_FPSFrameCount++;

  if (s_FPSAccumulator >= 1.0F) {
    s_FPS = static_cast<float>(s_FPSFrameCount) / s_FPSAccumulator;
    s_FPSAccumulator = 0.0F;
    s_FPSFrameCount = 0;
  }
}

} // namespace BeEngine
