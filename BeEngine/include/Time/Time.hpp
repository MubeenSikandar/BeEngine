// Time.hpp
#pragma once
#include <Core.hpp>

namespace BeEngine {

/**
 * @brief Time management for both games and simulations
 *
 * Supports:
 * - Variable timestep (games)
 * - Fixed timestep (simulations)
 * - Time scaling (slow-mo, fast-forward)
 * - Pause/resume
 */
class BE_API Time {
public:
  // Frame timing (call once per frame from Application)
  static void Update();

  // ===== Getters =====

  /// Time since last frame (scaled)
  static float GetDeltaTime() { return s_DeltaTime * s_TimeScale; }

  /// Unscaled delta time (ignores time scale)
  static float GetUnscaledDeltaTime() { return s_DeltaTime; }

  /// Time since application start (scaled)
  static float GetTime() { return s_Time; }

  /// Unscaled time since start
  static float GetUnscaledTime() { return s_UnscaledTime; }

  /// Fixed timestep for physics/simulation (default: 1/60)
  static float GetFixedDeltaTime() { return s_FixedDeltaTime; }

  /// Current frame count
  static uint64_t GetFrameCount() { return s_FrameCount; }

  /// Frames per second
  static float GetFPS() { return s_FPS; }

  // ===== Time Control =====

  /// Set time scale (1.0 = normal, 0.5 = slow-mo, 2.0 = fast)
  static void SetTimeScale(float scale) { s_TimeScale = scale; }
  static float GetTimeScale() { return s_TimeScale; }

  /// Pause/resume time
  static void SetPaused(bool paused) { s_Paused = paused; }
  static bool IsPaused() { return s_Paused; }

  /// Set fixed timestep (for simulations)
  static void SetFixedDeltaTime(float dt) { s_FixedDeltaTime = dt; }

  // ===== Simulation Support =====

  /// Get accumulated time for fixed timestep loop
  static float GetFixedTimeAccumulator() { return s_FixedAccumulator; }

  /// Consume fixed timestep (call after each physics step)
  static void ConsumeFixedTime() { s_FixedAccumulator -= s_FixedDeltaTime; }

  /// Check if we should run another fixed update
  static bool ShouldRunFixedUpdate() {
    return s_FixedAccumulator >= s_FixedDeltaTime;
  }

private:
  static float s_DeltaTime;
  static float s_Time;
  static float s_UnscaledTime;
  static float s_TimeScale;
  static float s_FixedDeltaTime;
  static float s_FixedAccumulator;
  static float s_LastFrameTime;
  static float s_FPS;
  static uint64_t s_FrameCount;
  static bool s_Paused;

  // For FPS calculation
  static float s_FPSAccumulator;
  static uint32_t s_FPSFrameCount;
};

} // namespace BeEngine
