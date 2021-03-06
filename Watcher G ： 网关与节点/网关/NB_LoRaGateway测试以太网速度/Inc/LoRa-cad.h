/*
**************************************************************************************************************
*	@file	LoRa-cad.h
*	@author Jason<Jason_531@163.com>
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_CAD_H
#define __LORA_CAD_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdio.h>
#include <stdint.h>

typedef struct{
	bool Cad_Mode;
	bool Cad_Done;
	bool Cad_Detect;
	bool Iq_Invert;
	uint32_t Cad_Done_Count;
	float symbolTime;
	uint8_t UID[4]; ///接收到的设备ID
	uint32_t TimeOnAir; ///空口时间
	int32_t timeover;
	int32_t randtime1; ///发送随机时间范围1
	int32_t randtime2; ///发送随机时间范围2
	uint8_t cad_counter; ///重复扫描次数
	uint8_t Channel_Num; ///信道扫描次数：8通道 = 8次
	int16_t Rssi[8];
	uint8_t Channel_Scan[8];
}LoRaCad_t;

extern LoRaCad_t LoRaCad;

extern uint32_t Send_time; ///系统时间

extern void OnRxWindow1TimerEvent( void ); ///接收窗口
extern uint8_t Channel; ///信道号

void LoRa_Cad_Init(void);

float SymbolTime(void);

void LoRa_Cad_Mode(void);

void LoRa_Detect_Mode(void);

uint8_t Get_max(int8_t m,int16_t array[]);

uint8_t Bublesort(uint8_t a[],uint8_t n);

void LoRaMacChannelAddFun( void );

uint16_t CalcCRC16(uint8_t *data, uint8_t len);


#endif /* __LoRa-cad_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
