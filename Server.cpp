//
// Created by bxc on 2023/4/18.
// 作者：北小菜
// 邮箱：bilibili_bxc@126.com
// 西瓜视频主页：https://www.ixigua.com/home/4171970536803763
// 哔哩哔哩主页：https://space.bilibili.com/487906612/
//

#include "Server.h"
#include <stdint.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include "Rtp.h"
#include "Utils.h"
#include "gb28181Player.h"

#pragma comment(lib, "ws2_32.lib")


Server::Server(const char* ip, uint16_t port, bool isUdp, GB28181Player* player):
	mIp(ip),mPort(port), mIsUdp(isUdp),
	mPlayer(player) {
	mRecvCache = (uint8_t*)malloc(Server_cache_max_size);
	mRecvRtpBuffer = (uint8_t*)malloc(Server_rtp_max_size);

	//char buf[4];
	//int size = 1440;

	//buf[0] = (uint8_t)(((size) & 0xFF00) >> 8);
	//buf[1] = (uint8_t)((size) & 0xFF);
	//buf[2] = (size >> 8) & 0xFF;
	//buf[3] = size & 0xFF;

}
Server::~Server() {

	if (mRecvCache) {
		free(mRecvCache);
		mRecvCache = nullptr;
	}
	if (mRecvRtpBuffer) {
		free(mRecvRtpBuffer);
		mRecvRtpBuffer = nullptr;
	}
}


int Server::start() {

	if (!mIsUdp) {
		LOGI("BXC_gb28181Player tcp://%s:%d", mIp, mPort);
		this->startTcpServer(mIp, mPort);
	}
	else {
		LOGI("BXC_gb28181Player udp://%s:%d", mIp, mPort);
		this->startUdpServer(mIp, mPort);
	}

	return 0;
}
int Server::startUdpServer(const char* ip, uint16_t port) {

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		LOGI("WSAStartup error");
		return -1;
	}

	SOCKET server_fd = -1;
	SOCKADDR_IN server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//server_addr.sin_addr.s_addr = inet_addr("192.168.2.61");
	server_addr.sin_port = htons(port);



	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_fd < 0) {
		LOGI("socket bind error");
		return -1;
	}
	int on = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&on, sizeof(on));

	if (bind(server_fd, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		LOGI("socket bind error");
		return -1;
	}

	char recvBuf[10000];
	int  recvBufSize;

	uint64_t speedTotalSize = 0;
	time_t  t1 = time(NULL);
	time_t  t2 = 0;



	SOCKADDR_IN addr;   //用来接收客户端的地址信息
	int addrLen = sizeof(SOCKADDR);

	while (true)
	{
		//std::string psFile = "udp-video-" + getCurTimeStr("%Y%m%d-%H%M%S") + ".ps";
		//FILE* psf = fopen(psFile.data(), "wb");

		while (true)
		{
			recvBufSize = recvfrom(server_fd, recvBuf, sizeof(recvBuf), 0, (SOCKADDR*)&addr, &addrLen);

			if (recvBufSize == SOCKET_ERROR) {
				LOGE("recvfrom error:%d", WSAGetLastError());
				break;
			}

			if (recvBufSize <= 0) {
				LOGE("recvfrom error:recvBufSize=%d", recvBufSize);
				break;
			}
			//for (int i= 0; i < 20; ++i) {
			//	uint8_t c = recvBuf[i];
			//	printf("%d=%d,", i, c);
			//}
			//printf("\n");


			// 计算码流速率 start
			speedTotalSize += recvBufSize;

			if (speedTotalSize > 6291456)/* 62914560=60*1024*1024=60mb*/
			{
				t2 = time(NULL);
				if (t2 - t1 > 0) {
					uint64_t speed = speedTotalSize / 1024 / 1024 / (t2 - t1);
					LOGI("client=%s:%d,recvBufSize=%d,speedTotalSize=%llu,speed=%lluMbps",
						inet_ntoa(addr.sin_addr), ntohs(addr.sin_port),
						recvBufSize, speedTotalSize, speed);
					speedTotalSize = 0;
					t1 = time(NULL);
				}
			}
			// 计算码流速率 end
			
			//fwrite(recvBuf, 1, recvBufSize, psf);
			if ((mPlayer->bufferSize + recvBufSize - RTP_HEADER_SIZE) < GB28181Player_buffer_max_size) {
				memcpy(mPlayer->buffer + mPlayer->bufferSize, recvBuf + RTP_HEADER_SIZE, recvBufSize - RTP_HEADER_SIZE);
				mPlayer->bufferSize += recvBufSize - RTP_HEADER_SIZE;
			}
			else {
				LOGE("recvBufSize = %d over GB28181Player_buffer_max_size ", recvBufSize);
			}
		}
		
		//fclose(psf);

	}

	closesocket(server_fd);

	return 0;
}
int Server::startTcpServer(const char* ip, uint16_t port) {

	SOCKET server_fd = -1;
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		LOGI("WSAStartup error");
		return -1;
	}
	SOCKADDR_IN server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//server_addr.sin_addr.s_addr = inet_addr("192.168.2.61");
	server_addr.sin_port = htons(port);

	server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (bind(server_fd, (SOCKADDR*)&server_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		LOGI("socket bind error");
		return -1;
	}

	if (listen(server_fd, SOMAXCONN) < 0) {
		LOGI("socket listen error");
		return -1;
	}

	while (true)
	{
		LOGI("阻塞监听新连接...");
		// 阻塞接收请求 start
		int len = sizeof(SOCKADDR);
		SOCKADDR_IN accept_addr;
		int clientFd = accept(server_fd, (SOCKADDR*)&accept_addr, &len);
		//const char* clientIp = inet_ntoa(accept_addr.sin_addr);

		if (clientFd == SOCKET_ERROR) {
			LOGI("accept connection error");
			break;
		}
		// 阻塞接收请求 end
		LOGI("发现新连接：clientFd=%d", clientFd);

		{
			char recvBuf[10000];
			int  recvBufSize;

			uint64_t speedTotalSize = 0;
			time_t  t1 = time(NULL);
			time_t  t2 = 0;


			while (true) {
				recvBufSize = recv(clientFd, recvBuf, sizeof(recvBuf), 0);
				if (recvBufSize <= 0) {
					LOGE("::recv error: clientFd=%d,recvBufSize=%d", clientFd, recvBufSize);
					break;
				}

				speedTotalSize += recvBufSize;
				if (speedTotalSize > 2097152) /* 2097152=2*1024*1024=2mb*/
				{
					t2 = time(NULL);
					if (t2 - t1 > 0) {
						uint64_t speed = speedTotalSize / 1024 / (t2 - t1);
						printf("clientFd=%d,speedTotalSize=%llu,speed=%llu Kbps\n", clientFd, speedTotalSize, speed);
						speedTotalSize = 0;
						t1 = time(NULL);
					}
				}
				parseTcpData(recvBuf, recvBufSize);
			}
		}
		closesocket(clientFd);
		LOGI("关闭连接 clientFd=%d", clientFd);

	}
	return 0;

}

void Server::parseTcpData(char* recvBuf, int recvBufSize) {

	if ((mRecvCacheSize + recvBufSize) > Server_cache_max_size) {
		LOGE("超过缓冲容量上限，忽略本次读取的数据。mRecvCacheSize=%d,recvBufSize=%d",mRecvCacheSize, recvBufSize);
	
	}
	else {
		memcpy(mRecvCache + mRecvCacheSize, recvBuf, recvBufSize);
		mRecvCacheSize += recvBufSize;
	}
	//LOGI("cacheSize=%d，开始进入解析 ... ...", cacheSize);

	while (true) {

		if (mRecvCacheSize > 2) {

			bool success = false;

			if (mRecvCacheSize > 2) {
				mRecvRtpBufferSize = ntohs(*(int16_t*)(mRecvCache));
				if ((mRecvCacheSize - 2) >= mRecvRtpBufferSize) {
					success = true;
				}
			}

			if (success) {
				mRecvCacheSize -= 2;
				mRecvCacheSize -= mRecvRtpBufferSize;

				// 提取RTP
				memcpy(mRecvRtpBuffer, mRecvCache + 2, mRecvRtpBufferSize);
				memmove(mRecvCache, mRecvCache + 2 + mRecvRtpBufferSize, mRecvCacheSize);

				// RTP
				struct RtpHeader rtpHeader;
				parseRtpHeader(mRecvRtpBuffer, &rtpHeader);
				printf("get a rtp seq=%d,RtpBufferSize=%d,mRecvCacheSize=%d,marker=%d,timestamp=%d\n", 
					rtpHeader.seq, 
					mRecvRtpBufferSize, 
					mRecvCacheSize,rtpHeader.marker, rtpHeader.timestamp);


				//for (int i = 0; i < pktSize; i++) {
				//	printf("%d-%d\n", i, pktBuf[i]);
				//}
				//printf("x\n");

				// 将从mRecvCache提取出来的rtp字节流 mRecvRtpBuffer去掉RTP_HEADER_SIZE，存储到播放器缓存中
				if ((mPlayer->bufferSize + mRecvRtpBufferSize - RTP_HEADER_SIZE) < GB28181Player_buffer_max_size) {
					memcpy(mPlayer->buffer + mPlayer->bufferSize, mRecvRtpBuffer + RTP_HEADER_SIZE, mRecvRtpBufferSize - RTP_HEADER_SIZE);
					mPlayer->bufferSize += mRecvRtpBufferSize - RTP_HEADER_SIZE;
				}
				else {
					LOGE("recvBufSize = %d over GB28181Player_buffer_max_size ", recvBufSize);
				}

			}
			else {
				//LOGI("跳出解析:cacheSize=%d,pktSize=%d", cacheSize, pktSize);
				break;
			}
		}
		else {
			//LOGI("跳出解析:缓冲数据未发现完整数据包");
			break;
		}
	}



}
