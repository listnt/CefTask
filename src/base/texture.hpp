#ifndef TEXTURE_H
#define TEXTURE_H
#include "base.hpp"
#include <cstdio>
#include <iostream>

class Texture {
protected:
  GLuint ObjectTexture;

public:
  Texture(){};

  bool Init(GLsizei width, GLsizei height, GLint internalFormat, GLenum format);

  void Bind(GLenum TextureUnit);
};

#endif // PICKINGTEXTURE_H
