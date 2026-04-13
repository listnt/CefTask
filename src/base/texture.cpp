//
// Created by aidar on 04.05.25.
//

#include "texture.hpp"

void Texture::Bind(GLenum TextureUnit) {
  glActiveTexture(TextureUnit);
  glBindTexture(GL_TEXTURE_2D, ObjectTexture);
}

bool Texture::Init(GLsizei width, GLsizei height, GLint internalFormat,
                   GLenum format) {
  glGenTextures(1, &ObjectTexture);
  glBindTexture(GL_TEXTURE_2D, ObjectTexture);
  std::cout << "create texture " << ObjectTexture << " with dims " << width
            << "x" << height << std::endl;

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
               GL_UNSIGNED_BYTE, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return true;
}
