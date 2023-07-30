# BXC_gb28181Player

* 作者：北小菜 
* 个人网站：http://www.any12345.com
* 邮箱：bilibili_bxc@126.com
* QQ：1402990689
* 微信：bilibili_bxc
* 作者-哔哩哔哩主页：https://space.bilibili.com/487906612
* 作者-头条西瓜主页：https://www.ixigua.com/home/4171970536803763
* github开源地址：https://github.com/any12345com/BXC_gb28181Player
* gitee开源地址：https://gitee.com/Vanishi/BXC_gb28181Player

## 视频教程
*  [哔哩哔哩-从零开发国标GB28181流媒体服务器](https://www.bilibili.com/video/BV1Mv4y1d7Vy)
*  [哔哩哔哩-开发一个支持国标GB28181协议的摄像头模拟软件](https://www.bilibili.com/video/BV1cK411z73C)
*  [哔哩哔哩-开发一个零延迟的摄像头拉流转码推流器](https://www.bilibili.com/video/BV1RN411K75R)
*  [（本项目）哔哩哔哩-基于C++开发的支持国标GB28181协议的视频流播放器](https://www.bilibili.com/video/BV1sL411h7cN)



## 项目介绍
1. 一个基于C++开发的支持国标GB28181协议的视频流播放器，该项目的主要使用场景在安防领域，当然也不限于安防领域，任何需要实时播放ps视频流的场景，都可以使用该项目。尤其是项目中涉及ffmpeg探测流和播放流的代码实现部分，是非常具有参考价值的，我本身写这个项目也花了挺长时间，网上这部分的资料非常少。
2. BXC_gb28181Player作为支持国标GB28181协议的视频流播放器，接收数据部分支持tcp/udp两种方式，解码和渲染部分支持大部分常见的音视频编码格式。
* 注意：如果需要在BXC_gb28181Player基础上，实现信令部分，可以参考我另外的项目 [BXC_SipServer](https://www.bilibili.com/video/BV1Mv4y1d7Vy/?spm_id_from=333.999.0.0&vd_source=f70a17c45cd3d279ce923ca59e12fff0)

### 没有支持国标GB28181协议的摄像头，可以使用ZLMediaKit模拟


###### 使用ZLMediaKit模拟摄像头简要过程
~~~
1，下载编译一份 ZLMediaKit
	ZLMediaKit开源地址：https://gitee.com/xia-chu/ZLMediaKit
	ZLMediaKit直接可用无需编译的程序：https://gitee.com/Vanishi/zlm

2，准备一份视频文件，用于推流到ZLMediaKit，用于模拟摄像头
	// 没有视频文件的可以使用data文件夹下的提供的文件

3，视频文件推流模拟摄像头

	// rtsp推流（文件推流）
	ffmpeg -re -i camera_18383_300s.mp4 -rtsp_transport tcp -c copy -f rtsp rtsp://127.0.0.1:554/live/camera

	// rtsp推流（文件循环推流）
	ffmpeg -re -stream_loop  -1  -i camera_18383_300s.mp4 -rtsp_transport tcp -c copy -f rtsp rtsp://127.0.0.1:554/live/camera

4，将推送到ZLMediaKit的视频流封装为ps流，并通过tcp/udp的方式推送到BXC_gb28181Player用于播放

	// 查看ZLMediaKit的API文档，文档地址：https://github.com/zlmediakit/ZLMediaKit/wiki/MediaServer支持的HTTP-API
	找到 startSendRtp，并调用


附
1，查看ZLMediaKit在线视频流
	http://127.0.0.1/index/api/getMediaList?secret=035c73f7-bb6b-4889-a715-d9eb2d1925cc
2，获取ZLMediaKit在线视频流的播放地址
	
	//rtsp播放地址
	rtsp://127.0.0.1:554/live/camera
	
	//rtmp播放地址
	rtmp://127.0.0.1:1935/live/camera

~~~


###### 附常用ffmpeg命令行

~~~

// 拉流转推流
ffmpeg  -rtsp_transport tcp -i "rtsp://admin:123456@192.168.1.10:554/cam/realmonitor?channel=1&subtype=0&unicast=true"  -codec copy   -f rtsp  -rtsp_transport tcp rtsp://127.0.0.1:554/live/camera

// 拉流转本地录像文件
ffmpeg -rtsp_transport tcp -i "rtsp://admin:123456@192.168.1.10:554/cam/realmonitor?channel=1&subtype=0&unicast=true" -c:v copy -an -t 300 camera_18383_300s.mp4


~~~




