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

/************************设置CAD模式参数**************************/
LoRaCad_t LoRaCad = {true, false, false, false, 0, 0, {0}, 0, 0, 0, 0, 0, 0, {0}, {0}, 0};


TimerEvent_t CSMATimer;
volatile bool CSMATimerEvent = false;

void OnCsmaTimerEvent( void )
{  
	CSMATimerEvent = true;
	TimerStop( &CSMATimer );
}

/*
* LoRa CAD：防误触发操作
* LoRa CAD侦听异常保护机制,防止环境干扰触发CAD DETECT mode 8S时间重新切换为CAD侦听
*/
TimerEvent_t CadTimer;
void OnCadUnusualTimerEvent( void )
{  
	if( LoRapp_SenSor_States.Tx_States == RFWRITE )  ///CAD侦听等待状态下，状态切换，防止CAD异常触发
	{
		LoRapp_SenSor_States.Tx_States = RFREADY;
		DEBUG(2,"%s  LoRapp_SenSor_States = %d \r\n",__func__, LoRapp_SenSor_States.Tx_States );
	}
	TimerStop( &CadTimer );
}

/*
 * LoRa_Cad_Init:	 CAD初始化
 * 参数:				   无
 * 返回值:				 无
*/
void LoRa_Cad_Init(void)
{
	Radio.Standby();
	Radio.StartCad( );  // Set the device into CAD mode
}

///RF_Send_Data.default_datarate
float SymbolTime(uint8_t Datarate)
{
	LoRaCad.symbolTime = 0;
	uint8_t LORA_SPREADING_FACTOR = 0;
	switch(Datarate)
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
	 DEBUG(3,"LORA_SPREADING_FACTOR = %d symbolTime = %lf\r\n",LORA_SPREADING_FACTOR,LoRaCad.symbolTime);
	 return LoRaCad.symbolTime;
}


extern void OnadTimerEvent( void );

/*
 * LoRa_Cad_Mode:	 发送数据前信道检测
 * 参数:				   无
 * 返回值:				 无
*/
void LoRa_Cad_Mode(void)
{	
	LoRaCadtime = 0;
					
	Channel = Get_Flash_Datas.channels = randr( 5, 7 ); ///获取信道ID: flash读取
	
	LoRaCad.Iq_Invert = true;  ///使能节点间通信
	
	/*********************************监听信道：忙则再次监听，待空时发送*******************************/
	if(LoRaCad.Cad_Detect == true)
	{
		if(LoRapp_SenSor_States.Tx_States != RFWRITE)  ///排除掉CAD长期侦听模式
		{
		 DEBUG(2,"int cad again\r\n");
		
		 LoRaCad.Cad_Detect = false;
		}
	}	

#if 1
	LoRaCad.TimeOnAir = Radio.TimeOnAir( MODEM_LORA, (RF_Send_Data.TX_Len + 13) ); 
	
	DEBUG(2,"TimeOnAir = %d\r\n",LoRaCad.TimeOnAir);
	int32_t data[8] = {0.5*LoRaCad.TimeOnAir,1*LoRaCad.TimeOnAir,1.5*LoRaCad.TimeOnAir,2*LoRaCad.TimeOnAir,2.5*LoRaCad.TimeOnAir,
					   3*LoRaCad.TimeOnAir,3.5*LoRaCad.TimeOnAir,4*LoRaCad.TimeOnAir}; ///发送随机时间数组，该时间为空速：需要针对不同SF进行不同处理 paload + total time on air

	Get_Flash_Datas.sync_time = randr( 0, 3 ); ///非固定模式下：随机时隙								
	
	/********************sync_time时间范围判断*************************/										
	if(Get_Flash_Datas.sync_time == 0)
	{
		LoRaCad.randtime1 = -LoRaCad.TimeOnAir;
		LoRaCad.randtime2	= 0;			
	}
	else
	{
		LoRaCad.randtime1 = data[Get_Flash_Datas.sync_time-1];	
		LoRaCad.randtime2	= data[Get_Flash_Datas.sync_time];
		DEBUG(2,"sync_time = %d\r\n",Get_Flash_Datas.sync_time);				
	}	
	
	Radio.Standby();
	OnRxWindow1TimerEvent( ); ///设置接受模式为节点侦听模式
	
	LoRa_Cad_Init( ); ///注意：侦听完成获取数据需要重新再初始化Rx， 否则不接收数据
	TimerStop( &CSMATimer );
	TimerSetValue( &CSMATimer, LoRaCad.TimeOnAir + randr(LoRaCad.randtime1, LoRaCad.randtime2)); //+ randr(-1.5*TimeOnAir, 0)
	TimerStart( &CSMATimer );	

	LoRaCad.cad_all_time += LoRaCad.TimeOnAir + randr(LoRaCad.randtime1, LoRaCad.randtime2); ///保留触发CAD的总时间
		
	DEBUG(2,"GET UID LoRaCad.cad_all_time = %d LoRaCad.TimeOnAir = %d tt = %d\r\n",LoRaCad.cad_all_time, LoRaCad.TimeOnAir, LoRaCad.TimeOnAir + randr(LoRaCad.randtime1, LoRaCad.randtime2));
	while(!CSMATimerEvent);

#endif	
	
	LoRaCad.Channel_Num = 0;
		
	while(!LoRaCad.Cad_Detect && LoRaCad.Channel_Num<3)	
	{		
		LoRaCad.Cad_Done = false;

		Radio.Standby();
		OnRxWindow1TimerEvent( ); ///设置接受模式为节点侦听模式
		
		LoRa_Cad_Init( ); ///注意：侦听完成获取数据需要重新再初始化Rx， 否则不接收数据
						
		uint32_t symbolTime = SymbolTime(RF_Send_Data.default_datarate);	
		
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
		
		uint32_t t = LoRaCadtime;

		while(LoRaCad.Cad_Done != true && (LoRaCadtime - t < 50));	///50ms  
	}
			
	/********************RTC时间取整运算：低功耗唤醒当前不支持MS级，防止时间出现偏移***********************/
	LoRaCad.Iq_Invert = false;
	LoRaCad.Cad_Mode = false;
	Radio.Sleep();
}


extern TimerEvent_t SleepTimer;
extern volatile bool SleepTimerEvent;
extern void OnsleepTimerEvent( void );

/*
 * LoRa_Detect_Mode:	CAD侦听，进行CAD、RX mode切换 
 * 参数:				      无
 * 返回值:				    无
*/
void LoRa_Detect_Mode(void)
{
	int16_t rssi[2];
			
	///长期侦听模式
	if( !LoRaCad.Cad_Detect && (LoRapp_SenSor_States.Tx_States == RFWRITE) )
	{
		LoRaCad.Cad_Done = false;
		LoRaCad.Iq_Invert = false;
		
		LoRa_Cad_Init( ); ///注意：侦听完成获取数据需要重新再初始化Rx， 否则不接收数据
						
		uint32_t symbolTime = SymbolTime(Rx2Channel.Datarate);	
		
		delay_us( 240 ); 

		delay_us( (symbolTime + 240)/2 ); 
		rssi[0] = Radio.Rssi(MODEM_LORA); ///记录信号强度
		
		delay_us( (symbolTime + 240)/2 ); 
		rssi[1] = Radio.Rssi(MODEM_LORA); ///记录信号强度
					
		uint32_t t = LoRaCadtime;

		while((LoRaCad.Cad_Done != true) && (LoRaCadtime - t <50));	///50ms  
	 
		DEBUG(3,"get rssi[0] = %d rssi[1] = %d\r\n", rssi[0], rssi[1]);

		///此部分添加射频sleep，需要两个数据包，一数据包唤醒，一数据包控制数据，同时cad detect触发时SX1278不休眠
		if(LoRaCad.Cad_Detect == false) 
		{
			DEBUG(3,"LoRaCad.Cad_Detect != Cad_Detect %d\r\n", 15*( symbolTime + 480));
			
			SleepTimerEvent = false;
			TimerStop( &SleepTimer );
			TimerSetValue( &SleepTimer, 15 *( symbolTime + 480) ); 
			TimerStart( &SleepTimer );
			
			LoRapp_SenSor_States.WKUP_State = true;
			Into_Low_Power(  );	
			while(!SleepTimerEvent);
			
			LoRapp_SenSor_States.WKUP_State = false;
		}else if(LoRaCad.Cad_Detect) ///CAD侦听到数据，自动切换为接收模式10S后没接收到数据，10S后再次切换为CAD MODE：防止误操作
		{	
			DEBUG(3,"Rx_States222 = %d\r\n",LoRapp_SenSor_States.Tx_States);
			TimerStop( &CadTimer );
			TimerSetValue( &CadTimer, 10000000 );
			TimerStart( &CadTimer );		
		}
	}
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
