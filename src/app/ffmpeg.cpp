#include "ffmpeg.hpp"

FFmpeg::FFmpeg() {
  if (avformat_open_input(&track.fmt_ctx, "assets/sample2.mp4", nullptr,
                          nullptr) < 0) {
    fprintf(stderr, "Could not open %s\n", "assets/sample2.mp4");
    exit(1);
  }

  if (avformat_find_stream_info(track.fmt_ctx, nullptr) < 0) {
    fprintf(stderr, "Could not find stream information\n");
    exit(1);
  }

  av_dump_format(track.fmt_ctx, 0, "assets/sample2.mp4", 0);

  auto stream_idx =
      av_find_best_stream(track.fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
  if (stream_idx < 0) {
    std::cout << "no video stream found" << std::endl;
    exit(1);
  }

  track.stream = track.fmt_ctx->streams[stream_idx];
  track.codec = avcodec_find_decoder(track.stream->codecpar->codec_id);
  if (!track.codec) {
    std::cout << "codec not found" << std::endl;
    exit(1);
  }

  track.c = avcodec_alloc_context3(track.codec);
  if (!track.c) {
    std::cout << "contextnot found" << std::endl;
    exit(1);
  }

  if (avcodec_parameters_to_context(track.c, track.stream->codecpar) < 0) {
    std::cout << "failed to copy codec parameters to context" << std::endl;
    exit(1);
  }

  if (avcodec_open2(track.c, track.codec, NULL) < 0) {
    std::cout << "failed to open codec" << std::endl;
    exit(1);
  }

  track.frame = av_frame_alloc();
  track.pkt = av_packet_alloc();

  lx = 64;
  ly = 48;

  points = {
      Vector2f(0.0, 0.0),           Vector2f(0.0, ly / 4.0),
      Vector2f(lx / 2.0, ly / 4.0), Vector2f(lx / 2.0, 0.0),

      Vector2f(lx / 2.0, 0.0),      Vector2f(lx / 2.0, ly / 4.0),
      Vector2f(lx, ly / 4.0),       Vector2f(lx, 0.0),

      Vector2f(0.0, ly / 4.0),      Vector2f(0.0, ly / 2.0),
      Vector2f(lx / 2.0, ly / 2.0), Vector2f(lx / 2.0, ly / 4.0),

      Vector2f(lx / 2.0, ly / 4.0), Vector2f(lx / 2.0, ly / 2.0),
      Vector2f(lx, ly / 2.0),       Vector2f(lx, ly / 4.0),
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

  y_tex = std::make_shared<Texture>();
  u_tex = std::make_shared<Texture>();
  v_tex = std::make_shared<Texture>();
}

void FFmpeg::FrameLoop() {
  int ret;

  auto frame_duration = std::chrono::milliseconds(1000 / 25);

  while (1) {
    ret = av_read_frame(track.fmt_ctx, track.pkt);

    if (ret == AVERROR_EOF) {
      avformat_seek_file(track.fmt_ctx, track.stream->index, 0, 0,
                         track.fmt_ctx->streams[track.stream->index]->duration,
                         0);

      avcodec_flush_buffers(track.c);

      ret = av_read_frame(track.fmt_ctx, track.pkt);
    } else if (ret < 0) {
      break;
    }

    auto start_time = std::chrono::steady_clock::now();
    writeToTex();
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed = end_time - start_time;

    if (elapsed < frame_duration) {
      std::this_thread::sleep_for(frame_duration - elapsed);
    }

    av_packet_unref(track.pkt);
  }

  std::cout << "finished" << std::endl;
}

void FFmpeg::writeToTex() {
  auto ret = avcodec_send_packet(track.c, track.pkt);
  if (ret < 0) {

    char errbuf[256];
    av_strerror(ret, errbuf, sizeof(errbuf));
    std::cout << "error occured: " << ret << " " << errbuf << std::endl;

    exit(1);
  }

  while (ret >= 0) {
    ret = avcodec_receive_frame(track.c, track.frame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
      return;
    }

    {
      std::lock_guard<std::mutex> lock(data.mtx);

      data.y = track.frame->data[0];
      data.u = track.frame->data[1];
      data.v = track.frame->data[2];

      data.linewidth_y = track.frame->linesize[0];
      data.linewidth_u = track.frame->linesize[1];
      data.linewidth_v = track.frame->linesize[2];

      data.height_y = track.frame->height;
      data.height_u = track.frame->height / 2;
      data.height_v = track.frame->height / 2;

      data.is_new = true;
    }
  }
}

void FFmpeg::Render(Matrix4x4 View) {
  if (data.mtx.try_lock()) {
    if (data.is_new) {
      std::call_once(data.texInitFlag, [this]() {
        this->y_tex->Init(this->data.linewidth_y, this->data.height_y, GL_RED,
                          GL_RED);
        this->u_tex->Init(this->data.linewidth_u, this->data.height_u, GL_RED,
                          GL_RED);
        this->v_tex->Init(this->data.linewidth_v, this->data.height_v, GL_RED,
                          GL_RED);
      });

      data.is_new = false;

      y_tex->Bind(GL_TEXTURE1);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.linewidth_y, data.height_y,
                      GL_RED, GL_UNSIGNED_BYTE, data.y);

      u_tex->Bind(GL_TEXTURE2);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.linewidth_u, data.height_u,
                      GL_RED, GL_UNSIGNED_BYTE, data.u);

      v_tex->Bind(GL_TEXTURE3);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data.linewidth_v, data.height_v,
                      GL_RED, GL_UNSIGNED_BYTE, data.v);
    }

    data.mtx.unlock();
  }

  instance::Render(View);
}