#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "../base/base.hpp"
#include "../base/instance.hpp"
#include "../base/texture.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libavutil/imgutils.h>
#include <libavutil/mem.h>
}

#define INBUF_SIZE 4096

struct decoder {
  AVFormatContext *fmt_ctx = nullptr;
  AVStream *stream;

  AVCodec *codec;
  AVCodecContext *c;
  AVFrame *frame;
  AVPacket *pkt;
};

class FFmpeg : public instance {
  float lx = 20.0;
  float ly = 10.0;
  decoder track;

public:
  FFmpeg();
  ~FFmpeg() {
    avcodec_close(track.c);
    av_free(track.c);
    av_frame_free(&track.frame);
  };

  void FrameLoop();

  void Render(Matrix4x4 view);

  shared_data data;

  std::shared_ptr<Texture> y_tex;
  std::shared_ptr<Texture> u_tex;
  std::shared_ptr<Texture> v_tex;

private:
  void writeToTex();
};