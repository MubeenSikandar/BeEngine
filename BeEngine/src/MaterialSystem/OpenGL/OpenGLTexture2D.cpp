#include "PCH/BeEnginePCH.hpp"

namespace BeEngine {

// ============================================================================
// Format Conversion Helpers
// ============================================================================

GLenum OpenGLTexture2D::ToOpenGLFormat(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8:
  case TextureFormat::R16F:
  case TextureFormat::R32F:
    return GL_RED;

  case TextureFormat::RG8:
  case TextureFormat::RG16F:
  case TextureFormat::RG32F:
    return GL_RG;

  case TextureFormat::RGB8:
  case TextureFormat::RGB16F:
  case TextureFormat::RGB32F:
    return GL_RGB;

  case TextureFormat::RGBA8:
  case TextureFormat::RGBA16F:
  case TextureFormat::RGBA32F:
    return GL_RGBA;

  case TextureFormat::Depth16:
  case TextureFormat::Depth24:
  case TextureFormat::Depth32F:
    return GL_DEPTH_COMPONENT;

  case TextureFormat::Depth24Stencil8:
    return GL_DEPTH_STENCIL;

  case TextureFormat::None:
  default:
    return GL_RGBA;
  }
}

GLenum OpenGLTexture2D::ToOpenGLInternalFormat(TextureFormat format,
                                               bool sRGB) {
  switch (format) {
  case TextureFormat::R8:
    return GL_R8;
  case TextureFormat::RG8:
    return GL_RG8;
  case TextureFormat::RGB8:
    return sRGB ? GL_SRGB8 : GL_RGB8;
  case TextureFormat::RGBA8:
    return sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
  case TextureFormat::R16F:
    return GL_R16F;
  case TextureFormat::RG16F:
    return GL_RG16F;
  case TextureFormat::RGB16F:
    return GL_RGB16F;
  case TextureFormat::RGBA16F:
    return GL_RGBA16F;
  case TextureFormat::R32F:
    return GL_R32F;
  case TextureFormat::RG32F:
    return GL_RG32F;
  case TextureFormat::RGB32F:
    return GL_RGB32F;
  case TextureFormat::RGBA32F:
    return GL_RGBA32F;
  case TextureFormat::Depth16:
    return GL_DEPTH_COMPONENT16;
  case TextureFormat::Depth24:
    return GL_DEPTH_COMPONENT24;
  case TextureFormat::Depth32F:
    return GL_DEPTH_COMPONENT32F;
  case TextureFormat::Depth24Stencil8:
    return GL_DEPTH24_STENCIL8;
  case TextureFormat::None:
  default:
    return GL_RGBA8;
  }
}

GLenum OpenGLTexture2D::ToOpenGLDataType(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8:
  case TextureFormat::RG8:
  case TextureFormat::RGB8:
  case TextureFormat::RGBA8:
    return GL_UNSIGNED_BYTE;

  case TextureFormat::R16F:
  case TextureFormat::RG16F:
  case TextureFormat::RGB16F:
  case TextureFormat::RGBA16F:
    return GL_HALF_FLOAT;

  case TextureFormat::R32F:
  case TextureFormat::RG32F:
  case TextureFormat::RGB32F:
  case TextureFormat::RGBA32F:
  case TextureFormat::Depth32F:
    return GL_FLOAT;

  case TextureFormat::Depth16:
    return GL_UNSIGNED_SHORT;

  case TextureFormat::Depth24:
    return GL_UNSIGNED_INT;

  case TextureFormat::Depth24Stencil8:
    return GL_UNSIGNED_INT_24_8;

  case TextureFormat::None:
  default:
    return GL_UNSIGNED_BYTE;
  }
}

GLenum OpenGLTexture2D::ToOpenGLFilter(TextureFilter filter) {
  switch (filter) {
  case TextureFilter::Nearest:
    return GL_NEAREST;
  case TextureFilter::Linear:
    return GL_LINEAR;
  case TextureFilter::NearestMipMapNearest:
    return GL_NEAREST_MIPMAP_NEAREST;
  case TextureFilter::LinearMipMapNearest:
    return GL_LINEAR_MIPMAP_NEAREST;
  case TextureFilter::NearestMipMapLinear:
    return GL_NEAREST_MIPMAP_LINEAR;
  case TextureFilter::LinearMipMapLinear:
    return GL_LINEAR_MIPMAP_LINEAR;
  default:
    return GL_LINEAR;
  }
}

GLenum OpenGLTexture2D::ToOpenGLWrap(TextureWrap wrap) {
  switch (wrap) {
  case TextureWrap::Repeat:
    return GL_REPEAT;
  case TextureWrap::MirroredRepeat:
    return GL_MIRRORED_REPEAT;
  case TextureWrap::ClampToEdge:
    return GL_CLAMP_TO_EDGE;
  case TextureWrap::ClampToBorder:
    return GL_CLAMP_TO_BORDER;
  default:
    return GL_REPEAT;
  }
}

// ============================================================================
// OpenGLTexture2D Implementation
// ============================================================================

OpenGLTexture2D::OpenGLTexture2D(const std::filesystem::path &path, bool sRGB) {
  m_FilePath = path.string();
  m_Specification.sRGB = sRGB;
  m_Specification.DebugName = path.filename().string();

  // Determine if HDR
  bool isHDR = ImageLoader::IsHDRFormat(path);

  // Load image data
  ImageData imageData;
  if (isHDR) {
    imageData = ImageLoader::LoadHDR(path, true);
  } else {
    imageData = ImageLoader::Load(path, true);
  }

  if (!imageData.IsValid) {
    BE_CORE_ERROR("Failed to load texture: {}", m_FilePath);
    // Create a magenta error texture
    m_Specification.Width = 2;
    m_Specification.Height = 2;
    m_Specification.Format = TextureFormat::RGBA8;

    std::array<uint8_t, 16> errorPixels = {255, 0, 255, 255, 255, 0, 255, 255,
                                           255, 0, 255, 255, 255, 0, 255, 255};
    CreateTexture(errorPixels.data());
    return;
  }

  m_Specification.Width = imageData.Width;
  m_Specification.Height = imageData.Height;

  // Determine format based on channels
  if (isHDR) {
    switch (imageData.Channels) {
    case 1:
      m_Specification.Format = TextureFormat::R32F;
      break;
    case 2:
      m_Specification.Format = TextureFormat::RG32F;
      break;
    case 3:
      m_Specification.Format = TextureFormat::RGB32F;
      break;
    case 4:
      m_Specification.Format = TextureFormat::RGBA32F;
      break;
    }
    m_Specification.sRGB = false; // HDR is linear
  } else {
    switch (imageData.Channels) {
    case 1:
      m_Specification.Format = TextureFormat::R8;
      break;
    case 2:
      m_Specification.Format = TextureFormat::RG8;
      break;
    case 3:
      m_Specification.Format = TextureFormat::RGB8;
      break;
    case 4:
      m_Specification.Format = TextureFormat::RGBA8;
      break;
    }
  }

  CreateTexture(imageData.Pixels);
  ImageLoader::Free(imageData);

  m_IsLoaded = true;

  BE_CORE_INFO("Created texture: {} ({}x{}, {})", m_Specification.DebugName,
               m_Specification.Width, m_Specification.Height,
               imageData.Channels);
}

OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification &spec,
                                 const void *data)
    : m_Specification(spec) {

  m_FilePath = "";
  CreateTexture(data);
  m_IsLoaded = (data != nullptr);

  BE_CORE_TRACE("Created texture from spec: {} ({}x{})",
                m_Specification.DebugName, m_Specification.Width,
                m_Specification.Height);
}

OpenGLTexture2D::~OpenGLTexture2D() {
  if (m_RendererID != 0) {
    glDeleteTextures(1, &m_RendererID);
    BE_CORE_TRACE("Deleted texture: {} (ID: {})", m_Specification.DebugName,
                  m_RendererID);
  }
}

OpenGLTexture2D::OpenGLTexture2D(OpenGLTexture2D &&other) noexcept
    : m_RendererID(other.m_RendererID),
      m_Specification(std::move(other.m_Specification)),
      m_FilePath(std::move(other.m_FilePath)), m_IsLoaded(other.m_IsLoaded),
      m_InternalFormat(other.m_InternalFormat),
      m_DataFormat(other.m_DataFormat), m_DataType(other.m_DataType) {
  other.m_RendererID = 0;
  other.m_IsLoaded = false;
}

OpenGLTexture2D &OpenGLTexture2D::operator=(OpenGLTexture2D &&other) noexcept {
  if (this != &other) {
    if (m_RendererID != 0) {
      glDeleteTextures(1, &m_RendererID);
    }

    m_RendererID = other.m_RendererID;
    m_Specification = std::move(other.m_Specification);
    m_FilePath = std::move(other.m_FilePath);
    m_IsLoaded = other.m_IsLoaded;
    m_InternalFormat = other.m_InternalFormat;
    m_DataFormat = other.m_DataFormat;
    m_DataType = other.m_DataType;

    other.m_RendererID = 0;
    other.m_IsLoaded = false;
  }
  return *this;
}

void OpenGLTexture2D::CreateTexture(const void *data) {
  m_InternalFormat =
      ToOpenGLInternalFormat(m_Specification.Format, m_Specification.sRGB);
  m_DataFormat = ToOpenGLFormat(m_Specification.Format);
  m_DataType = ToOpenGLDataType(m_Specification.Format);

  // Create texture using DSA if available
  if (GLAD_GL_VERSION_4_5) {
    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);

    // Allocate storage
    uint32_t mipLevels = m_Specification.GenerateMipmaps
                             ? CalculateMipMapCount(m_Specification.Width,
                                                    m_Specification.Height)
                             : 1;

    glTextureStorage2D(m_RendererID, mipLevels, m_InternalFormat,
                       m_Specification.Width, m_Specification.Height);

    // Upload data if provided
    if (data) {
      glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Specification.Width,
                          m_Specification.Height, m_DataFormat, m_DataType,
                          data);
    }

    // Set parameters
    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER,
                        ToOpenGLFilter(m_Specification.MinFilter));
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER,
                        ToOpenGLFilter(m_Specification.MagFilter));
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S,
                        ToOpenGLWrap(m_Specification.WrapS));
    glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T,
                        ToOpenGLWrap(m_Specification.WrapT));

    if (m_Specification.WrapS == TextureWrap::ClampToBorder ||
        m_Specification.WrapT == TextureWrap::ClampToBorder) {

      const std::array<float, 4> borderColor = {
          m_Specification.BorderColor.r, m_Specification.BorderColor.g,
          m_Specification.BorderColor.b, m_Specification.BorderColor.a};

      glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR,
                           borderColor.data());
    }

    // Generate mipmaps
    if (m_Specification.GenerateMipmaps && data) {
      glGenerateTextureMipmap(m_RendererID);
    }
  } else {
    // Fallback for older OpenGL
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);

    glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Specification.Width,
                 m_Specification.Height, 0, m_DataFormat, m_DataType, data);

    SetupTextureParameters();

    if (m_Specification.GenerateMipmaps && data) {
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void OpenGLTexture2D::SetupTextureParameters() {
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  ToOpenGLFilter(m_Specification.MinFilter));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                  ToOpenGLFilter(m_Specification.MagFilter));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  ToOpenGLWrap(m_Specification.WrapS));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                  ToOpenGLWrap(m_Specification.WrapT));

  if (m_Specification.WrapS == TextureWrap::ClampToBorder ||
      m_Specification.WrapT == TextureWrap::ClampToBorder) {
    const std::array<float, 4> borderColor = {
        m_Specification.BorderColor.r, m_Specification.BorderColor.g,
        m_Specification.BorderColor.b, m_Specification.BorderColor.a};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,
                     borderColor.data());
  }
}

void OpenGLTexture2D::Bind(uint32_t slot) const {
  if (GLAD_GL_VERSION_4_5) {
    glBindTextureUnit(slot, m_RendererID);
  } else {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
  }
}

void OpenGLTexture2D::Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }

void OpenGLTexture2D::SetData(const void *data, uint32_t size) {
  uint32_t bpp = GetBytesPerPixel(m_Specification.Format);
  uint32_t expectedSize = m_Specification.Width * m_Specification.Height * bpp;

  BE_CORE_ASSERT(size == expectedSize,
                 "Data size must match texture dimensions!");

  if (GLAD_GL_VERSION_4_5) {
    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Specification.Width,
                        m_Specification.Height, m_DataFormat, m_DataType, data);

    if (m_Specification.GenerateMipmaps) {
      glGenerateTextureMipmap(m_RendererID);
    }
  } else {
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_Specification.Width,
                    m_Specification.Height, m_DataFormat, m_DataType, data);

    if (m_Specification.GenerateMipmaps) {
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  m_IsLoaded = true;
}

void OpenGLTexture2D::SetSubData(const void *data, uint32_t xOffset,
                                 uint32_t yOffset, uint32_t width,
                                 uint32_t height) {
  BE_CORE_ASSERT(xOffset + width <= m_Specification.Width &&
                     yOffset + height <= m_Specification.Height,
                 "Sub-region exceeds texture bounds!");

  if (GLAD_GL_VERSION_4_5) {
    glTextureSubImage2D(m_RendererID, 0, xOffset, yOffset, width, height,
                        m_DataFormat, m_DataType, data);
  } else {
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, yOffset, width, height,
                    m_DataFormat, m_DataType, data);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

// ============================================================================
// OpenGLTextureCube Implementation
// ============================================================================

OpenGLTextureCube::OpenGLTextureCube(
    const std::array<std::filesystem::path, 6> &facePaths) {
  m_DebugName = "Cubemap";

  glGenTextures(1, &m_RendererID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

  // Face order: +X, -X, +Y, -Y, +Z, -Z
  for (uint32_t i = 0; i < 6; i++) {
    ImageData imageData = ImageLoader::Load(facePaths[i], false);

    if (!imageData.IsValid) {
      BE_CORE_ERROR("Failed to load cubemap face {}: {}", i,
                    facePaths[i].string());
      continue;
    }

    if (i == 0) {
      m_Specification.Width = imageData.Width;
      m_Specification.Height = imageData.Height;
    }

    GLenum format = GL_RGB;
    GLenum internalFormat = GL_SRGB8;

    switch (imageData.Channels) {
    case 1:
      format = GL_RED;
      internalFormat = GL_R8;
      break;
    case 3:
      format = GL_RGB;
      internalFormat = GL_SRGB8;
      break;
    case 4:
      format = GL_RGBA;
      internalFormat = GL_SRGB8_ALPHA8;
      break;
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
                 imageData.Width, imageData.Height, 0, format, GL_UNSIGNED_BYTE,
                 imageData.Pixels);

    ImageLoader::Free(imageData);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

  m_IsLoaded = true;
  m_Specification.Format = TextureFormat::RGBA8;

  BE_CORE_INFO("Created cubemap texture ({}x{})", m_Specification.Width,
               m_Specification.Height);
}

OpenGLTextureCube::OpenGLTextureCube(const TextureSpecification &spec)
    : m_Specification(spec) {
  m_DebugName = spec.DebugName;

  glGenTextures(1, &m_RendererID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);

  GLenum internalFormat =
      OpenGLTexture2D::ToOpenGLInternalFormat(spec.Format, spec.sRGB);
  GLenum format = OpenGLTexture2D::ToOpenGLFormat(spec.Format);
  GLenum dataType = OpenGLTexture2D::ToOpenGLDataType(spec.Format);

  for (uint32_t i = 0; i < 6; i++) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
                 spec.Width, spec.Height, 0, format, dataType, nullptr);
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  OpenGLTexture2D::ToOpenGLFilter(spec.MinFilter));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER,
                  OpenGLTexture2D::ToOpenGLFilter(spec.MagFilter));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                  OpenGLTexture2D::ToOpenGLWrap(spec.WrapS));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                  OpenGLTexture2D::ToOpenGLWrap(spec.WrapT));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                  OpenGLTexture2D::ToOpenGLWrap(spec.WrapR));

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

OpenGLTextureCube::~OpenGLTextureCube() {
  if (m_RendererID != 0) {
    glDeleteTextures(1, &m_RendererID);
  }
}

void OpenGLTextureCube::Bind(uint32_t slot) const {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
}

void OpenGLTextureCube::Unbind() const {
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void OpenGLTextureCube::SetData(const void *data, uint32_t size) {
  // Not typically used for cubemaps
  BE_CORE_WARN("SetData not implemented for cubemap textures");
}

void OpenGLTextureCube::SetSubData(const void *data, uint32_t xOffset,
                                   uint32_t yOffset, uint32_t width,
                                   uint32_t height) {
  // Not typically used for cubemaps
  BE_CORE_WARN("SetSubData not implemented for cubemap textures");
}

} // namespace BeEngine
