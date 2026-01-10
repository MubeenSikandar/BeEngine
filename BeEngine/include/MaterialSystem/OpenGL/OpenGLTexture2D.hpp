#pragma once

#include <MaterialSystem/Texture.hpp>
#include <cstdint>
#include <glad/glad.h>

namespace BeEngine {

/**
 * @brief OpenGL 2D texture implementation
 */
class OpenGLTexture2D : public Texture2D {
public:
  // Create from file
  OpenGLTexture2D(const std::filesystem::path &path, bool sRGB = true);

  // Create from specification
  OpenGLTexture2D(const TextureSpecification &spec, const void *data = nullptr);

  OpenGLTexture2D(uint32_t width, uint32_t height, bool hasAlpha, bool sRGB);

  ~OpenGLTexture2D() override;

  // Disable copy
  OpenGLTexture2D(const OpenGLTexture2D &) = delete;
  OpenGLTexture2D &operator=(const OpenGLTexture2D &) = delete;

  // Move semantics
  OpenGLTexture2D(OpenGLTexture2D &&other) noexcept;
  OpenGLTexture2D &operator=(OpenGLTexture2D &&other) noexcept;

  // Texture interface
  void Bind(uint32_t slot = 0) const override;
  void Unbind() const override;

  NODISCARD uint32_t GetWidth() const override { return m_Specification.Width; }
  NODISCARD uint32_t GetHeight() const override {
    return m_Specification.Height;
  }
  NODISCARD uint32_t GetRendererID() const override { return m_RendererID; }
  NODISCARD TextureFormat GetFormat() const override {
    return m_Specification.Format;
  }
  NODISCARD TextureType GetType() const override {
    return TextureType::Texture2D;
  }
  NODISCARD const TextureSpecification &GetSpecification() const override {
    return m_Specification;
  }
  NODISCARD const std::string &GetPath() const override { return m_FilePath; }

  void SetData(const void *data, uint32_t size) override;
  void SetSubData(const void *data, uint32_t xOffset, uint32_t yOffset,
                  uint32_t width, uint32_t height) override;

  bool operator==(const Texture &other) const override {
    return m_RendererID ==
           static_cast<const OpenGLTexture2D &>(other).m_RendererID;
  }

  NODISCARD bool IsLoaded() const override { return m_IsLoaded; }

  static GLenum ToOpenGLFormat(TextureFormat format);
  static GLenum ToOpenGLInternalFormat(TextureFormat format, bool sRGB);
  static GLenum ToOpenGLDataType(TextureFormat format);
  static GLenum ToOpenGLFilter(TextureFilter filter);
  static GLenum ToOpenGLWrap(TextureWrap wrap);

private:
  void CreateTexture(const void *data);
  void SetupTextureParameters();

  uint32_t m_RendererID = 0;
  TextureSpecification m_Specification;
  std::string m_FilePath;
  bool m_IsLoaded = false;

  GLenum m_InternalFormat = GL_RGBA8;
  GLenum m_DataFormat = GL_RGBA;
  GLenum m_DataType = GL_UNSIGNED_BYTE;

  bool m_HasAlpha{};
  uint32_t m_Width{};
  uint32_t m_Height{};
  bool m_sRGB{true};
};

/**
 * @brief OpenGL Cubemap texture implementation
 */
class OpenGLTextureCube : public TextureCube {
public:
  // Create from 6 face images
  OpenGLTextureCube(const std::array<std::filesystem::path, 6> &facePaths);

  // Create from specification
  OpenGLTextureCube(const TextureSpecification &spec);

  ~OpenGLTextureCube() override;

  void Bind(uint32_t slot = 0) const override;
  void Unbind() const override;

  NODISCARD uint32_t GetWidth() const override { return m_Specification.Width; }
  NODISCARD uint32_t GetHeight() const override {
    return m_Specification.Height;
  }
  NODISCARD uint32_t GetRendererID() const override { return m_RendererID; }
  NODISCARD TextureFormat GetFormat() const override {
    return m_Specification.Format;
  }
  NODISCARD TextureType GetType() const override {
    return TextureType::TextureCube;
  }
  NODISCARD const TextureSpecification &GetSpecification() const override {
    return m_Specification;
  }
  NODISCARD const std::string &GetPath() const override { return m_DebugName; }

  void SetData(const void *data, uint32_t size) override;
  void SetSubData(const void *data, uint32_t xOffset, uint32_t yOffset,
                  uint32_t width, uint32_t height) override;

  bool operator==(const Texture &other) const override {
    return m_RendererID ==
           static_cast<const OpenGLTextureCube &>(other).m_RendererID;
  }

  NODISCARD bool IsLoaded() const override { return m_IsLoaded; }

private:
  uint32_t m_RendererID = 0;
  TextureSpecification m_Specification;
  std::string m_DebugName;
  bool m_IsLoaded = false;
};

} // namespace BeEngine
