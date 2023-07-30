//
// Created by bxc on 2023/4/18.
// 作者：北小菜
// 邮箱：bilibili_bxc@126.com
// 西瓜视频主页：https://www.ixigua.com/home/4171970536803763
// 哔哩哔哩主页：https://space.bilibili.com/487906612/
//

#ifndef GB28181PLAYER_UTILS_H
#define GB28181PLAYER_UTILS_H

#include <time.h>
#include <chrono>
#include <string>


static int64_t getCurTime()// 获取当前系统启动以来的毫秒数
{
    long long now = std::chrono::steady_clock::now().time_since_epoch().count();
    return now / 1000000;
}
static int64_t getCurMillisecond()// 获取毫秒级时间戳（13位）
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).
        count();

}

/**
 * 获取1970年至今的毫秒数
 * @param system_time 是否为系统时间(系统时间可以回退),否则为程序启动时间(不可回退)
 *
 */
static int64_t getCurrentMillisecond(bool system_time = false) {

    return getCurMillisecond();
}

static std::string getCurTimeStr(const char* time_fmt = "%Y-%m-%d %H:%M:%S") {
    time_t t = time(nullptr);
    char time_str[64];
    strftime(time_str, sizeof(time_str), time_fmt, localtime(&t));

    return time_str;
}

//  __FILE__ 获取源文件的相对路径和名字
//  __LINE__ 获取该行代码在文件中的行号
//  __func__ 或 __FUNCTION__ 获取函数名

#define LOGI(format, ...)  fprintf(stderr,"[INFO]%s [::%d %s()] " format "\n", getCurTimeStr().data(),__LINE__,__func__ ,##__VA_ARGS__)
#define LOGE(format, ...)  fprintf(stderr,"[ERROR]%s [::%d %s()] " format "\n",getCurTimeStr().data(),__LINE__,__func__ ,##__VA_ARGS__)


#endif //GB28181PLAYER_UTILS_H