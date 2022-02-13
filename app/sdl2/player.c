#include "stdio.h"
#include <SDL.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int ffplayer() {
  AVFormatContext *pFormatCtx;
  int i, videoindex;
  AVCodecContext *pCodecCtx;
  AVCodec *pCodec;
  char filepath[]="/test.wmv";
//   char rtspUrl[] = "rtsp://admin:12345@192.168.10.76:554";
  av_register_all();                      //注册组件
  avformat_network_init();                //支持网络流
  pFormatCtx = avformat_alloc_context();  //初始化AVFormatContext
  if (avformat_open_input(&pFormatCtx, filepath /*rtspUrl*/, NULL, NULL) !=
      0) {  //打开文件或网络流
    printf("无法打开文件\n");
    return -1;
  }
  if (avformat_find_stream_info(pFormatCtx, NULL) < 0)  //查找流信息
  {
    printf("Couldn't find stream information.\n");
    return -1;
  }
  videoindex = -1;
  for (i = 0; i < pFormatCtx->nb_streams; i++)  //获取视频流ID
    if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
      videoindex = i;
      break;
    }
  if (videoindex == -1) {
    printf("Didn't find a video stream.\n");
    return -1;
  }
  pCodecCtx = pFormatCtx->streams[videoindex]->codec;
  pCodec = avcodec_find_decoder(pCodecCtx->codec_id);  //查找解码器
  if (pCodec == NULL) {
    printf("Codec not found.\n");
    return -1;
  }
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)  //打开解码器
  {
    printf("Could not open codec.\n");
    return -1;
  }
  AVFrame *pFrame, *pFrameYUV;
  pFrame = avcodec_alloc_frame();     //存储解码后AVFrame
  pFrameYUV = avcodec_alloc_frame();  //存储转换后AVFrame
  uint8_t *out_buffer;
  out_buffer =
      malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width,
                                pCodecCtx->height));  //分配AVFrame所需内存
  avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P,
                 pCodecCtx->width, pCodecCtx->height);  //填充AVFrame

  //------------SDL初始化--------
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
    printf("Could not initialize SDL - %s\n", SDL_GetError());
    return -1;
  }
  SDL_Window *screen = SDL_CreateWindow(
      "RTSP Client Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      pCodecCtx->width, pCodecCtx->height,
      SDL_WINDOW_RESIZABLE /* SDL_WINDOW_HIDDEN*/ | SDL_WINDOW_OPENGL);
  if (!screen) {
    printf("SDL: could not set video mode - exiting\n");
    return -1;
  }
  SDL_Renderer *sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
  SDL_Texture *sdlTexture = SDL_CreateTexture(
      sdlRenderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
      pCodecCtx->width, pCodecCtx->height);

  SDL_Rect rect;
  //-----------------------------
  int ret, got_picture;
  static struct SwsContext *img_convert_ctx;
  int y_size = pCodecCtx->width * pCodecCtx->height;

  SDL_Event event;
  AVPacket *packet =
      (AVPacket *)malloc(sizeof(AVPacket));  //存储解码前数据包AVPacket
  av_new_packet(packet, y_size);
  //输出一下信息-----------------------------
  printf("文件信息-----------------------------------------\n");
  // av_dump_format(pFormatCtx,0,filepath,0);
  printf("-------------------------------------------------\n");
  //------------------------------
  while (av_read_frame(pFormatCtx, packet) >= 0)  //循环获取压缩数据包AVPacket
  {
    if (packet->stream_index == videoindex) {
      ret =
          avcodec_decode_video2(pCodecCtx, pFrame, &got_picture,
                                packet);  //解码。输入为AVPacket，输出为AVFrame
      if (ret < 0) {
        printf("解码错误\n");
        return -1;
      }
      if (got_picture) {
        //像素格式转换。pFrame转换为pFrameYUV。
        img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,
                                         pCodecCtx->pix_fmt, pCodecCtx->width,
                                         pCodecCtx->height, AV_PIX_FMT_YUV420P,
                                         SWS_BICUBIC, NULL, NULL, NULL);
        sws_scale(img_convert_ctx, (const uint8_t *const *)pFrame->data,
                  pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data,
                  pFrameYUV->linesize);
        sws_freeContext(img_convert_ctx);
        //------------SDL显示--------
        rect.x = 0;
        rect.y = 0;
        rect.w = pCodecCtx->width;
        rect.h = pCodecCtx->height;

        SDL_UpdateTexture(sdlTexture, &rect, pFrameYUV->data[0],
                          pFrameYUV->linesize[0]);
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, sdlTexture, &rect, &rect);
        SDL_RenderPresent(sdlRenderer);
        //延时20ms
        SDL_Delay(20);
        //------------SDL-----------
      }
    }
    av_free_packet(packet);
    SDL_PollEvent(&event);
    switch (event.type) {
      case SDL_QUIT:
        SDL_Quit();
        exit(0);
        break;
      default:
        break;
    }
  }

  SDL_DestroyTexture(sdlTexture);
  free(out_buffer);
  av_free(pFrameYUV);
  avcodec_close(pCodecCtx);
  avformat_close_input(&pFormatCtx);

  return 0;
}

int main(int argc, char *argv[]) {
  ffplayer();

  return 0;
}