/*
**************************************************************************************************************
*	@file	  solen-vale.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief  电磁阀
***************************************************************************************************************
*/

#include <stdint.h>
#include "solen-vale.h"

SolenoidState Solenoid_State;	//电磁阀当前状态

void solen_vale_init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟

	GPIO_Initure.Pin=BI_PIN|SOLEN_POWER0N;    
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	GPIO_Initure.Pin=FI_PIN;    
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(FI_GPIO_PORT,&GPIO_Initure);

	HAL_GPIO_WritePin(BI_GPIO_PORT,BI_PIN,GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(FI_GPIO_PORT,FI_PIN,GPIO_PIN_RESET);	
	Solenoid_State = SLND_CLOSE;
}

/*
 * PowerOn:		打开电磁阀电源开关
 * 参数：			无
 * 返回参数： 无
*/
void SolenoidPowerOn(void)
{
	//12V电磁阀电源开关
	HAL_GPIO_WritePin(BI_GPIO_PORT,SOLEN_POWER0N,GPIO_PIN_SET);	 	
}

/*
 * PowerOn:	  关闭电磁阀电源开关
 * 参数：			无
 * 返回参数： 无
*/
void SolenoidPowerOff(void)
{
	HAL_GPIO_WritePin(BI_GPIO_PORT,SOLEN_POWER0N,GPIO_PIN_RESET);	 	
}

void solen_vale_open(void)
{
	//100ms正脉冲
	HAL_GPIO_WritePin(FI_GPIO_PORT,FI_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BI_GPIO_PORT,BI_PIN,GPIO_PIN_SET);
	HAL_Delay(100);	
	HAL_GPIO_WritePin(BI_GPIO_PORT,BI_PIN,GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(FI_GPIO_PORT,FI_PIN,GPIO_PIN_RESET);	
	Solenoid_State = SLND_OPEN;
}

void solen_vale_close(void)
{
	//100ms反脉冲
	HAL_GPIO_WritePin(BI_GPIO_PORT, BI_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(FI_GPIO_PORT, FI_PIN,GPIO_PIN_SET);
	HAL_Delay(100);	
	HAL_GPIO_WritePin(BI_GPIO_PORT, BI_PIN,GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(FI_GPIO_PORT, FI_PIN,GPIO_PIN_RESET);	
	Solenoid_State = SLND_CLOSE;
}

/*
 * GetSolenoidState:	获取当前电磁阀开关状态
 * 参数：				 			无
 * 返回值：				    当前电磁阀开关状态
*/
SolenoidState GetSolenoidState(void)
{
	return Solenoid_State;
}

/*
 * SetSolenoidState:	设置当前电磁阀开关状态
 * 参数：				 			无
 * 返回值：				    当前电磁阀开关状态
*/
SolenoidState SetSolenoidState(SolenoidState state)
{
	Solenoid_State=state;
	return Solenoid_State;
}
