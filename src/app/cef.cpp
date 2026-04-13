#include "cef.hpp"

void RenderHandler::OnPaint(CefRefPtr<CefBrowser> browser,
                            PaintElementType type, const RectList &dirtyRects,
                            const void *buffer, int width, int height) {
  std::lock_guard<std::mutex> lock(data.mtx);

  memcpy(data.y, buffer, data.linewidth_y * data.height_y * 4);
  data.linewidth_y = width;
  data.height_y = height;

  data.is_new = true;
}

Cef::Cef() {
  lx = 64;
  ly = 48;

  points = {
      Vector2f(0.0, ly / 2.0),
      Vector2f(0.0, 3.0 * ly / 4.0),
      Vector2f(lx / 2.0, 3.0 * ly / 4.0),
      Vector2f(lx / 2.0, ly / 2.0),

      Vector2f(lx / 2.0, ly / 2.0),
      Vector2f(lx / 2.0, 3.0 * ly / 4.0),
      Vector2f(lx, 3.0 * ly / 4.0),
      Vector2f(lx, ly / 2.0),

      Vector2f(0.0, 3.0 * ly / 4.0),
      Vector2f(0.0, ly),
      Vector2f(lx / 2.0, ly),
      Vector2f(lx / 2.0, 3.0 * ly / 4.0),

      Vector2f(lx / 2.0, 3.0 * ly / 4.0),
      Vector2f(lx / 2.0, ly),
      Vector2f(lx, ly),
      Vector2f(lx, 3.0 * ly / 4.0),
  };

  triangles = {0,  1,  2,  0,  2,  3,

               4,  5,  6,  4,  6,  7,

               8,  9,  10, 8,  10, 11,

               12, 13, 14, 12, 14, 15};

  colors = {
      Vector4f(1, 1, 1, 1), Vector4f(1, 1, 1, 1),
      Vector4f(1, 1, 1, 1), Vector4f(1, 1, 1, 1),

      Vector4f(1, 0, 1, 1), Vector4f(1, 0, 1, 1),
      Vector4f(1, 0, 1, 1), Vector4f(1, 0, 1, 1),

      Vector4f(1, 0, 1, 1), Vector4f(1, 0, 1, 1),
      Vector4f(1, 0, 1, 1), Vector4f(1, 0, 1, 1),

      Vector4f(1, 1, 1, 1), Vector4f(1, 1, 1, 1),
      Vector4f(1, 1, 1, 1), Vector4f(1, 1, 1, 1),
  };

  uv = {
      Vector2f(0, 0),     Vector2f(0, 0.5),
      Vector2f(0.5, 0.5), Vector2f(0.5, 0),

      Vector2f(0.5, 0),   Vector2f(0.5, 0.5),
      Vector2f(1.0, 0.5), Vector2f(1.0, 0),

      Vector2f(0, 0.5),   Vector2f(0, 1.0),
      Vector2f(0.5, 1.0), Vector2f(0.5, 0.5),

      Vector2f(0.5, 0.5), Vector2f(0.5, 1.0),
      Vector2f(1.0, 1.0), Vector2f(1.0, 0.5),
  };

  LoadGLBuffers();

  browserImg = std::make_shared<Texture>();
  browserImg->Init(userData->WindowWidth, userData->WindowHeight, GL_RGBA8,
                   GL_RGBA);

  InitCef();
}

void Cef::InitCef() {
  CefMainArgs main_args(userData->argc, userData->argv);

  std::string cachePath = std::filesystem::current_path().string() + "/cache";
  std::string animationPath =
      std::filesystem::current_path().string() + "/assets/animation.html";

  std::cout << "cachePath " << cachePath << std::endl;

  CefSettings settings;
  settings.no_sandbox = true;
  CefString(&settings.cache_path).FromASCII(cachePath.c_str());
  // settings.windowless_rendering_enabled = true;

  if (!CefInitialize(main_args, settings, nullptr, nullptr)) {
    return;
  }

  renderHandler = new RenderHandler();
  renderHandler->init(userData->WindowWidth, userData->WindowHeight);

  CefWindowInfo windowInfo;
  windowInfo.SetAsWindowless(kNullWindowHandle);

  CefBrowserSettings browserSettings;

  browser = CefBrowserHost::CreateBrowserSync(
      windowInfo, renderHandler, "file:///" + animationPath, browserSettings,
      nullptr, nullptr);
}

void Cef::Render(Matrix4x4 view) {
  if (renderHandler->data.mtx.try_lock()) {
    if (renderHandler->data.is_new) {
      browserImg->Bind(GL_TEXTURE0);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, userData->WindowWidth,
                      userData->WindowHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                      renderHandler->data.y);

      renderHandler->data.is_new = false;
    }

    renderHandler->data.mtx.unlock();
  }
  instance::Render(view);
}

Cef::~Cef() { CefShutdown(); }