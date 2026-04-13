#include "app.hpp"

void App::initScene() {
  left = 0.0f / (1.0 * userData->WindowHeight / userData->WindowWidth);
  right = 72.0f / (1.0 * userData->WindowHeight / userData->WindowWidth);
  bottom = 0.0f;
  top = 72.0;

  projection = computeOrthoMatrix(left, right, bottom, top, -100, 100);
  camera = Matrix4x4();

  auto projectMat = glGetUniformLocation(userData->programObject, "projection");
  glUniformMatrix4fv(projectMat, 1, GL_FALSE, flatten(projection).data());
  auto ViewModelMat =
      glGetUniformLocation(userData->programObject, "viewModel");
  glUniformMatrix4fv(ViewModelMat, 1, GL_FALSE, flatten(camera).data());

  auto browserImg = glGetUniformLocation(userData->programObject, "browserImg");
  glUniform1i(browserImg, 0);

  auto y = glGetUniformLocation(userData->programObject, "y_texture");
  glUniform1i(y, 1);

  auto u = glGetUniformLocation(userData->programObject, "u_texture");
  glUniform1i(u, 2);

  auto v = glGetUniformLocation(userData->programObject, "v_texture");
  glUniform1i(v, 3);

  mode = glGetUniformLocation(userData->programObject, "mode");
  glUniform1i(mode, 0);

  glUseProgram(userData->programObject);

  cef = std::make_shared<Cef>();

  ffmpeg = std::make_shared<FFmpeg>();

  std::thread t1(&FFmpeg::FrameLoop, ffmpeg);
  t1.detach();
}

void App::Render() {
  glClearColor(0.2, 0.2, 0.2, 0.2); // background for color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniform1i(mode, 0);
  cef->Render(camera);

  glUniform1i(mode, 1);
  ffmpeg->Render(camera);

  eglSwapBuffers(userData->eglDisplay, userData->eglSurface);
}

App::~App() { cef.reset(); }