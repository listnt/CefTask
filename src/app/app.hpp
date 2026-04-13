#include <memory>
#include <thread>

#include "../base/base.hpp"
#include "../base/instance.hpp"
#include "../base/texture.hpp"
#include "cef.hpp"
#include "ffmpeg.hpp"

class App {
protected:
  std::shared_ptr<FFmpeg> ffmpeg;
  std::shared_ptr<Cef> cef;

  GLint mode = 0;

  float left = -25, right = 25, bottom = -25, top = 25;

  Matrix4x4 projection;
  Matrix4x4 camera;

public:
  ~App();
  void Render();

  void initScene();
};