#pragma once

#include <Core.hpp>
#include <chrono>

namespace BeEngine {
/**
 * @brief Represents the time elapsed between frames
 *
 * Provides a clean interface for delta time with automatic conversion
 * between seconds and milliseconds. Immutable by design.
 *
 * Usage:
 *   Timestep ts(0.016f);  // 16ms, ~60 FPS
 *   float seconds = ts.GetSeconds();      // 0.016
 *   float ms = ts.GetMilliseconds();      // 16.0
 */

class BE_API Timestep {
public:
  /**
   * @brief Construct timestep from seconds
   * @param time Time in seconds (e.g., 0.016 for 60 FPS)
   */
  explicit Timestep(float time = 0.0F) : m_Time(time) {}

  /**
   * @brief Create timestep from high-resolution clock
   */
  static Timestep FromDuration(const std::chrono::duration<float> &duration) {
    return Timestep(duration.count());
  }

  // ======== Getters ========

  /**
   * @brief Get time in seconds
   */
  NODISCARD float GetSeconds() const { return m_Time; }

  /**
   * @brief Get time in milliseconds
   */
  NODISCARD float GetMilliseconds() const { return m_Time * MILLISECONDS; }

  /**
   * @brief Get time in microseconds
   */
  NODISCARD float GetMicroseconds() const { return m_Time * MICROSECONDS; }

  // ======== Operators ========

  /**
   * @brief Implicit conversion to float (seconds)
   */
  operator float() const { return m_Time; }

  /**
   * @brief Compare timesteps
   */
  bool operator==(const Timestep &other) const {
    return m_Time == other.m_Time;
  }

  bool operator!=(const Timestep &other) const { return !(*this == other); }

  bool operator<(const Timestep &other) const { return m_Time < other.m_Time; }

  bool operator>(const Timestep &other) const { return m_Time > other.m_Time; }

  // ======== Utility ========

  /**
   * @brief Get frames per second (1 / deltaTime)
   */
  NODISCARD float GETFPS() const {
    return m_Time > 0.0F ? 1.0F / m_Time : 0.0F;
  }

private:
  float m_Time{};
  const float MILLISECONDS{1000.0F};
  const float MICROSECONDS{1000000.0F};
};
} // namespace BeEngine
