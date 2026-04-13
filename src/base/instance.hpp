//
// Created by aidar on 16.04.25.
//

#ifndef INSTANCE_H
#define INSTANCE_H

#include <algorithm>
#include <math.h>
#include <vector>

#include "base.hpp"

class model {
protected:
  std::vector<Vector2f> points;
  std::vector<Vector4f> colors;
  std::vector<GLint> triangles;
  std::vector<Vector2f> uv;

public:
  model(const std::vector<Vector2f> &points,
        const std::vector<Vector4f> &colors,
        const std::vector<GLint> &triangles, const std::vector<Vector2f> &uv)
      : points(points), colors(colors), triangles(triangles), uv(uv) {}

  model() = default;

  std::vector<Vector2f> getPoints() { return points; }

  std::vector<Vector4f> getColors() { return colors; }

  std::vector<GLint> getTriangles() { return triangles; }

  std::vector<Vector2f> getUVs() { return uv; }
};

class instance : public model {
protected:
  GLuint VBO = 0;
  GLuint VAO = 0;
  GLuint EBO = 0;
  GLuint Colors = 0;
  GLuint UV = 0;

  GLint ViewModelMat;

public:
  Vector3f T, S, R;

  instance() : T(0, 0, 0), S(1, 1, 1), R(0, 0, 0){};

  void LoadGLBuffers();

  void loadModel(model *obj) {
    if (VAO != 0) {
      glDeleteBuffers(1, &EBO);
      glDeleteBuffers(1, &VBO);
      glDeleteBuffers(1, &Colors);
      glDeleteBuffers(1, &VAO);
      glDeleteBuffers(1, &UV);
    }

    points = obj->getPoints();
    colors = obj->getColors();
    uv = obj->getUVs();
  }

  ~instance() {
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &UV);
    glDeleteBuffers(1, &Colors);
    glDeleteBuffers(1, &VAO);
  }

  virtual void Render(Matrix4x4 View);

  void Transform(Vector3f T);

  void Rotate(Vector3f R);

  void Scale(Vector3f S);
};

#endif // INSTANCE_H
