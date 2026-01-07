#include "PCH/BeEnginePCH.hpp"
#include <stb_image.h>

namespace BeEngine {

// ============================================================================
// Texture Utilities
// ============================================================================
uint32_t Texture::CalculateMipMapCount(uint32_t width, uint32_t height) {
  return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) +
         1;
}

uint32_t Texture::GetBytesPerPixel(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8:
    return 1;
  case TextureFormat::RG8:
    return 2;
  case TextureFormat::RGB8:
    return 3;
  case TextureFormat::RGBA8:
    return 4;
  case TextureFormat::R16F:
    return 2;
  case TextureFormat::RG16F:
    return 4;
  case TextureFormat::RGB16F:
    return 6;
  case TextureFormat::RGBA16F:
    return 8;
  case TextureFormat::R32F:
    return 4;
  case TextureFormat::RG32F:
    return 8;
  case TextureFormat::RGB32F:
    return 12;
  case TextureFormat::RGBA32F:
    return 16;
  case TextureFormat::Depth16:
    return 2;
  case TextureFormat::Depth24:
    return 3;
  case TextureFormat::Depth32F:
    return 4;
  case TextureFormat::Depth24Stencil8:
    return 4;
  case TextureFormat::None:
    return 0;
  }
  return 0;
}

uint32_t Texture::GetChannelCount(TextureFormat format) {
  switch (format) {
  case TextureFormat::R8:
  case TextureFormat::R16F:
  case TextureFormat::R32F:
  case TextureFormat::Depth16:
  case TextureFormat::Depth24:
  case TextureFormat::Depth32F:
    return 1;

  case TextureFormat::RG8:
  case TextureFormat::RG16F:
  case TextureFormat::RG32F:
    return 2;

  case TextureFormat::RGB8:
  case TextureFormat::RGB16F:
  case TextureFormat::RGB32F:
    return 3;

  case TextureFormat::RGBA8:
  case TextureFormat::RGBA16F:
  case TextureFormat::RGBA32F:
  case TextureFormat::Depth24Stencil8:
    return 4;

  case TextureFormat::None:
    return 0;
  }
  return 0;
}

// ============================================================================
// Texture2D Factory
// ============================================================================

std::shared_ptr<Texture2D> Texture2D::Create(const std::filesystem::path &path,
                                             bool sRGB) {
  switch (RendererAPI::GetAPI()) {
  case RenderAPI::OpenGL:
    return std::make_shared<OpenGLTexture2D>(path, sRGB);

  case RenderAPI::Vulkan:
  case RenderAPI::DirectX11:
  case RenderAPI::DirectX12:
  case RenderAPI::Metal:
    BE_CORE_ASSERT(false, "Render API not yet supported!");
    return nullptr;

  case RenderAPI::None:
  default:
    BE_CORE_ASSERT(false, "No rendering API selected!");
    return nullptr;
  }
}

std::shared_ptr<Texture2D> Texture2D::Create(const TextureSpecification &spec,
                                             const void *data) {
  switch (RendererAPI::GetAPI()) {
  case RenderAPI::OpenGL:
    return std::make_shared<OpenGLTexture2D>(spec, data);

  case RenderAPI::Vulkan:
  case RenderAPI::DirectX11:
  case RenderAPI::DirectX12:
  case RenderAPI::Metal:
    BE_CORE_ASSERT(false, "Render API not yet supported!");
    return nullptr;

  case RenderAPI::None:
  default:
    BE_CORE_ASSERT(false, "No rendering API selected!");
    return nullptr;
  }
}

std::shared_ptr<Texture2D> Texture2D::CreateSolidColor(const glm::vec4 &color,
                                                       uint32_t width,
                                                       uint32_t height) {
  TextureSpecification spec;
  spec.Width = width;
  spec.Height = height;
  spec.Format = TextureFormat::RGBA8;
  spec.GenerateMipmaps = false;
  spec.DebugName = "SolidColor";

  // Create pixel data
  std::vector<uint8_t> pixels(width * height * 4);
  auto r = static_cast<uint8_t>(color.r * 255.0F);
  auto g = static_cast<uint8_t>(color.g * 255.0F);
  auto b = static_cast<uint8_t>(color.b * 255.0F);
  auto a = static_cast<uint8_t>(color.a * 255.0F);

  for (uint32_t i = 0; i < width * height; i++) {
    pixels[(i * 4) + 0] = r;
    pixels[(i * 4) + 1] = g;
    pixels[(i * 4) + 2] = b;
    pixels[(i * 4) + 3] = a;
  }

  return Create(spec, pixels.data());
}

std::shared_ptr<Texture2D>
Texture2D::CreateCheckerboard(uint32_t width, uint32_t height,
                              uint32_t checkSize, const glm::vec4 &color1,
                              const glm::vec4 &color2) {
  TextureSpecification spec;
  spec.Width = width;
  spec.Height = height;
  spec.Format = TextureFormat::RGBA8;
  spec.GenerateMipmaps = true;
  spec.DebugName = "Checkerboard";

  std::vector<uint8_t> pixels(width * height * 4);

  for (uint32_t y = 0; y < height; y++) {
    for (uint32_t x = 0; x < width; x++) {
      bool isColor1 = ((x / checkSize) + (y / checkSize)) % 2 == 0;
      const glm::vec4 &color = isColor1 ? color1 : color2;

      uint32_t index = (y * width + x) * 4;
      pixels[index + 0] = static_cast<uint8_t>(color.r * 255.0F);
      pixels[index + 1] = static_cast<uint8_t>(color.g * 255.0F);
      pixels[index + 2] = static_cast<uint8_t>(color.b * 255.0F);
      pixels[index + 3] = static_cast<uint8_t>(color.a * 255.0F);
    }
  }

  return Create(spec, pixels.data());
}

// ============================================================================
// ImageLoader
// ============================================================================

ImageData ImageLoader::Load(const std::filesystem::path &path,
                            bool flipVertically) {
  ImageData result;

  std::string pathStr = path.string();

  if (!std::filesystem::exists(path)) {
    BE_CORE_ERROR("Image file not found: {}", pathStr);
    return result;
  }

  stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

  int width, height, channels;
  result.Pixels = stbi_load(pathStr.c_str(), &width, &height, &channels, 0);

  if (result.Pixels) {
    result.Width = static_cast<uint32_t>(width);
    result.Height = static_cast<uint32_t>(height);
    result.Channels = static_cast<uint32_t>(channels);
    result.IsHDR = false;
    result.IsValid = true;

    BE_CORE_TRACE("Loaded image: {} ({}x{}, {} channels)", pathStr, width,
                  height, channels);
  } else {
    BE_CORE_ERROR("Failed to load image: {} - {}", pathStr,
                  stbi_failure_reason());
  }

  return result;
}

ImageData ImageLoader::LoadHDR(const std::filesystem::path &path,
                               bool flipVertically) {
  ImageData result;

  std::string pathStr = path.string();

  if (!std::filesystem::exists(path)) {
    BE_CORE_ERROR("HDR image file not found: {}", pathStr);
    return result;
  }

  stbi_set_flip_vertically_on_load(flipVertically ? 1 : 0);

  int width, height, channels;
  float *pixels = stbi_loadf(pathStr.c_str(), &width, &height, &channels, 0);

  if (pixels) {
    result.Pixels = reinterpret_cast<uint8_t *>(pixels);
    result.Width = static_cast<uint32_t>(width);
    result.Height = static_cast<uint32_t>(height);
    result.Channels = static_cast<uint32_t>(channels);
    result.IsHDR = true;
    result.IsValid = true;

    BE_CORE_TRACE("Loaded HDR image: {} ({}x{}, {} channels)", pathStr, width,
                  height, channels);
  } else {
    BE_CORE_ERROR("Failed to load HDR image: {} - {}", pathStr,
                  stbi_failure_reason());
  }

  return result;
}

void ImageLoader::Free(ImageData &data) {
  if (data.Pixels) {
    stbi_image_free(data.Pixels);
    data.Pixels = nullptr;
    data.IsValid = false;
  }
}

bool ImageLoader::IsHDRFormat(const std::filesystem::path &path) {
  std::string ext = path.extension().string();
  std::ranges::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return ext == ".hdr" || ext == ".exr";
}

std::vector<std::string> ImageLoader::GetSupportedExtensions() {
  return {".png", ".jpg", ".jpeg", ".bmp", ".tga",
          ".gif", ".psd", ".hdr",  ".pic", ".pnm"};
}

} // namespace BeEngine
