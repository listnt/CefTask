#include "instance.hpp"

#include <cstring>
#include <numeric>

#include <chrono> // Import the ctime library

// void instanse::Rotate(Vector3f R) { this->R = this->R + R; }

void instance::LoadGLBuffers() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &UV);
  glGenBuffers(1, &Colors);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Vector2f), points.data(),
               GL_STATIC_DRAW);
  auto position = glGetAttribLocation(userData->programObject, "vPosition");
  glVertexAttribPointer(position, 2, GL_FLOAT, 0, sizeof(Vector2f), 0);
  glEnableVertexAttribArray(position);

  glBindBuffer(GL_ARRAY_BUFFER, Colors);
  glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(Vector4f), colors.data(),
               GL_STATIC_DRAW);
  auto color = glGetAttribLocation(userData->programObject, "vColor");
  glVertexAttribPointer(color, 4, GL_FLOAT, 0, sizeof(Vector4f), 0);
  glEnableVertexAttribArray(color);

  glBindBuffer(GL_ARRAY_BUFFER, UV);
  glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(Vector2f), uv.data(),
               GL_STATIC_DRAW);
  auto uvLoc = glGetAttribLocation(userData->programObject, "vUV");
  glVertexAttribPointer(uvLoc, 2, GL_FLOAT, 0, sizeof(Vector2f), 0);
  glEnableVertexAttribArray(uvLoc);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(GLint),
               triangles.data(), GL_DYNAMIC_DRAW);

  ViewModelMat = glGetUniformLocation(userData->programObject, "viewModel");

  glBindVertexArray(0);
}

void instance::Render(Matrix4x4 View) {
  glBindVertexArray(this->VAO);
  auto ViewModel = View * translate(T.x, T.y, T.z) * roll(R.z) * pitch(R.y) *
                   yaw(R.x) * scale(S.x, S.y, S.z);

  glUniformMatrix4fv(ViewModelMat, 1, GL_FALSE, flatten(ViewModel).data());

  glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void instance::Transform(Vector3f T) { this->T = T; }

void instance::Rotate(Vector3f R) { this->R = R; }

void instance::Scale(Vector3f S) { this->S = S; }
