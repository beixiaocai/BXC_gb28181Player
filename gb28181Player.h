//
// Created by bxc on 2023/4/18.
// 作者：北小菜
// 邮箱：bilibili_bxc@126.com
// 西瓜视频主页：https://www.ixigua.com/home/4171970536803763
// 哔哩哔哩主页：https://space.bilibili.com/487906612/
//

#ifndef GB28181PLAYER_GB28181PLAYER_H
#define GB28181PLAYER_GB28181PLAYER_H

#include <windows.h>
#include <atomic>
#define  SDL_MAIN_HANDLED
#include <SDL.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
	#include <libswscale/swscale.h>
}
#define GB28181Player_buffer_max_size 4194304 // 4M = 4 * 1024 * 1024 = 4194304 字节

class GB28181Player
{
public:
	GB28181Player();
	~GB28181Player();
public:
	bool probe();//阻塞式探测国标流并获取解码参数
	void play();//在探测国标流成功以后，解码并渲染国标视频流
public:
	std::atomic<char> buffer[GB28181Player_buffer_max_size];
	std::atomic_int   bufferSize = 0;
private:
	AVFormatContext    * mFmtCtx;
	AVIOContext        * mAvioCtx;
	const AVInputFormat* mInputFmt;
	int                  mVideoStream = -1;
	AVCodecParameters  * mVideoCodecPar;
	AVCodecContext     * mVideoCodecCtx;

	AVDictionary* net_options;//网络连接参数
	AVDictionary* codec_options;//编码参数

};
#endif //GB28181PLAYER_GB28181PLAYER_H