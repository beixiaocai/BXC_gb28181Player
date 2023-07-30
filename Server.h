//
// Created by bxc on 2023/4/18.
// 作者：北小菜
// 邮箱：bilibili_bxc@126.com
// 西瓜视频主页：https://www.ixigua.com/home/4171970536803763
// 哔哩哔哩主页：https://space.bilibili.com/487906612/
//

#ifndef GB28181PLAYER_SERVER_H
#define GB28181PLAYER_SERVER_H

#include <stdint.h>

#define Server_cache_max_size 4194304 // 1M = 1 * 1024 * 1024 = 1048576 字节
#define Server_rtp_max_size 1800

class GB28181Player;
class Server {
public:
	Server(const char* ip, uint16_t port, bool isUdp,GB28181Player *player);
	~Server();
public:
	int start();
private:
	int startUdpServer(const char* ip, uint16_t port);
	int startTcpServer(const char* ip, uint16_t port);

	void parseTcpData(char* recvBuf, int recvBufSize);
private:
	const char* mIp; 
	uint16_t    mPort;
	bool        mIsUdp;
	GB28181Player* mPlayer;
	uint8_t* mRecvCache = nullptr;
	uint64_t mRecvCacheSize = 0;

	uint8_t* mRecvRtpBuffer;    // 从mRecvCache提取出来的rtp字节流
	int16_t  mRecvRtpBufferSize = 0;// 从mRecvCache提取出来的rtp字节流总长度 （rtpHeader+rtpBody）

};

#endif //GB28181PLAYER_SERVER_H

