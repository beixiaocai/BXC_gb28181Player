//
// Created by bxc on 2023/4/18.
// 作者：北小菜
// 邮箱：bilibili_bxc@126.com
// 西瓜视频主页：https://www.ixigua.com/home/4171970536803763
// 哔哩哔哩主页：https://space.bilibili.com/487906612/
//

#include "Rtp.h"
#include <stdio.h>
#include <string.h>

void rtpHeaderInit(struct RtpPacket* rtpPacket, uint8_t csrcLen, uint8_t extension,
                   uint8_t padding, uint8_t version, uint8_t payloadType, uint8_t marker,
                   uint16_t seq, uint32_t timestamp, uint32_t ssrc){
    rtpPacket->rtpHeader.csrcLen = csrcLen;
    rtpPacket->rtpHeader.extension = extension;
    rtpPacket->rtpHeader.padding = padding;
    rtpPacket->rtpHeader.version = version;
    rtpPacket->rtpHeader.payloadType =  payloadType;
    rtpPacket->rtpHeader.marker = marker;
    rtpPacket->rtpHeader.seq = seq;
    rtpPacket->rtpHeader.timestamp = timestamp;
    rtpPacket->rtpHeader.ssrc = ssrc;
}
int parseRtpHeader(uint8_t* headerBuf, struct RtpHeader* rtpHeader){
    memset(rtpHeader,0,sizeof(*rtpHeader));
    /* byte 0 */
    rtpHeader->version = (headerBuf[0] & 0xC0) >> 6;
    rtpHeader->padding = (headerBuf[0] & 0x20) >> 5;
    rtpHeader->extension = (headerBuf[0] & 0x10) >> 4;
    rtpHeader->csrcLen = (headerBuf[0] & 0x0F);
    /* byte 1 */
    rtpHeader->marker = (headerBuf[1] & 0x80) >> 7;
    rtpHeader->payloadType = (headerBuf[1] & 0x7F);
    /* bytes 2,3 */
    rtpHeader->seq = ((headerBuf[2] & 0xFF) << 8) | (headerBuf[3] & 0xFF);
    /* bytes 4-7 */
    rtpHeader->timestamp = ((headerBuf[4] & 0xFF) << 24) | ((headerBuf[5] & 0xFF) << 16)
            | ((headerBuf[6] & 0xFF) << 8)
            | ((headerBuf[7] & 0xFF));
    /* bytes 8-11 */
    rtpHeader->ssrc = ((headerBuf[8] & 0xFF) << 24) | ((headerBuf[9] & 0xFF) << 16)
                           | ((headerBuf[10] & 0xFF) << 8)
                           | ((headerBuf[11] & 0xFF));

    return 0;
}

