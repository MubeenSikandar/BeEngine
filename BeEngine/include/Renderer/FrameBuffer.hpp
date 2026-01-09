#pragma once

#include "Core.hpp"
#include <cstdint>
#include <vector>
namespace BeEngine {

/**
 * @brief Framebuffer texture format specification
 *
 * Defines the format of attachments (color, depth, stencil)
 */
enum class FramebufferTextureFormat : uint8_t {
  None = 0,

  // Color formats
  RGBA8 = 1,       // Standard 8-bit RGBA
  RGBA16F = 2,     // HDR 16-bit float RGBA
  RGBA32F = 3,     // HDR 32-bit float RGBA
  RED_INTEGER = 4, // For entity/object picking (stores IDs)

  // Depth/stencil formats
  DEPTH24STENCIL8 = 5, // Combined depth (24-bit) + stencil (8-bit)
  DEPTH32F = 6,        // 32-bit float depth (for shadow maps)

  // Defaults
  Depth = DEPTH24STENCIL8
};

/**
 * @brief Texture filtering mode
 */
enum class FramebufferTextureFilter : uint8_t {
  Nearest, // No interpolation (pixelated)
  Linear   // Bilinear interpolation (smooth)
};

/**
 * @brief Texture wrap mode
 */
enum class FramebufferTextureWrap : uint8_t {
  Repeat,
  ClampToEdge,
  ClampToBorder
};

/**
 * @brief Specification for a single framebuffer attachment
 */
struct FramebufferTextureSpecification {
  FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
  FramebufferTextureFilter FilterMode = FramebufferTextureFilter::Linear;
  FramebufferTextureWrap WrapMode = FramebufferTextureWrap::ClampToEdge;

  FramebufferTextureSpecification() = default;
  FramebufferTextureSpecification(FramebufferTextureFormat format)
      : TextureFormat(format) {}
  FramebufferTextureSpecification(FramebufferTextureFormat format,
                                  FramebufferTextureFilter filter,
                                  FramebufferTextureWrap wrap)
      : TextureFormat(format), FilterMode(filter), WrapMode(wrap) {}
};

/**
 * @brief Collection of attachment specifications
 */
struct FramebufferAttachmentSpecification {
  std::vector<FramebufferTextureSpecification> Attachments;

  FramebufferAttachmentSpecification() = default;
  FramebufferAttachmentSpecification(
      std::initializer_list<FramebufferTextureSpecification> attachments)
      : Attachments(attachments) {}
};

/**
 * @brief Complete framebuffer specification
 */
struct FramebufferSpecification {
  uint32_t Width = 1280;
  uint32_t Height = 720;
  uint32_t Samples = 1; // 1 = no multisampling, >1 = MSAA

  FramebufferAttachmentSpecification Attachments;

  // Whether this framebuffer renders to the screen (swap chain)
  bool SwapChainTarget = false;
};

/**
 * @brief Abstract framebuffer interface
 *
 * A framebuffer is an off-screen render target. It allows you to render
 * a scene to a texture instead of directly to the screen. This is essential
 * for:
 * - Editor viewports (render scene to ImGui window)
 * - Post-processing effects (bloom, blur, etc.)
 * - Shadow mapping
 * - Deferred rendering (G-buffer)
 * - Object picking (render entity IDs to texture)
 *
 * Usage:
 *   1. Create framebuffer with desired size and attachments
 *   2. Bind framebuffer before rendering
 *   3. Render your scene
 *   4. Unbind framebuffer
 *   5. Use GetColorAttachmentRendererID() to get the texture for display
 */
class BE_API Framebuffer {
public:
  virtual ~Framebuffer() = default;

  /**
   * @brief Bind this framebuffer for rendering
   *
   * All subsequent draw calls will render to this framebuffer
   * instead of the default screen framebuffer.
   */
  virtual void Bind() = 0;

  /**
   * @brief Unbind this framebuffer
   *
   * Subsequent draw calls will render to the default framebuffer (screen).
   */
  virtual void Unbind() = 0;

  /**
   * @brief Resize the framebuffer
   *
   * Called when the viewport size changes. Recreates all attachments
   * at the new resolution.
   *
   * @param width New width in pixels
   * @param height New height in pixels
   */
  virtual void Resize(uint32_t width, uint32_t height) = 0;

  /**
   * @brief Read a pixel value from a color attachment
   *
   * Useful for object picking - render entity IDs to a RED_INTEGER
   * attachment, then read the pixel under the mouse.
   *
   * @param attachmentIndex Which color attachment to read from
   * @param x X coordinate in pixels
   * @param y Y coordinate in pixels
   * @return Pixel value as int (for RED_INTEGER format)
   */
  virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

  /**
   * @brief Clear a specific color attachment to a value
   *
   * @param attachmentIndex Which color attachment to clear
   * @param value Clear value (interpretation depends on format)
   */
  virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

  /**
   * @brief Get the GPU texture ID of a color attachment
   *
   * Use this to display the framebuffer contents in ImGui:
   *   ImGui::Image((void*)(intptr_t)fb->GetColorAttachmentRendererID(0), ...);
   *
   * @param index Color attachment index (0 = first color attachment)
   * @return GPU texture handle (e.g., OpenGL texture ID)
   */
  NODISCARD virtual uint32_t
  GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

  /**
   * @brief Get the framebuffer specification
   */
  NODISCARD virtual const FramebufferSpecification &
  GetSpecification() const = 0;

  /**
   * @brief Create a framebuffer for the current rendering API
   *
   * @param spec Framebuffer specification
   * @return Shared pointer to the created framebuffer
   */
  static Ref<Framebuffer> Create(const FramebufferSpecification &spec);
};

} // namespace BeEngine
