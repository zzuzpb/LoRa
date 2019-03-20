/*
**************************************************************************************************************
*	@file	control.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief  控制设备文件
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CONTROL_H
#define __CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define  KEY_ON         1
#define  KEY_OFF        0

typedef struct{
	bool 		Auto_Mode;
	uint8_t control_buf[4];
	uint8_t send_buf[4];
	uint8_t retry_conter;	
}Control_State;

extern Control_State Control_States;

uint8_t HextoDec(uint8_t *hex, uint8_t length) ;

void Get_Work_ModeInit( void );
void Control_Relay_Init( void );
void Enable_Stm8_Power( void );
void Check_Key_Mode( void );
void Control_Relay(uint8_t *rfdata);


#endif /* __CONTROL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
