/*
**************************************************************************************************************
*	@file	main.c
*	@author Ysheng
*	@version V1.1
*	@date    2017/2/23
*	@brief	NBI_LoRaWAN功能代码: add OTAA
***************************************************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <math.h>
#include "stm32l0xx_hal.h"
#include "usart.h"
#include "rtc-board.h"
#include "timer.h"
#include "delay.h"
#include "board.h"
#include "user-app.h"
#include "LoRa-cad.h"

#define TEST_MULTIPLE_DATA											 1

#define VREFINT_CAL_ADDR                   							((uint16_t*) ((uint32_t)0x1FF80078U)) /* Internal voltage reference, address of parameter VREFINT_CAL: VrefInt ADC raw data acquired at temperature 30 DegC (tolerance: +-5 DegC), Vref+ = 3.0 V (tolerance: +-10 mV). */
#define VREFINT_CAL_VREF                   							((uint32_t) 3U)                    /* Analog voltage reference (Vref+) value with which temperature sensor has been calibrated in production (tolerance: +-10 mV) (unit: mV). */
#define VDD_APPLI                      		 						((uint32_t) 1220U)    /* Value of analog voltage supply Vdda (unit: mV) */
#define VFULL														((uint32_t) 4095U)


#define LC4                { 472100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 472300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 472500000, { ( ( DR_6 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 472700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 472900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }

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

/*
* LoRa CAD侦听异常保护机制,防止环境干扰触发CAD DETECT mode 8S时间重新切换为CAD侦听
*/
TimerEvent_t CadTimer;
void OncadTimerEvent( void )
{  
	if( LoRapp_SenSor_States.Rx_States == RFWRITE )
	LoRapp_SenSor_States.Rx_States = RFREADY;
	DEBUG(2,"%s  LoRapp_SenSor_States = %d \r\n",__func__, LoRapp_SenSor_States.Rx_States );
	TimerStop( &CadTimer );
}

/*
* LoRa休眠时间与非正常唤醒数据处理时间: 8S接收等待后RADIO切换为RFREADY状态
*/
void OnsleepTimerEvent( void )
{  
	SleepTimerEvent = true;
	TimerStop( &SleepTimer );
	if(LoRapp_SenSor_States.Rx_States == RADIO) ///等待RADIO状态一段时间后,恢复位侦听
	LoRapp_SenSor_States.Rx_States = RFREADY;
	DEBUG(2,"%s\r\n",__func__);
}

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


bool test_rtc_state = false;

bool first_time = false;

bool test_cad = false;

#define Temperature 								0x00	// Lux寄存器高字节的地址
#define Humidity					             	0x01


#define Hdc1080_WRITE_ADDR							0x80	// Hdc1080备地址: TI提供7Bit 地址，最低位Bit 缺省为0 则1000000 =  0x80

/* Register addresses */
#define Configuration                   			0x02
#define HDC_Manufacturer_ID							0xFE
#define HDC_Device_ID 								0xFF
#define HDC1080_EXP									16

#define Manufacturer_ID_value 						0x5449
#define Device_ID_value 							0x1050

#define ADC_MODE									0

#ifndef SUCCESS
#define SUCCESS                        				1
#endif

#ifndef FAIL
#define FAIL                            			0
#endif


/*******************************************************************************
  * @函数名称	main
  * @函数说明   主函数 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无

	版本说明：
	【1】：V2.0.2：MCU---stm32L0，数据采集设备 PRO II;

  优化功能：
	【1】： 实现LORAWAN与网关通信。
	【2】： RTC停机唤醒机制。
	【3】： 增加CAD机制，只使用5路信道：定时上报传感器数据
	【4】： ABP/OTAA模式都具备，实际只使用ABP mode。
	【5】： NwkSKey、AppSKey采用内部固定不开放接口更改参数，DEVID、datarate暂时内定，待添加FLASH更改参数。
	【6】： lower-power run mode
	【7】： 待优化低功耗机制
	【8】： BQ24195不使用软件配置，软件配置存在问题
	【9】： 关闭RX
				  LoRaMacSetReceiveDelay1( 200000 );
				  LoRaMacSetReceiveDelay2( 200000 );
	【10】: 该版本：GPS超时设置为30S用于测试定位，内部休眠时间不是分钟，正常版本需要更改
	【11】：空速默认=max 同时开启ADR自动更改使用
    【12】：V2.0.1上优化低功耗休眠处理
	
  *****************************************************************************/
/* variable functions ---------------------------------------------------------*/	

int main(void)
{	
   BoardInitMcu(  );	
   DEBUG(2,"TIME : %s  DATE : %s\r\n",__TIME__, __DATE__);
	
   user_app_init(app_mac_cb);
	
   LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
   LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
   
   TimerInit( &ReportTimer, OnReportTimerEvent );
   TimerInit( &CadTimer, OncadTimerEvent ); 
   TimerInit( &SleepTimer, OnsleepTimerEvent ); 
   TimerInit( &CSMATimer, OnCsmaTimerEvent ); 
   TimerInit( &GPSTimer, OnGpsTimerEvent );
	
   ReportTimerEvent = true;
   LoRapp_SenSor_States.loramac_evt_flag = 0;
   LoRaCad.cad_all_time = 0;
    
   RF_Send_Data.AT_PORT = randr( 1, 0xDF );

   RF_Send_Data.Get_sensor = true;

   RF_Send_Data.Send_Buf = (uint8_t *)malloc(sizeof(uint8_t)*64); ///使用指针必须分配地址空间，否则会出现HardFault_Handler错误
   samples.sockets = (uint8_t *)malloc(sizeof(uint8_t)*8); ///使用指针必须分配地址空间，否则会出现HardFault_Handler错误     

   DEBUG(3, "Battery = %d\r\n",CheckBattery( ));
   
   /********************只用于出厂验证服务器***********************/
   char *data = "deve";
   RF_Send_Data.RX_LEN = 5;
   RF_Send_Data.TX_Len = 5;
   do
   {								
      user_app_send(UNCONFIRMED, (uint8_t *)data, RF_Send_Data.TX_Len,3);
      HAL_Delay(3000);
   }
   while(!LoRapp_SenSor_States.loramac_evt_flag);
   
    memset(RF_Send_Data.Send_Buf, 0, RF_Send_Data.RX_LEN);
    RF_Send_Data.RX_LEN = 0;
    __disable_irq();
    LoRapp_SenSor_States.loramac_evt_flag = 0;
    __enable_irq();
   /**************************************************************/
    
   /***********开启GPS***********/	
//   GPS_Init(  );
//   Gps_Set(  );		

//   GpsSendAgainTime(  );
   
   /***********开启GPS***********/   
 	 
   while (1)
   {		
//	if(!Set_Gps_Ack.GPS_DONE)
//	{
//		Get_Gps_Position( );
//	}
//	else
	{
		User_Send_Api(  );	
	}		
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

