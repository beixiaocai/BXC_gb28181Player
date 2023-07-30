//
// Created by bxc on 2023/4/18.
// 作者：北小菜
// 邮箱：bilibili_bxc@126.com
// 西瓜视频主页：https://www.ixigua.com/home/4171970536803763
// 哔哩哔哩主页：https://space.bilibili.com/487906612/
//

#include "gb28181Player.h"
#include "Utils.h"
#include <string.h>

int avio_read_packet(void* opaque, uint8_t* buf, int buffsize){
	GB28181Player* player = (GB28181Player*)opaque;

	int ret = 0;
	if (player->bufferSize >= buffsize)
	{
		memcpy(buf, player->buffer, buffsize);
		player->bufferSize = player->bufferSize - buffsize;
		memmove(player->buffer, player->buffer + buffsize, player->bufferSize);
		ret = buffsize;

		LOGI("avio_read_packet=%d", buffsize);
	}
	return ret;
}


GB28181Player::GB28181Player()
{
}

GB28181Player::~GB28181Player()
{
	if (mVideoCodecPar) {
		avcodec_parameters_free(&mVideoCodecPar);
	}
	if (mVideoCodecCtx) {
		avcodec_close(mVideoCodecCtx);
		mVideoCodecCtx = nullptr;
	}

	if (mFmtCtx) {
		avformat_close_input(&mFmtCtx);
		mFmtCtx = nullptr;
	}


}

bool GB28181Player::probe()
{


	mFmtCtx = avformat_alloc_context();

	unsigned char* avioBuff = (unsigned char*)av_malloc(1920 * 1080);
	mAvioCtx = avio_alloc_context(avioBuff, sizeof(avioBuff), 0, this, avio_read_packet, NULL, NULL);
	//探测流（获取码流格式）
	if (av_probe_input_buffer2(mAvioCtx, (const AVInputFormat**)&mInputFmt, "", NULL, 0, 0) < 0){
		LOGE("av_probe_input_buffer2 error");
		return false;
	}
	mFmtCtx->pb = mAvioCtx;

	//配置流参数
	//av_dict_set(&options, "fflags", "nobuffer", 0); //不缓存直接解码

	//打开流
	if (avformat_open_input(&mFmtCtx, "", mInputFmt, &net_options) != 0)
	{
		LOGE("avformat_open_input error");
		return false;
	}
	//获取流信息
	if (avformat_find_stream_info(mFmtCtx, NULL) < 0)//?
	{
		LOGE("avformat_find_stream_info error");
		return false;
	}
	//获取视频流
	mVideoStream = av_find_best_stream(mFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (mVideoStream < 0)
	{
		LOGE("av_find_best_stream error");
		return false;
	}
	//获取解码信息
	mVideoCodecPar = mFmtCtx->streams[mVideoStream]->codecpar;
	const AVCodec* videoCodec = avcodec_find_decoder(mVideoCodecPar->codec_id);
	if (!videoCodec){
		LOGE("avcodec_find_decoder error");
		return false;
	}
	mVideoCodecCtx = avcodec_alloc_context3(videoCodec);

	//codecpar为解码器上下文赋值
	if (avcodec_parameters_to_context(mVideoCodecCtx, mVideoCodecPar) != 0)
	{
		LOGE("avcodec_parameters_to_context error");
		return false;
	}

	//设置解码器参数
	//av_dict_set(&codec_options, "tune", "zero-latency", 0);//设置零延迟
	//av_dict_set(&codec_options, "preset", "ultrafast", 0);//设置最模糊但是最快的解码方式
	//av_dict_set(&codec_options, "x265-params", "qp=20", 0);//设置265量化参数
	//量化参数：控制了视频帧中每一个宏区块（Macroblock）的压缩量。较大的数值，量化值更高，意味着更多的压缩，更低的质量，较小的数值代表相反的含义。

	//打开解码器
	if (avcodec_open2(mVideoCodecCtx, videoCodec, &codec_options) < 0)
	{
		LOGE("avcodec_open2 error");
		return false;
	}
	LOGI("mVideoCodecCtx->width=%d,mVideoCodecCtx->height=%d", mVideoCodecCtx->width, mVideoCodecCtx->height);
	return true;
}

void GB28181Player::play(){
	LOGI("start");
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		LOGE("SDL_Init error");
		return;
	}
	SDL_Rect sdl_rect;
	SDL_Window* sdl_window;
	SDL_Renderer* sdl_renderer;
	SDL_Texture* sdl_texture;
	int width = mVideoCodecCtx->width / 2;
	int height = mVideoCodecCtx->height / 2;

	std::string name = "BXC_gb28181Player_" + std::to_string(width) + "*" + std::to_string(height);
	//创建SDL窗口
	if (!(sdl_window = SDL_CreateWindow(name.data(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE)))
	{
		LOGE("SDL_CreateWindow error");
		return;
	}
	//创建渲染器
	if (!(sdl_renderer = SDL_CreateRenderer(sdl_window, -1, 0)))
	{
		LOGE("SDL_CreateRenderer error");
		return;
	}
	//创建纹理
	if (!(sdl_texture = SDL_CreateTexture(sdl_renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,
		mVideoCodecCtx->width, mVideoCodecCtx->height)))
	{
		LOGE("SDL_CreateTexture error");
		return;
	}

	int ret;


	AVFrame* frame = av_frame_alloc();
	AVPacket pkt;

	while (av_read_frame(mFmtCtx, &pkt) >= 0) {
		if (pkt.stream_index == mVideoStream){
			ret = avcodec_send_packet(mVideoCodecCtx, &pkt);
			LOGI("avcodec_send_packet=%d", ret);

			while (true){
				ret = avcodec_receive_frame(mVideoCodecCtx, frame);
				LOGI("avcodec_receive_frame=%d", ret);
				if (0 == ret) {
					SDL_UpdateYUVTexture(sdl_texture, NULL,
						frame->data[0], frame->linesize[0],
						frame->data[1], frame->linesize[1],
						frame->data[2], frame->linesize[2]);
					// 设置大小
					sdl_rect.x = 0;
					sdl_rect.y = 0;
					sdl_rect.w = width;
					sdl_rect.h = height;
					SDL_RenderClear(sdl_renderer);
					SDL_RenderCopy(sdl_renderer, sdl_texture, NULL, &sdl_rect);
					SDL_RenderPresent(sdl_renderer);
				}
				else {
					break;
				}
			}
		}
		av_packet_unref(&pkt);
		SDL_Event event;
		SDL_PollEvent(&event);
	}

	if (frame) {
		av_frame_free(&frame);
	}

	if (sdl_window) {
		SDL_DestroyWindow(sdl_window);
	}

	if (sdl_renderer) {
		SDL_DestroyRenderer(sdl_renderer);
	}

	if (sdl_texture) {
		SDL_DestroyTexture(sdl_texture);
	}

	SDL_Quit();
	LOGI("end");
}