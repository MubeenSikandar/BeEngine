#pragma once

#include "Core.hpp"
#include <cstdint>
#include <filesystem>
#include <glm/glm.hpp>
#include <string>

namespace BeEngine {

/**
 * @brief Texture format specification
 */
enum class TextureFormat : uint8_t {
  None = 0,

  // Standard formats
  R8,    // Single channel (grayscale)
  RG8,   // Two channels
  RGB8,  // Three channels (no alpha)
  RGBA8, // Four channels (with alpha)

  // High precision
  R16F,
  RG16F,
  RGB16F,
  RGBA16F,

  // HDR formats
  R32F,
  RG32F,
  RGB32F,
  RGBA32F,

  // Depth formats
  Depth16,
  Depth24,
  Depth32F,
  Depth24Stencil8
};

/**
 * @brief Texture filtering mode
 */
enum class TextureFilter : uint8_t {
  Nearest, // No interpolation (pixelated)
  Linear,  // Bilinear interpolation (smooth)
  NearestMipMapNearest,
  LinearMipMapNearest,
  NearestMipMapLinear,
  LinearMipMapLinear // Trilinear (best quality)
};

/**
 * @brief Texture wrapping mode
 */
enum class TextureWrap : uint8_t {
  Repeat,         // Tile the texture
  MirroredRepeat, // Tile with mirroring
  ClampToEdge,    // Stretch edge pixels
  ClampToBorder   // Use border color
};

/**
 * @brief Texture type
 */
enum class TextureType : uint8_t {
  Texture2D,
  TextureCube,
  Texture2DArray,
  Texture3D
};

/**
 * @brief Specification for creating a texture
 */
struct TextureSpecification {
  uint32_t Width = 1;
  uint32_t Height = 1;
  uint32_t Depth = 1;  // For 3D textures
  uint32_t Layers = 1; // For array textures

  TextureFormat Format = TextureFormat::RGBA8;
  TextureType Type = TextureType::Texture2D;

  TextureFilter MinFilter = TextureFilter::LinearMipMapLinear;
  TextureFilter MagFilter = TextureFilter::Linear;
  TextureWrap WrapS = TextureWrap::Repeat;
  TextureWrap WrapT = TextureWrap::Repeat;
  TextureWrap WrapR = TextureWrap::Repeat; // For 3D/Cube textures

  bool GenerateMipmaps = true;
  bool sRGB = true; // Gamma-correct color textures

  glm::vec4 BorderColor = {0.0F, 0.0F, 0.0F, 1.0F};

  std::string DebugName = "Unnamed Texture";
};

/**
 * @brief Image data loaded from file
 */
struct ImageData {
  uint8_t *Pixels = nullptr;
  uint32_t Width = 0;
  uint32_t Height = 0;
  uint32_t Channels = 0;
  bool IsHDR = false;
  bool IsValid = false;

  ~ImageData() {
    if (Pixels) {
      // Will be freed by stb_image_free
    }
  }
};

/**
 * @brief Abstract texture interface
 */
class BE_API Texture {
public:
  virtual ~Texture() = default;

  // Binding
  virtual void Bind(uint32_t slot = 0) const = 0;
  virtual void Unbind() const = 0;

  // Properties
  NODISCARD virtual uint32_t GetWidth() const = 0;
  NODISCARD virtual uint32_t GetHeight() const = 0;
  NODISCARD virtual uint32_t GetRendererID() const = 0;
  NODISCARD virtual TextureFormat GetFormat() const = 0;
  NODISCARD virtual TextureType GetType() const = 0;
  NODISCARD virtual const TextureSpecification &GetSpecification() const = 0;
  NODISCARD virtual const std::string &GetPath() const = 0;

  // Data upload
  virtual void SetData(const void *data, uint32_t size) = 0;
  virtual void SetSubData(const void *data, uint32_t xOffset, uint32_t yOffset,
                          uint32_t width, uint32_t height) = 0;

  // Comparison (for material sorting)
  virtual bool operator==(const Texture &other) const = 0;

  // Utility
  NODISCARD virtual bool IsLoaded() const = 0;
  NODISCARD static uint32_t CalculateMipMapCount(uint32_t width,
                                                 uint32_t height);
  NODISCARD static uint32_t GetBytesPerPixel(TextureFormat format);
  NODISCARD static uint32_t GetChannelCount(TextureFormat format);
};

/**
 * @brief 2D Texture
 */
class BE_API Texture2D : public Texture {
public:
  // Create from file
  static std::shared_ptr<Texture2D> Create(const std::filesystem::path &path,
                                           bool sRGB = true);

  // Create from specification (empty or with data)
  static std::shared_ptr<Texture2D> Create(const TextureSpecification &spec,
                                           const void *data = nullptr);

  // Create solid color texture
  static std::shared_ptr<Texture2D> CreateSolidColor(const glm::vec4 &color,
                                                     uint32_t width = 1,
                                                     uint32_t height = 1);

  // Create checkerboard texture (useful for debugging)
  static std::shared_ptr<Texture2D>
  CreateCheckerboard(uint32_t width = 64, uint32_t height = 64,
                     uint32_t checkSize = 8,
                     const glm::vec4 &color1 = {1, 1, 1, 1},
                     const glm::vec4 &color2 = {0.5F, 0.5F, 0.5F, 1});
};

/**
 * @brief Cubemap texture (for skyboxes, reflections)
 */
class BE_API TextureCube : public Texture {
public:
  // Create from 6 face images
  // Order: +X, -X, +Y, -Y, +Z, -Z
  static std::shared_ptr<TextureCube>
  Create(const std::array<std::filesystem::path, 6> &facePaths);

  // Create from single equirectangular HDR image
  static std::shared_ptr<TextureCube>
  CreateFromEquirectangular(const std::filesystem::path &hdrPath,
                            uint32_t resolution = 512);

  // Create from specification
  static std::shared_ptr<TextureCube> Create(const TextureSpecification &spec);
};

/**
 * @brief Image loading utilities
 */
class BE_API ImageLoader {
public:
  // Load image from file
  static ImageData Load(const std::filesystem::path &path,
                        bool flipVertically = true);

  // Load HDR image
  static ImageData LoadHDR(const std::filesystem::path &path,
                           bool flipVertically = true);

  // Free image data
  static void Free(ImageData &data);

  // Check if file is HDR format
  static bool IsHDRFormat(const std::filesystem::path &path);

  // Get supported extensions
  static std::vector<std::string> GetSupportedExtensions();
};
} // namespace BeEngine
