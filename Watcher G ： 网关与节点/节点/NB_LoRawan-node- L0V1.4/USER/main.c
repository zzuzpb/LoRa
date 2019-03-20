/*
**************************************************************************************************************
*	@file	main.c
*	@author Jason_531@163.com
*	@version V1.1
*	@date    2017/12/13
*	@brief	NBI_LoRaWAN功能代码: add OTAA
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rtc-board.h"
#include "timerserver.h"
#include "delay.h"
#include "board.h"
#include "user-app.h"
#include "etimer.h"
#include "autostart.h"


#ifndef SUCCESS
#define SUCCESS                         1
#endif

#ifndef FAIL
#define FAIL                            0
#endif


/*!***********************************空中激活************************************/

#if  OVER_THE_AIR_ACTIVATION

extern uint8_t DevEui[8];
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

extern TimerEvent_t JoinReqTimer;
extern volatile bool IsNetworkJoined;
extern bool JoinReq_flag;

#endif

LoRaMacRxInfo *loramac_rx_info;
mac_evt_t loramac_evt;

void app_mac_cb (mac_evt_t evt, void *msg)
{
    switch(evt){
    case MAC_STA_TXDONE:                
    case MAC_STA_RXDONE:
    case MAC_STA_RXTIMEOUT:
    case MAC_STA_ACK_RECEIVED:
    case MAC_STA_ACK_UNRECEIVED:
    case MAC_STA_CMD_JOINACCEPT:         
    case MAC_STA_CMD_RECEIVED:
         loramac_rx_info = msg;   ///mac层接收数据信息：rssi 端口等
         loramac_evt = evt;
         
         break;
    }
}


/*!***********************************分割线************************************/

extern UART_HandleTypeDef 			    UartHandle;
extern RTC_HandleTypeDef 				RtcHandle;
extern SPI_HandleTypeDef            	SPI1_Handler;  

bool TXSTATE = false;

PROCESS(GPSLocation_process,"GPSLocation_process");
PROCESS(SX1278Send_process,"SX1278Send_process");
AUTOSTART_PROCESSES(&GPSLocation_process,&SX1278Send_process);  // ,&SX1278Receive_process SX1278Send_process
void RFTXDONE(void)
{
	process_poll(&SX1278Send_process);
}

static process_event_t GPSLocationDone;

extern uint32_t UpLinkCounter;

bool Gps_Send_Stae = false;

/*!
 * Channels default datarate
 */
extern int8_t ChannelsDefaultDatarate;

PROCESS_THREAD(SX1278Send_process,ev,data)
{
	static struct etimer et;
    static uint32_t WorkTime = 0;
        
	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System SX1278Send Process..."); 
    
    etimer_set(&et,CLOCK_SECOND);	    
	while(1)
	{		
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
       		
//		Channel = 7; ///获取信道ID 4G模块获取       
 #if 1             
        PowerEnableLed(  ); 
        WorkTime = HAL_GetTick( );
       
		SamplingData( );											//获取传感器数据		
   		PowerDisbleLed(  ); 
        
        /***************************传感器数据存入发送缓存区*********************************/		
        memset(RF_Send_Data.Send_Buf,0,SAMPLE_SIZE);

		RF_Send_Data.Send_Buf[0] = samples.structver;
		RF_Send_Data.Send_Buf[1] = samples.count;
		for(uint8_t i = 0; i <= samples.socket_id; i++)
		{
			RF_Send_Data.Send_Buf[2+i] = samples.sockets[i];
			DEBUG(2,"%02x",RF_Send_Data.Send_Buf[2+i]);
		}
		DEBUG(2,"\r\nsamples.structver = 0x%02x\r\n",samples.structver);
		DEBUG(2,"Rechargeing:0x%02x; GPS:0x%02x;\r\n",RF_Send_Data.Rechargeing,RF_Send_Data.GPS);

		for(uint8_t i = 0, j = 0; i < samples.count; i++, j+=2)
		{
			RF_Send_Data.Send_Buf[samples.socket_id+3+j] = (samples.fields[i] >> 8)&0xff;
			RF_Send_Data.Send_Buf[samples.socket_id+4+j] = (samples.fields[i])&0xff;

			DEBUG(3,"%02x",RF_Send_Data.Send_Buf[samples.socket_id+3+j]);
			DEBUG(3,"%02x",RF_Send_Data.Send_Buf[samples.socket_id+4+j]);
		}
		RF_Send_Data.TX_Len = SAMPLE_SIZE;  //MAC+PHY=56  MAC = 13  SAMPLE_SIZE

        ///ALOHA+CSMA+多路径处理	        		
        CsmaTimerEvent = false;
        
        //开启接收模式
        Radio.Standby( );
        LoRaMacSetDeviceClass( CLASS_C ); ///设置接受模式为节点侦听模式

        /*********************************每次发送数据前在随机时间内监听信道：空闲则发送********************************/
        
        LoRaCsma.Disturb = true;  ///侦听模式
        int32_t Csmatime = randr(-10, 10)*TIMEONAIR;
        TimerStop( &CsmaTimer );
        TimerSetValue( &CsmaTimer, 10*TIMEONAIR + Csmatime); ///ALOHA
        TimerStart( &CsmaTimer );

        DEBUG(2,"TimeOnAir = %d\r\n", 10*TIMEONAIR + Csmatime);
        
        //浅度休眠            
        BoardSleep(  );

        ///sleep
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
      
        DEBUG(2,"CsmaTimerEvent\r\n");
        while(!CsmaTimerEvent);
        Radio.Standby( );
		LoRaMacSetDeviceClass( CLASS_A );
 
#else
        CsmaTimerEvent = true;
        RF_Send_Data.Send_Buf = "helloworld";
        RF_Send_Data.TX_Len = 15;

#endif 
        
        if(CsmaTimerEvent)
        {
            Gps_Send_Stae = false;
            RF_Send_Data.Send_Counter = 0;
                        
            do
            {
                if(UserAppSend(UNCONFIRMED, RF_Send_Data.Send_Buf, RF_Send_Data.TX_Len, 2) == 0)
                {
                    DEBUG(2,"Wait ACK app_send UpLinkCounter = %d\r\n", LoRaMacGetUpLinkCounter( ));
                    
                    PROCESS_YIELD_UNTIL(LoRapp_SenSor_States.loramac_evt_flag == 1);
                    LoRapp_SenSor_States.loramac_evt_flag = 0;
                    LoRaCsma.Disturb = false; ///解除侦听状态
                    CsmaTimerEvent = false;
                    LoRapp_SenSor_States.Work_Time = HAL_GetTick( ) - WorkTime;
                }
                else
                {
                    DEBUG(2,"app_send again\r\n");
                    Radio.Standby( );
                    etimer_set(&et,CLOCK_SECOND*4 + randr(-CLOCK_SECOND*4,CLOCK_SECOND*4));
                    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
                }
            }while(CsmaTimerEvent);
        }

        Gps_Send_Stae = true;
        
        PROCESS_YIELD_UNTIL(GPSLocationDone == ev);
        LoRapp_SenSor_States.WKUP_State = true; 
                
        if(Get_Flash_Datas.sleep_times>60)
        {
            Get_Flash_Datas.sleep_times -= (LoRapp_SenSor_States.Work_Time/1000);
        }
        DEBUG(2,"sleep_times = %d Work_Time = %d\r\n", Get_Flash_Datas.sleep_times,LoRapp_SenSor_States.Work_Time);
        LoRapp_SenSor_States.Work_Time = 0;
        SetRtcAlarm(60);  ///设置闹钟时间 Get_Flash_Datas.sleep_times
        IntoLowPower(  );                      
	}
	PROCESS_END();
}

extern bool rx_start;

PROCESS_THREAD(GPSLocation_process,ev,data)
{	
    static struct etimer et;

	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System GPSLocation Process..."); 
	etimer_set(&et,CLOCK_SECOND);	

	while(1)
	{	
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
        if(!Get_Gps_Ack.GPS_DONE)
        {
            DEBUG(3,"Get_Gps_Ack.GPS_DONE\r\n");
            GetGpsPosition( );
        }
        else
        {
            process_post(&SX1278Send_process,GPSLocationDone,NULL);
        }  
       etimer_reset(&et);
	}
    PROCESS_END(); 
}

void processset(void)
{
    process_init();
	process_start(&etimer_process,NULL); ///自动包含下面的线程
	autostart_start(autostart_processes);
}

/*******************************************************************************
  * @函数名称	main
  * @函数说明   主函数 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无

	版本说明：
	【1】：V2.1.1：MCU---stm32L0，数据采集设备 PRO II适应小网关;

	优化功能：
	【1】： 实现LORAWAN与小网关通信。
	【2】： RTC停机唤醒机制。
	【3】： ABP/OTAA模式都具备，实际只使用ABP mode。
	【4】： NwkSKey、AppSKey采用内部固定不开放接口更改参数，DEVID、datarate、Freq由外部写入更改与小网关进行配对
    【5】： 双频段FreqTX = FreqRX2  FreqRX1 = FreqTX+200khz
    【6】： FreqTX预留一频段作为FreqRX1偏移使用
    【7】： 增加GPS一个数据状态：0x40：定位中(GPS定位过程上报传感器数据，定位完成后再上报一次GPS数据，然后休眠)
    【8】： 增加CLASS A模式下行数据接收处理，解决频率差异导致接收数据失败
  *****************************************************************************/
/* variable functions ---------------------------------------------------------*/	

int main(void)
{	
	BoardInitMcu(  );	
	DEBUG(2,"TIME : %s  DATE : %s\r\n",__TIME__, __DATE__);
    
    /***********开启GPS***********/	
    GpsInit(  );
    GpsSet(  );			
        
//	MX_IWDG_Init(  );
	
//	HAL_IWDG_Refresh(&hiwdg); ///看门狗喂狗

    UserAppInit(app_mac_cb);
	LoRaMacTestRxWindowsOn( true ); ///开启接收窗口
    
    LoRaMacChannelAddFun( );
		
	Channel = 7; ///获取信道ID flash读取
	
	LoRaCsma.Iq_Invert = true;  ///使能节点间通信

	LoRapp_SenSor_States.loramac_evt_flag = 0;

	RF_Send_Data.AT_PORT = randr( 1, 0xDF );

	RF_Send_Data.Send_Buf = (uint8_t *)malloc(sizeof(uint8_t)*64); ///使用指针必须分配地址空间，否则会出现HardFault_Handler错误
	samples.sockets = (uint8_t *)malloc(sizeof(uint8_t)*8); ///使用指针必须分配地址空间，否则会出现HardFault_Handler错误

//	clock_init();
    processset( );

	USR_UsrLog("System Contiki InitSuccess...");	
	
    TimerInit( &GPSTimer, OnGpsTimerEvent );
    TimerInit( &CsmaTimer, OnCsmaTimerEvent );

    int8_t RSSI = Radio.Rssi(MODEM_LORA);
    printf("---hahahaRSSI--- : %d\r\n",RSSI);
    
    Radio.Sleep();
    	
	while (1)
	{		
		do
		{
		}while(process_run() > 0);     
	}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{ 
	DEBUG(2,"error\r\n");
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/*--------------------------------------------------------------------------------------------------------
                   									     0ooo											
                   								ooo0     (   )
                								(   )     ) /
                								 \ (     (_/
                								  \_)
----------------------------------------------------------------------------------------------------------*/

