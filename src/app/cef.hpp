#include <filesystem>
#include <memory>
#include <thread>

#include "../base/base.hpp"
#include "../base/instance.hpp"
#include "../base/texture.hpp"

#include <include/cef_app.h>
#include <include/cef_client.h>
#include <include/cef_render_handler.h>

class RenderHandler : public CefRenderHandler, public CefClient {
public:
  shared_data data;

  void init(int width, int height) {
    data.linewidth_y = width;
    data.height_y = height;

    data.y = new uint8_t[width * height * 4];
  }

  virtual CefRefPtr<CefRenderHandler> GetRenderHandler() override {
    return this;
  }

  void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect) override {
    rect = CefRect(0, 0, data.linewidth_y, data.height_y);
  }
  void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
               const RectList &dirtyRects, const void *buffer, int width,
               int height) override;

  IMPLEMENT_REFCOUNTING(RenderHandler);
};

class Cef : public instance {
  RenderHandler *renderHandler;
  CefRefPtr<CefBrowser> browser;

  std::shared_ptr<Texture> browserImg;

  float lx = 20.0;
  float ly = 10.0;

public:
  Cef();
  ~Cef();

  void InitCef();

  void Render(Matrix4x4 View);
};