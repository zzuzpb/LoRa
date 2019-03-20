/*
**************************************************************************************************************
*	@file	LoRa-cad.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include "LoRa-cad.h"

/******************************24路选择：TX1---23**************************************/
#define LC4                { 470900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 471100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 471300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 471500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 471700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }

#define LC9                { 478100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC10               { 478300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC11               { 478500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC12               { 478900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC13               { 479100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC14               { 479300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC15               { 479500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC16               { 479700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }

#define LC17               { 485900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC18               { 486100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC19               { 486300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC20               { 486500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC21               { 486700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC22               { 486900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC23               { 487100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC24               { 487300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }


/************************设置CAD模式参数**************************/
LoRaCsma_t LoRaCsma = {false, false, false, false, 0};

TimerEvent_t CsmaTimer;
volatile bool CsmaTimerEvent = false;
void OnCsmaTimerEvent( void )
{  
	TimerStop( &CsmaTimer );

	LoRaCsma.Disturb = false;
    
    LoRaCsma.Cad_Done = false;
    
    CsmaTimerEvent = true;
}

/*
*LoRaMacChannelAddFun：增加设备频段
*参数：                无
*返回值：              无
*/
void LoRaMacChannelAddFun( void )
{
	LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4  );
	LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5  );
	LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6  );
	LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7  );
	LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8  );
    LoRaMacChannelAdd( 8, ( ChannelParams_t )LC9  );
	LoRaMacChannelAdd( 9, ( ChannelParams_t )LC10 );
	LoRaMacChannelAdd( 10,( ChannelParams_t )LC11 );
	LoRaMacChannelAdd( 11,( ChannelParams_t )LC12 );
	LoRaMacChannelAdd( 12,( ChannelParams_t )LC13 );
    LoRaMacChannelAdd( 13,( ChannelParams_t )LC14 );
	LoRaMacChannelAdd( 14,( ChannelParams_t )LC15 );
	LoRaMacChannelAdd( 15,( ChannelParams_t )LC16 );
	LoRaMacChannelAdd( 16,( ChannelParams_t )LC17 );
	LoRaMacChannelAdd( 17,( ChannelParams_t )LC18 );
    LoRaMacChannelAdd( 18,( ChannelParams_t )LC19 );
	LoRaMacChannelAdd( 19,( ChannelParams_t )LC20 );
	LoRaMacChannelAdd( 20,( ChannelParams_t )LC21 );
	LoRaMacChannelAdd( 21,( ChannelParams_t )LC22 );
	LoRaMacChannelAdd( 22,( ChannelParams_t )LC23 );
    LoRaMacChannelAdd( 23,( ChannelParams_t )LC24 );    
}

/*
 * LoRaCadInit:	 CAD初始化
 * 参数:	     无
 * 返回值:		 无
*/
void LoRaCadInit(void)
{
	Radio.Standby();
	Radio.StartCad( );  // Set the device into CAD mode
}

float SymbolTime(void)
{
	LoRaCsma.symbolTime = 0;
	uint8_t LORA_SPREADING_FACTOR = 0;
	
	if(RF_Send_Data.default_datarate == 0)
		LORA_SPREADING_FACTOR = 12;
	else if(RF_Send_Data.default_datarate == 1)
		LORA_SPREADING_FACTOR = 11;
	else if(RF_Send_Data.default_datarate == 2)
		LORA_SPREADING_FACTOR = 10;
	else if(RF_Send_Data.default_datarate == 3)
		LORA_SPREADING_FACTOR = 9;
	else if(RF_Send_Data.default_datarate == 4)
		LORA_SPREADING_FACTOR = 8;
	else 
		LORA_SPREADING_FACTOR = 7;
	
	 LoRaCsma.symbolTime = (( pow( (float)2, (float)LORA_SPREADING_FACTOR ) ) + 32 ) / 125000;  // SF7 and BW = 125 KHz
	 LoRaCsma.symbolTime = LoRaCsma.symbolTime * 1000000;  // symbol Time is in us
	 DEBUG(3,"LORA_SPREADING_FACTOR = %d symbolTime = %d\r\n",LORA_SPREADING_FACTOR,LoRaCsma.symbolTime);
	 return LoRaCsma.symbolTime;
}

