#include "Texture.h"

#include "Debug.h"
#include "rw/rwcore.h"
#include "rw/rwplcore.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image.h"
// #include "stb/stb_image_write.h"

std::vector<Texture *> texturesToLoad;
std::unordered_map<std::string, RwTexture *> textureCache;

Texture::Texture(std::string src) {
  this->src = src;

  // CRITICAL: CSprite2d is a POD struct with no constructor — without this,
  // m_pTexture holds uninitialized garbage and the nullptr guards in
  // Render::DrawSprite / DrawTextureOnRadar are defeated, crashing the game
  // inside CSprite2d::Draw when the texture file fails to load.
  this->sprite.m_pTexture = nullptr;

  texturesToLoad.push_back(this);
  Textures::TryLoadTextures();
}

Texture::~Texture() {
  // ainda ta no cache, nao pode deletar
  // if(sprite.m_pTexture)
  // {
  //     delete sprite.m_pTexture;
  // }
}

RwTexture *LoadRwTextureFromFile(std::string file, std::string textureName,
                                 bool flipHorizontal) {
  // debug->AddLine("Loading texture: " + file);

  // Diagnose missing asset files explicitly (helps user fix install path)
  if (!file_exists(file)) {
    fileLog->Error("Texture file not found: " + file);
    return nullptr;
  }

  int width, height, channels;
  unsigned char *data =
      stbi_load(file.c_str(), &width, &height, &channels, STBI_rgb_alpha);

  if (!data) {
    fileLog->Error("Failed to load image: " + file);
    return nullptr;
  }

  // Create a raster
  RwRaster *raster = RwRasterCreate(width, height, 32,
                                    (RwInt32)rwRASTERTYPETEXTURE |
                                        (RwInt32)rwRASTERFORMAT8888);
  if (!raster) {
    fileLog->Error("Failed to create raster");
    stbi_image_free(data);
    return nullptr;
  }

  // Lock the raster to get access to its pixel data
  RwUInt8 *rasterPixels = (RwUInt8 *)RwRasterLock(raster, 0, rwRASTERLOCKWRITE);
  if (!rasterPixels) {
    fileLog->Error("Failed to lock raster");
    RwRasterDestroy(raster);
    stbi_image_free(data);
    return nullptr;
  }

  if (flipHorizontal) {
    // Copy the image data to the raster, flipping vertically
    int rowSize = width * 4; // 4 bytes per pixel (RGBA)
    for (int y = 0; y < height; ++y) {
      memcpy(rasterPixels + y * rowSize, data + (height - 1 - y) * rowSize,
             rowSize);
    }
  } else {
    // Copy the image data to the raster (assuming tightly packed)
    memcpy(rasterPixels, data, width * height * 4);
  }

  // Unlock the raster
  RwRasterUnlock(raster);

  // Create a texture from the raster
  RwTexture *texture = RwTextureCreate(raster);
  if (!texture) {
    fileLog->Error("Failed to create texture");
    RwRasterDestroy(raster);
    stbi_image_free(data);
    return nullptr;
  }

  // maybe sets the texture name
  memset(texture->name, 0, 32);
  textureName.copy(texture->name, 32);

  // Free the image data loaded by stb_image
  stbi_image_free(data);

  return texture;
}

void Textures::TryLoadTextures() {
  if (!g_hasFirstDrawn)
    return;

  for (auto texture : texturesToLoad) {
    // Verifica se já existe no cache
    auto it = textureCache.find(texture->src);
    if (it != textureCache.end()) {
      // Já está no cache — só reutiliza
      texture->sprite.m_pTexture = it->second;
      // menuDebug->AddLine("Texture reused from cache: " + texture->src,
      // CRGBA(0, 200, 200));
      continue;
    }

    // Carrega nova textura
    RwTexture *tex = LoadRwTextureFromFile(texture->src, "", true);
    if (tex == nullptr) {
      // Defense in depth: make sure a failed load can never leave a garbage
      // pointer behind (constructor already inits to nullptr).
      texture->sprite.m_pTexture = nullptr;
      menuDebug->AddLine("Error loading texture " + texture->src,
                         CRGBA(255, 0, 0));
      continue;
    }

    // Guarda no cache e aplica
    textureCache[texture->src] = tex;
    texture->sprite.m_pTexture = tex;

    // menuDebug->AddLine("Texture loaded and cached: " + texture->src, CRGBA(0,
    // 150, 0));
  }

  texturesToLoad.clear();
}

RwTexture *Textures::CreateEmptyTexture(int width, int height,
                                        const std::string &textureName) {
  // Cria raster RGBA (8888)
  RwRaster *raster = RwRasterCreate(width, height, 32,
                                    rwRASTERTYPETEXTURE | rwRASTERFORMAT8888);

  if (!raster) {
    fileLog->Error("Failed to create raster");
    return nullptr;
  }

  // Lock pra escrever pixels
  RwUInt8 *pixels = (RwUInt8 *)RwRasterLock(raster, 0, rwRASTERLOCKWRITE);
  if (!pixels) {
    fileLog->Error("Failed to lock raster");
    RwRasterDestroy(raster);
    return nullptr;
  }

  // 🔥 Inicializa com transparente (ou o que quiser)
  memset(pixels, 0, width * height * 4);

  // Unlock
  RwRasterUnlock(raster);

  // Cria textura
  RwTexture *texture = RwTextureCreate(raster);
  if (!texture) {
    fileLog->Error("Failed to create texture");
    RwRasterDestroy(raster);
    return nullptr;
  }

  // Nome
  memset(texture->name, 0, 32);
  textureName.copy(texture->name, 32);

  return texture;
}