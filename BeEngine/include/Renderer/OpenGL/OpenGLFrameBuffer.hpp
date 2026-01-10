#pragma once

#include <Core.hpp>
#include <Renderer/FrameBuffer.hpp>

namespace BeEngine {
/**
 * @brief OpenGL implementation of Framebuffer
 *
 * Uses OpenGL Framebuffer Objects (FBOs) to render to textures.
 * Supports multiple color attachments and depth/stencil attachments.
 *
 * OpenGL Concepts:
 * - FBO (Framebuffer Object): Container that holds attachments
 * - Color Attachment: Texture that receives color output from fragment shader
 * - Depth Attachment: Texture/renderbuffer for depth testing
 * - Renderbuffer: Optimized storage for attachments not read as textures
 */

class OpenGLFramebuffer : public Framebuffer {
public:
  /**
   * @brief Construct an OpenGL framebuffer
   *
   * @param spec Framebuffer specification defining size and attachments
   */
  explicit OpenGLFramebuffer(const FramebufferSpecification &spec);
  ~OpenGLFramebuffer() override;

  // Prevent copying (GPU resources are not copyable)
  OpenGLFramebuffer(const OpenGLFramebuffer &) = delete;
  OpenGLFramebuffer &operator=(const OpenGLFramebuffer &) = delete;

  // Allow moving
  OpenGLFramebuffer(OpenGLFramebuffer &&other) noexcept;
  OpenGLFramebuffer &operator=(OpenGLFramebuffer &&other) noexcept;

  void Bind() override;
  void Unbind() override;
  void Resize(uint32_t width, uint32_t height) override;
  int ReadPixel(uint32_t attachmentIndex, int x, int y) override;
  void ClearAttachment(uint32_t attachmentIndex, int value) override;

  NODISCARD uint32_t
  GetColorAttachmentRendererID(uint32_t index = 0) const override;
  NODISCARD const FramebufferSpecification &GetSpecification() const override {
    return m_Specification;
  }

  /**
   * @brief Recreate the framebuffer with current specification
   *
   * Called internally after construction and resize.
   * Deletes existing GPU resources and creates new ones.
   */
  void Invalidate();

private:
  /**
   * @brief Delete all GPU resources
   */
  void Destroy();

  /**
   * @brief Check if a format is a depth format
   */
  static bool IsDepthFormat(FramebufferTextureFormat format);

  /**
   * @brief Create a texture attachment
   */
  void CreateTextureAttachment(uint32_t &outID, FramebufferTextureFormat format,
                               uint32_t width, uint32_t height, int index);

  uint32_t m_RendererID = 0; // OpenGL FBO ID

  FramebufferSpecification m_Specification;

  // Separate color and depth attachment specs for easier processing
  std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecs;
  FramebufferTextureSpecification m_DepthAttachmentSpec;

  // GPU texture IDs
  std::vector<uint32_t> m_ColorAttachments;
  uint32_t m_DepthAttachment = 0;
};

} // namespace BeEngine
