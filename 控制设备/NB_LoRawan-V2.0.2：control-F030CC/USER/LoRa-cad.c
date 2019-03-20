/*
**************************************************************************************************************
*	@file	LoRa-cad.c
*	@author Ysheng
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


/*************************以下为CAD功能代码*************************/

extern void OnRxWindow1TimerEvent( void );
extern void OnRxWindow2TimerEvent( void );
//extern const uint8_t Datarates[];
//extern uint8_t SF_darate;

void LoRa_Cad_Init(void)
{
	Radio.Standby();
	Radio.StartCad( );  // Set the device into CAD mode
}

float SymbolTime(void)
{
	LoRaCad.symbolTime = 0;
	uint8_t LORA_SPREADING_FACTOR = 0;
	switch(RF_Send_Data.default_datarate)
	{
		case 0:  //12 -- 51
		LORA_SPREADING_FACTOR = 12;
		break;
		case 1:  //11 -- 51
		LORA_SPREADING_FACTOR = 11;
		break;
		case 2:  //10 -- 51
		LORA_SPREADING_FACTOR = 10;
		break;
		case 3:  //9 --- 115
		LORA_SPREADING_FACTOR = 9;
		break;
		case 4:  //8 --- 222
		LORA_SPREADING_FACTOR = 8;
		break;
		case 5:  //7 --- 222
		LORA_SPREADING_FACTOR = 7;
		break;
		default: break;
	}
	 LoRaCad.symbolTime = (( pow( (float)2, (float)LORA_SPREADING_FACTOR ) ) + 32 ) / 125000;  // SF7 and BW = 125 KHz
	 LoRaCad.symbolTime = LoRaCad.symbolTime * 1000000;  // symbol Time is in us
	 DEBUG(4,"LORA_SPREADING_FACTOR = %d symbolTime = %lf\r\n",LORA_SPREADING_FACTOR,LoRaCad.symbolTime);
	 return LoRaCad.symbolTime;
}


extern void OnadTimerEvent( void );

void LoRa_Cad_Mode(void)
{	
	Send_time = 0;
					
	Channel = Get_Flash_Datas.channels = randr( 5, 7 ); ///获取信道ID: flash读取
	
	LoRaCad.Iq_Invert = true;  ///使能节点间通信
	
	/*********************************监听信道：忙则再次监听，待空时发送*******************************/
	if(LoRaCad.Cad_Detect == true)
	{
		DEBUG(2,"int cad again\r\n");
		
		LoRaCad.Cad_Detect = false;
	}									
	
	DEBUG(2,"int cad MODE\r\n");
	LoRaCad.Channel_Num = 0;
		
	while(!LoRaCad.Cad_Detect&& LoRaCad.Channel_Num<3)	///
	{		
		LoRaCad.Cad_Done = false;

		Radio.Standby();
		OnRxWindow1TimerEvent( ); ///设置接受模式为节点侦听模式
		
		LoRa_Cad_Init( ); ///注意：侦听完成获取数据需要重新再初始化Rx， 否则不接收数据
						
		uint32_t symbolTime = SymbolTime();	
		
		delay_us( 240 ); 

		delay_us( symbolTime + 240 ); 
						 
		LoRaCad.Rssi[LoRaCad.Channel_Num]  = Radio.Rssi(MODEM_LORA); ///记录信号强度
		LoRaCad.Channel_Scan[LoRaCad.Channel_Num] = Channel;		///记录信道ID
					
		LoRaCad.Rssi[LoRaCad.Channel_Num] = ~(LoRaCad.Rssi[LoRaCad.Channel_Num] - 1); ///负数转正数
	
		DEBUG(2,"symbolTime = %d,rssi[0] = %d Channel = %d\r\n", symbolTime,LoRaCad.Rssi[LoRaCad.Channel_Num],LoRaCad.Channel_Scan[LoRaCad.Channel_Num]);
		
		LoRaCad.Channel_Num++;
		Channel++;
		
		if(Channel==8)
			Channel = 5;
		
		uint32_t t = Send_time;

		while(LoRaCad.Cad_Done != true && (Send_time - t < 50));	///50ms  
	}
			
	/********************RTC时间取整运算：低功耗唤醒当前不支持MS级，防止时间出现偏移***********************/
	LoRaCad.Iq_Invert = false;
	LoRaCad.Cad_Mode = false;
	Radio.Sleep();
}

uint8_t Get_max(int8_t m,int16_t array[])
{
	uint8_t channel_scan[8] = {0};
	uint8_t channel_new[8] = {0};

	int max,min;
	int8_t t = 0;
	
	max=min=array[0];  
	for(int8_t i=1;i<m;i++)  
	{    
		if(max<array[i]) 
		{
			max=array[i];
		}			
		else if(min>array[i]) 
		{			
			min=array[i]; 
		}
	}	
	
	for(int8_t j=0;j<m;j++)  
	{	
		if(max == array[j])
		{
			channel_scan[t] = j; ///记录相同RSSI时的信道ID号数组下标
			channel_new[t]= LoRaCad.Channel_Scan[channel_scan[t]];	///保存RSSI下读取到的信道ID		
			t++;
		}
	}

	uint8_t ID = randr( 0, t-1 ); 
	Channel = channel_new[ID]; 		
	return Channel;
}

uint8_t Bublesort(uint8_t a[],uint8_t n)
{
 int i,j,k;
	
 for(j=0;j<n-1;j++)   /* 冒泡法排序n次 */
 {
	for(i=0;i<n-j-1;i++)  /* 值比较大的元素沉下去，只把剩下的元素最大值再沉下去 */
	{
		 if(a[i]>a[i+1])  /* 最大值沉到底 */
		 {
				k=a[i];
				a[i]=a[i+1];
				a[i+1]=k;
		 }
	}
 }
 return *a;
}
