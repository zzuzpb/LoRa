/*
 * user_solenoid.h	LED驱动头文件
*/

#ifndef __USER_LED_H__
#define __USER_LED_H__

#include "user_config.h"

typedef enum 
{
    E_LED_INIT          =0,  // 启动并进入初始化,读取传感器数据
    E_LED_CONNECT       ,    // 连接上网络
    E_LED_DISCONNECT    ,    // 正在连接
    E_LED_LOCATION      ,    // GPS定位
    E_LED_ERROR         ,    // 其它异常（SIM模块损坏、SIM卡槽损坏）
    E_LED_SENDOK        ,    // 发送完成
}E_LedState;

void InitLed(void);

void LedOpen(void);

void LedClose(void);

void LedChangeState(E_LedState State);

void LedDisplay(uint16_t BaseTime);

#endif /* __USER_LED_H__ */
