/*
 * user_solenoid.h	LED驱动头文件
*/

#ifndef __USER_LED_H__
#define __USER_LED_H__

#include "stm32l0xx_hal.h"

/*
 * InitLed:				初始化LED
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t InitLed(void);

/*
 * LedPowerOn:				打开LED供电电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t LedPowerOn(void);

/*
 * LedPowerOff:			打开LED供电电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t LedPowerOff(void);

/*
 * OpenLed:				点亮Led灯
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t OpenLed(void);

/*
 * CloseLed:			熄灭Led灯
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t CloseLed(void);

/*
 * FlashLed:			闪烁Led灯
 * rate:				闪烁频率Hz 有效值为1~1000
 * count:				闪烁次数,0表示一直闪烁
 * 返回值:				1成功 0失败
*/
uint8_t FlashLed(uint32_t rate , uint32_t count);

#endif /* __USER_LED_H__ */
