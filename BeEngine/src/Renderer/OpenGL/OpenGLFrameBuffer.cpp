#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

// Maximum number of color attachments (OpenGL guarantees at least 8)
static constexpr uint32_t s_MaxFramebufferSize = 8192;
static constexpr uint32_t s_MaxColorAttachments = 8;

// ===== Helper Functions =====

static GLenum TextureTarget(bool multisampled) {
  return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
}

static void CreateTextures(bool multisampled, uint32_t *outID, uint32_t count) {
  glGenTextures(count, outID);
}

static void BindTexture(bool multisampled, uint32_t id) {
  glBindTexture(TextureTarget(multisampled), id);
}

static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat,
                               GLenum format, GLenum type, uint32_t width,
                               uint32_t height, int index) {
  bool multisampled = samples > 1;
  if (multisampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat,
                            width, height, GL_FALSE);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
                 type, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index,
                         TextureTarget(multisampled), id, 0);
}

static void AttachDepthTexture(uint32_t id, int samples, GLenum format,
                               GLenum attachmentType, uint32_t width,
                               uint32_t height) {
  bool multisampled = samples > 1;
  if (multisampled) {
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width,
                            height, GL_FALSE);
  } else {
    // Use glTexStorage2D for immutable storage (more efficient)
    // Fallback to glTexImage2D for compatibility
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_DEPTH_STENCIL,
                 GL_UNSIGNED_INT_24_8, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType,
                         TextureTarget(multisampled), id, 0);
}

// static GLenum BeEngineFBTextureFormatToGL(FramebufferTextureFormat format) {
//   switch (format) {
//   case FramebufferTextureFormat::RGBA8:
//     return GL_RGBA8;
//   case FramebufferTextureFormat::RGBA16F:
//     return GL_RGBA16F;
//   case FramebufferTextureFormat::RGBA32F:
//     return GL_RGBA32F;
//   case FramebufferTextureFormat::RED_INTEGER:
//     return GL_R32I;
//   case FramebufferTextureFormat::DEPTH24STENCIL8:
//     return GL_DEPTH24_STENCIL8;
//   case FramebufferTextureFormat::DEPTH32F:
//     return GL_DEPTH_COMPONENT32F;
//   default:
//     BE_CORE_ASSERT(false, "Unknown framebuffer texture format!");
//     return 0;
//   }
// }

// ===== OpenGLFramebuffer Implementation =====

OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification &spec)
    : m_Specification(spec) {
  // Separate color and depth attachments
  for (const auto &attachment : m_Specification.Attachments.Attachments) {
    if (IsDepthFormat(attachment.TextureFormat)) {
      m_DepthAttachmentSpec = attachment;
    } else {
      m_ColorAttachmentSpecs.push_back(attachment);
    }
  }

  Invalidate();
}

OpenGLFramebuffer::~OpenGLFramebuffer() { Destroy(); }

OpenGLFramebuffer::OpenGLFramebuffer(OpenGLFramebuffer &&other) noexcept
    : m_RendererID(other.m_RendererID), m_Specification(other.m_Specification),
      m_ColorAttachmentSpecs(std::move(other.m_ColorAttachmentSpecs)),
      m_DepthAttachmentSpec(other.m_DepthAttachmentSpec),
      m_ColorAttachments(std::move(other.m_ColorAttachments)),
      m_DepthAttachment(other.m_DepthAttachment) {
  other.m_RendererID = 0;
  other.m_DepthAttachment = 0;
}

OpenGLFramebuffer &
OpenGLFramebuffer::operator=(OpenGLFramebuffer &&other) noexcept {
  if (this != &other) {
    Destroy();

    m_RendererID = other.m_RendererID;
    m_Specification = other.m_Specification;
    m_ColorAttachmentSpecs = std::move(other.m_ColorAttachmentSpecs);
    m_DepthAttachmentSpec = other.m_DepthAttachmentSpec;
    m_ColorAttachments = std::move(other.m_ColorAttachments);
    m_DepthAttachment = other.m_DepthAttachment;

    other.m_RendererID = 0;
    other.m_DepthAttachment = 0;
  }
  return *this;
}

void OpenGLFramebuffer::Invalidate() {
  // Clean up existing resources if any
  if (m_RendererID != 0) {
    Destroy();
  }

  // Create framebuffer
  glGenFramebuffers(1, &m_RendererID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

  bool multisample = m_Specification.Samples > 1;

  // Create color attachments
  if (!m_ColorAttachmentSpecs.empty()) {
    m_ColorAttachments.resize(m_ColorAttachmentSpecs.size());
    CreateTextures(multisample, m_ColorAttachments.data(),
                   static_cast<uint32_t>(m_ColorAttachments.size()));

    for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
      BindTexture(multisample, m_ColorAttachments[i]);

      switch (m_ColorAttachmentSpecs[i].TextureFormat) {
      case FramebufferTextureFormat::RGBA8:
        AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples,
                           GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE,
                           m_Specification.Width, m_Specification.Height,
                           static_cast<int>(i));
        break;

      case FramebufferTextureFormat::RGBA16F:
        AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples,
                           GL_RGBA16F, GL_RGBA, GL_FLOAT, m_Specification.Width,
                           m_Specification.Height, static_cast<int>(i));
        break;

      case FramebufferTextureFormat::RGBA32F:
        AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples,
                           GL_RGBA32F, GL_RGBA, GL_FLOAT, m_Specification.Width,
                           m_Specification.Height, static_cast<int>(i));
        break;

      case FramebufferTextureFormat::RED_INTEGER:
        AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples,
                           GL_R32I, GL_RED_INTEGER, GL_INT,
                           m_Specification.Width, m_Specification.Height,
                           static_cast<int>(i));
        break;

      default:
        BE_CORE_ERROR("Unknown color attachment format!");
        break;
      }
    }
  }

  // Create depth attachment
  if (m_DepthAttachmentSpec.TextureFormat != FramebufferTextureFormat::None) {
    CreateTextures(multisample, &m_DepthAttachment, 1);
    BindTexture(multisample, m_DepthAttachment);

    switch (m_DepthAttachmentSpec.TextureFormat) {
    case FramebufferTextureFormat::DEPTH24STENCIL8:
      AttachDepthTexture(m_DepthAttachment, m_Specification.Samples,
                         GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT,
                         m_Specification.Width, m_Specification.Height);
      break;

    case FramebufferTextureFormat::DEPTH32F:
      AttachDepthTexture(m_DepthAttachment, m_Specification.Samples,
                         GL_DEPTH_COMPONENT32F, GL_DEPTH_ATTACHMENT,
                         m_Specification.Width, m_Specification.Height);
      break;

    default:
      BE_CORE_ERROR("Unknown depth attachment format!");
      break;
    }
  }

  // Configure draw buffers
  if (m_ColorAttachments.size() > 1) {
    BE_CORE_ASSERT(m_ColorAttachments.size() <= s_MaxColorAttachments,
                   "Too many color attachments!");

    std::vector<GLenum> buffers(m_ColorAttachments.size());
    for (size_t i = 0; i < m_ColorAttachments.size(); i++) {
      buffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
    }
    glDrawBuffers(static_cast<GLsizei>(buffers.size()), buffers.data());
  } else if (m_ColorAttachments.empty()) {
    // Depth-only framebuffer
    glDrawBuffer(GL_NONE);
  }

  // Verify framebuffer completeness
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    const char *errorMsg = "Unknown error";
    switch (status) {
    case GL_FRAMEBUFFER_UNDEFINED:
      errorMsg = "GL_FRAMEBUFFER_UNDEFINED";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      errorMsg = "GL_FRAMEBUFFER_UNSUPPORTED";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      errorMsg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
      break;
    }
    BE_CORE_CRITICAL("Framebuffer is incomplete: {}", errorMsg);
    BE_CORE_ASSERT(false, "Framebuffer is not complete!");
  }

  // Unbind framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  BE_CORE_INFO("OpenGL Framebuffer created (ID: {}, {}x{}, {} color "
               "attachments, depth: {})",
               m_RendererID, m_Specification.Width, m_Specification.Height,
               m_ColorAttachments.size(),
               m_DepthAttachment != 0 ? "yes" : "no");
}

void OpenGLFramebuffer::Destroy() {
  if (m_RendererID != 0) {
    glDeleteFramebuffers(1, &m_RendererID);
    m_RendererID = 0;
  }

  if (!m_ColorAttachments.empty()) {
    glDeleteTextures(static_cast<GLsizei>(m_ColorAttachments.size()),
                     m_ColorAttachments.data());
    m_ColorAttachments.clear();
  }

  if (m_DepthAttachment != 0) {
    glDeleteTextures(1, &m_DepthAttachment);
    m_DepthAttachment = 0;
  }

  BE_CORE_TRACE("OpenGL Framebuffer destroyed");
}

void OpenGLFramebuffer::Bind() {
  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
  glViewport(0, 0, m_Specification.Width, m_Specification.Height);
}

void OpenGLFramebuffer::Unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height) {
  // Validate dimensions
  if (width == 0 || height == 0 || width > s_MaxFramebufferSize ||
      height > s_MaxFramebufferSize) {
    BE_CORE_WARN("Attempted to resize framebuffer to {}x{} - invalid!", width,
                 height);
    return;
  }

  // Skip if size hasn't changed
  if (width == m_Specification.Width && height == m_Specification.Height) {
    return;
  }

  m_Specification.Width = width;
  m_Specification.Height = height;

  BE_CORE_TRACE("Resizing framebuffer to {}x{}", width, height);
  Invalidate();
}

int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y) {
  BE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(),
                 "Attachment index out of range!");

  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
  glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

  int pixelData = -1;
  glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  return pixelData;
}

void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value) {
  BE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size(),
                 "Attachment index out of range!");

  auto &spec = m_ColorAttachmentSpecs[attachmentIndex];

  glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

  if (spec.TextureFormat == FramebufferTextureFormat::RED_INTEGER) {
    glClearBufferiv(GL_COLOR, attachmentIndex, &value);
  } else {
    // For floating point formats, convert int to float
    float floatValue = static_cast<float>(value);
    float clearColor[4] = {floatValue, floatValue, floatValue, 1.0f};
    glClearBufferfv(GL_COLOR, attachmentIndex, clearColor);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

uint32_t OpenGLFramebuffer::GetColorAttachmentRendererID(uint32_t index) const {
  BE_CORE_ASSERT(index < m_ColorAttachments.size(),
                 "Color attachment index out of range!");
  return m_ColorAttachments[index];
}

bool OpenGLFramebuffer::IsDepthFormat(FramebufferTextureFormat format) {
  switch (format) {
  case FramebufferTextureFormat::DEPTH24STENCIL8:
  case FramebufferTextureFormat::DEPTH32F:
    return true;
  default:
    return false;
  }
}

} // namespace BeEngine
