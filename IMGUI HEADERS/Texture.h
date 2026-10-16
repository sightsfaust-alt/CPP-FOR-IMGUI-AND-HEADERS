#pragma once

#include "pch.h"
#include "rw/rwcore.h"

class Texture {
public:
  CSprite2d sprite;
  std::string src;

  Texture(std::string src);
  ~Texture();
};

inline Texture *g_textureLeft = nullptr;
inline Texture *g_textureRight = nullptr;

class Textures {
public:
  static void TryLoadTextures();
  static RwTexture *CreateEmptyTexture(int width, int height,
                                       const std::string &textureName);
};

inline RwTexture *LoadRwTextureFromFile(std::string file,
                                        std::string textureName,
                                        bool flipHorizontal);