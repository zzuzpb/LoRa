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
#include "user-app.h"

#define  FI_GPIO_PORT						 GPIOA
#define  FI_PIN								 GPIO_PIN_12

#define  BI_GPIO_PORT						 GPIOA
#define  BI_PIN        						 GPIO_PIN_11

#define  CTRL_12V_PORT					 	 GPIOA
#define  CTRL_12V_POWERON   				 GPIO_PIN_8


/*
 *SOLENOIDSTATE:	电磁阀状态结构体
*/
typedef enum SOLENOIDSTATE
{
	SLND_CLOSE	=0,		//关
	SLND_OPEN	=1		//开
}SolenoidState;

/*
 *solen_vale_init：电磁阀初始化
 *参数：		 无
 *返回值：		 无
*/
void solen_vale_init(void);

/*
 * Ctrl_12V_PowerInit: 初始化12V电源开关
 * 参数：	 		   无
 * 返回参数：		   无
*/
void Ctrl_12V_PowerInit(void);

/*
 * PowerOn:		打开电磁阀电源开关
 * 参数：		无
 * 返回参数：	无
*/
void Ctrl_12V_PowerOn(void);

/*
 * PowerOn:	  关闭电磁阀电源开关
 * 参数：	  无
 * 返回参数： 无
*/
void Ctrl_12V_PowerOff(void);

/*
 * solen_vale_open:	 打开电磁阀
 * 参数：	 		 无
 * 返回参数：		 无
*/
void solen_vale_open(void);

/*
 * solen_vale_close: 关闭电磁阀
 * 参数：	 		 无
 * 返回参数：		 无
*/
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

/*
 *WaterMCounter：水阀手动关闭次数计算
 *参数：		 无
 *返回值：		 无
*/
void WaterMCounter(void);

/*
 *WaterMOCounter：水阀手动打开次数计算
 *参数：		  无
 *返回值：		  无
*/
void WaterMOCounter(void);

/*
 *OnLoRaHeartDeal：LoRa心跳处理函数
 *参数：		   无
 *返回值：		   无
*/
void OnLoRaHeartDeal(void);

/*
 *OnLoRaAckDeal：LoRa ACK应答处理
 *参数：		 无
 *返回值：		 无
*/
void OnLoRaAckDeal(void);

/*
 *OnWaterAutopen：水阀自动控制开操作
 *参数：		  无
 *返回值：		  无
*/
void OnWaterAutopen(void);

/*
 *OnWaterAutClose：水阀自动模式关闭操作
 *参数：		   无
 *返回值：		   无
*/
void OnWaterAutClose(void);

/*
 *WaterManOpen：水阀手动模式打开操作
 *参数：		无
 *返回值：		无
*/
void WaterManOpen(void);

/*
 *WaterManClose：水阀手动模关闭操作
 *参数：		 无
 *返回值：		 无
*/
void WaterManClose(void);


#endif
