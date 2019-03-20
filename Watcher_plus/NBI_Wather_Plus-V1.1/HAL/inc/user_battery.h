
#ifndef __USER_BAT_H__
#define __USER_BAT_H__

#include "stm32l0xx_hal.h"

/* 包含头文件 ----------------------------------------------------------------*/

/* 类型定义 ------------------------------------------------------------------*/


/* 宏定义 --------------------------------------------------------------------*/


/* 变量声明 ------------------------------------------------------------------*/


/* 函数声明 ------------------------------------------------------------------*/
/*
 * Bq24195Init:			初始化供电电源芯片
 * 参数:				无
 * 返回值:				无
*/
void  Bq24195Init(void);


void BatEnableCharge(void);

void BatDisableCharge(void);

uint8_t BatCheck(uint8_t *bat);

uint8_t BatGetBattery(float *Battery_V);

float BatGetChargePower(float *ChargePower);

#endif /* __USER_BAT_H__ */
