/*
**************************************************************************************************************
*	@file	  solen-vale.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief  电磁阀
***************************************************************************************************************
*/

#ifndef __SOLEN_VALE__H
#define __SOLEN_VALE__H

#include "stm32l0xx_hal.h"

#define  FI_GPIO_PORT						 GPIOA
#define  BI_GPIO_PORT						 GPIOB

#define  SOLEN_POWER0N     			 GPIO_PIN_3
#define  FI_PIN								   GPIO_PIN_8
#define  BI_PIN        					 GPIO_PIN_15

/*
 *SOLENOIDSTATE:	电磁阀状态结构体
*/
typedef enum SOLENOIDSTATE
{
	SLND_CLOSE	=0,		//关
	SLND_OPEN	=1		//开
}SolenoidState;


void solen_vale_init(void);

void SolenoidPowerOn(void);

void SolenoidPowerOff(void);

void solen_vale_open(void);

void solen_vale_close(void);

/*
 * GetSolenoidState:	获取当前电磁阀开关状态
 * 参数：				 			无
 * 返回值：				    当前电磁阀开关状态
*/
SolenoidState GetSolenoidState(void);

/*
 * SetSolenoidState:	设置当前电磁阀开关状态
 * 参数：				 			无
 * 返回值：				    当前电磁阀开关状态
*/
SolenoidState SetSolenoidState(SolenoidState state);


#endif
