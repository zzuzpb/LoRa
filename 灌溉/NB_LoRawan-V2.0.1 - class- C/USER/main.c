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
#include "board.h"


#define TEST_MULTIPLE_DATA															 1


/*!***********************************空中激活************************************/

#if ( OVER_THE_AIR_ACTIVATION == 1 ) 

extern uint8_t DevEui[8];
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

extern TimerEvent_t JoinReqTimer;
extern volatile bool IsNetworkJoined;
extern bool JoinReq_flag;

#endif

#define 	TEST_WATER				0x00
#define 	TEST_WATER_PRESSURE     0x00
#define 	TEST_WATER_FLOW			0x00
#define     LORA_MODE				0x04

#define LC4                { 472100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC5                { 472300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC6                { 472500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC7                { 472700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }
#define LC8                { 472900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 0 }


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

extern UART_HandleTypeDef 			UartHandle;
extern RTC_HandleTypeDef 			RtcHandle;
extern SPI_HandleTypeDef            SPI1_Handler;  


bool test_rtc_state = false;

/*******************************************************************************
  * @函数名称		main
  * @函数说明   主函数 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无

	版本说明：
	【1】 ：V2.0.1：MCU---stm32L0，主要用于控制设备室外;
	【2】 ：work on LOWER-RUN 存在问题：MCU时钟过低SPI读写频繁会出错，test_lora_pingpong.c不能测试lora，
				 使用LoRaWAN时不存在问题，但需要谨慎处理。

  优化功能：
	【1】 ：实现LORAWAN与网关通信。
	【2】 ：RTC停机唤醒机制。
	【3】 ：增加CAD机制，只使用3路信道：接收下行数据后判断信道然后直接发送数据；数据格式要求：上下行数据量越小越好。
	【4】 ：ABP/OTAA模式都具备，实际只使用ABP mode。
	【5】 ：NwkSKey、AppSKey采用内部固定不开放接口更改参数，DEVID、datarate暂时内定，待添加FLASH更改参数。
	【6】 ：lower-power run mode;
	【7】 ：待优化低功耗机制;
	【8】 ：增加CAD唤醒功能，响应速度优化：采用周期性CAD时间不使用其它外加延时，直接下发就可以唤醒;
	【9】 ：长期CAD mode侦听时1/15时间侦听CAD，其余时间进入休眠mode;
	【10】：周期性上报数据时：设备进入CAD mode等待下次发送数据或者接收数据.
	【11】：需要添加上报数据周期、控制水阀执行保护机制(**************)
	【12】：GPS测试失败：模块有问题
	【13】：增加RX2配置
	【14】  下发数据：唤醒数据帧+  开：开+开启时间  /关：关闭
	        上报数据：电量+水压+水脉冲+脉冲总时间
	【15】：添加手动模式开、关上报：待
	【16】：待机唤醒出现串口打印0：串口线原因
	【17】：数据上报时不存于CAD mode 防止下行控制响应灵敏度不够：正常模式开水阀需要RADIO+PAYLOAD 关水阀：PAYLOAD一帧数据
	        需要结合后台测试：(1)当解析出RADIO数据时，是否能正常接收下行数据
		                      (2)有效数据下发4S内没接收到应答，则再次下发，防止命令失效 
                              (3)关闭命令采用单帧数据，休眠时下发命令则在规定4S内没应答则再次下发，默认第一次为唤醒				
	【18】：添加心跳包：1次/h		
	【19】：待机测试关闭USART：2.5ma(max) BUG：会造成RTC时间定时不准，采用不关闭USART
					不关闭USART: 3ma(max)：不关闭USART功耗会大1ma左右
   	       
  *****************************************************************************/
/* variable functions ---------------------------------------------------------*/	

int main(void)
{	
	BoardInitMcu();	
	
	/********************上电模式选择：待机或者开机*********************/
//	WorkStatusJudgment( );
	
	DEBUG(2,"hello world NBI LoRaWAN\r\n");

	/******************读flash获取配置参数*****************/
	Read_Flash_Data(  );

	LoRaPower_Enable(  );
	user_app_init(app_mac_cb);
	
	LoRaMacSetRx2Channel( Rx2Channel );  ///配置RX2参数
	
	LoRaMacChannelAdd( 3, ( ChannelParams_t )LC4 );
	LoRaMacChannelAdd( 4, ( ChannelParams_t )LC5 );
	LoRaMacChannelAdd( 5, ( ChannelParams_t )LC6 );
	LoRaMacChannelAdd( 6, ( ChannelParams_t )LC7 );
	LoRaMacChannelAdd( 7, ( ChannelParams_t )LC8 );
   
	TimerInit( &ReportTimer, OnReportTimerEvent );
	TimerInit( &CadTimer, OnCadUnusualTimerEvent ); 
	TimerInit( &SleepTimer, OnsleepTimerEvent ); 
	TimerInit( &CSMATimer, OnCsmaTimerEvent ); 
	TimerInit( &HEARTimer, OnHearTimerEvent ); 
   
	LoRapp_SenSor_States.loramac_evt_flag = 0;
	LoRaCad.cad_all_time = 0;
	
	/******************空速初始化*****************/
	RF_Send_Data.AT_PORT = randr( 0, 0xDF );
	LoRaMacSetDeviceClass( CLASS_C );
	 	 
	LoRapp_SenSor_States.Tx_States = RFWRITE;
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
		 
	///使能GPS
//	Gps_Set(  ); 
    Set_Gps_Ack.GPS_DONE = true;
	/************GPS定位时LoRa休眠************/
	Radio.Sleep();
	
	/*****************设置心跳时间*****************/
	TimerStop( &HEARTimer );
	TimerSetValue( &HEARTimer, 60000000 ); ///1min 60000000
	TimerStart( &HEARTimer );
	
//#ifdef	TEST_WATER
//	Ctrl_12V_PowerOn( );
//#else 
//	Ctrl_12V_PowerOff( );
//#endif

	while (1)
	{	

//#ifdef	TEST_WATER_PRESSURE   	
//		SamplingData( WaterSensorsData.temp );
//		delay_ms(1000);
//#endif
		
//#ifdef 	TEST_WATER_FLOW
//		WaterSensorsData.pulsecount = 0;
//		delay_ms(1000);
//		DEBUG(2, "WaterSensorsData.pulsecount = %d\r\n",WaterSensorsData.pulsecount);
//		WaterSensorsData.pulsecount = 0;
//#endif	
		
//#ifdef 	LORA_MODE	
		
		if(!Set_Gps_Ack.GPS_DONE)
		{    
			Get_Gps_Position(  ); //发送GPS信息		
		}
		else  /***************GPS定位完成后再执行控制模式****************/
		{		 
			Irrigate_Control(  );
		}
//#endif	
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

