// Camera/OrthographicCamera.hpp
#pragma once

#include <Camera/Camera.hpp>

namespace BeEngine {

/**
 * @brief 2D Orthographic Camera
 *
 * Perfect for:
 * - 2D games (platformers, top-down)
 * - UI rendering
 * - Scientific visualization
 * - CAD applications
 *
 * No perspective distortion - parallel lines stay parallel.
 * Objects don't get smaller with distance.
 */
class BE_API OrthographicCamera : public Camera {
public:
  /**
   * @brief Construct with explicit bounds
   * @param left   Left boundary
   * @param right  Right boundary
   * @param bottom Bottom boundary
   * @param top    Top boundary
   */
  OrthographicCamera(float left, float right, float bottom, float top);

  /**
   * @brief Construct with aspect ratio and size
   * @param aspectRatio Width / Height
   * @param size        Half-height of the view (zoom level)
   */
  OrthographicCamera(float aspectRatio, float size = 1.0F);

  // ===== Projection Settings =====

  /// Set orthographic bounds directly
  void SetProjection(float left, float right, float bottom, float top);

  /// Set projection using aspect ratio and size
  void SetProjectionByAspect(float aspectRatio, float size);

  /// Set near/far clip planes (default: -1 to 1 for 2D)
  void SetClipPlanes(float nearClip, float farClip);

  // ===== Zoom =====

  /// Get current zoom level (1.0 = default, 2.0 = zoomed in 2x)
  NODISCARD float GetZoom() const { return m_Zoom; }

  /// Set zoom level
  void SetZoom(float zoom);

  /// Get orthographic size (half-height of view)
  NODISCARD float GetSize() const { return m_Size; }

  /// Set orthographic size
  void SetSize(float size);

  // ===== Bounds =====

  NODISCARD float GetLeft() const { return m_Left; }
  NODISCARD float GetRight() const { return m_Right; }
  NODISCARD float GetBottom() const { return m_Bottom; }
  NODISCARD float GetTop() const { return m_Top; }
  NODISCARD float GetAspectRatio() const { return m_AspectRatio; }

  // ===== Viewport =====

  /// Update projection when viewport resizes
  void OnViewportResize(float width, float height);

protected:
  void RecalculateProjectionMatrix() override;

private:
  // Orthographic bounds
  float m_Left{-1.0F};
  float m_Right{1.0F};
  float m_Bottom{-1.0F};
  float m_Top{1.0F};

  // Derived values
  float m_AspectRatio{16.0F / 9.0F};
  float m_Size{1.0F}; // Half-height of view
  float m_Zoom{1.0F}; // Zoom multiplier
};

} // namespace BeEngine
