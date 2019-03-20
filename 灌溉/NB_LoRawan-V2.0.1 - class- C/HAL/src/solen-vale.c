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
#include "sensor.h"
#include "user-app.h"

SolenoidState Solenoid_State;	//电磁阀当前状态

/*
 *solen_vale_init：电磁阀初始化
 *参数：		 无
 *返回值：		 无
*/
void solen_vale_init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟
	__HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟	
	
	GPIO_Initure.Pin=BI_PIN;    
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(BI_GPIO_PORT,&GPIO_Initure);
	
	GPIO_Initure.Pin=FI_PIN;    
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(FI_GPIO_PORT,&GPIO_Initure);
	
	HAL_GPIO_WritePin(BI_GPIO_PORT,BI_PIN,GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(FI_GPIO_PORT,FI_PIN,GPIO_PIN_RESET);	
	
	Ctrl_12V_PowerOff(  );
	Solenoid_State = SLND_CLOSE;
}

/*
 * Ctrl_12V_PowerInit: 初始化12V电源开关
 * 参数：	 		   无
 * 返回参数：		   无
*/
void Ctrl_12V_PowerInit(void)
{	
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟	
	GPIO_Initure.Pin=CTRL_12V_POWERON;    
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
	HAL_GPIO_Init(CTRL_12V_PORT,&GPIO_Initure);
}

/*
 * PowerOn:		打开电磁阀电源开关
 * 参数：		无
 * 返回参数：	无
*/
void Ctrl_12V_PowerOn(void)
{
	//12V电磁阀电源开关
	HAL_GPIO_WritePin(CTRL_12V_PORT,CTRL_12V_POWERON,GPIO_PIN_SET);	 	
}

/*
 * PowerOn:	  关闭电磁阀电源开关
 * 参数：	  无
 * 返回参数： 无
*/
void Ctrl_12V_PowerOff(void)
{
	HAL_GPIO_WritePin(CTRL_12V_PORT,CTRL_12V_POWERON,GPIO_PIN_RESET);	 	
}

/*
 * solen_vale_open:	 打开电磁阀
 * 参数：	 		 无
 * 返回参数：		 无
*/
void solen_vale_open(void)
{
	//100ms正脉冲
	HAL_GPIO_WritePin(BI_GPIO_PORT,BI_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(FI_GPIO_PORT,FI_PIN,GPIO_PIN_SET);
	HAL_Delay(100);	
	HAL_GPIO_WritePin(BI_GPIO_PORT,BI_PIN,GPIO_PIN_RESET);	
	HAL_GPIO_WritePin(FI_GPIO_PORT,FI_PIN,GPIO_PIN_RESET);	
	Solenoid_State = SLND_OPEN;

	WaterSensorsData.pulsecount = 0;
}

/*
 * solen_vale_close: 关闭电磁阀
 * 参数：	 		 无
 * 返回参数：		 无
*/
void solen_vale_close(void)
{
	//100ms反脉冲
	HAL_GPIO_WritePin(FI_GPIO_PORT, FI_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BI_GPIO_PORT, BI_PIN,GPIO_PIN_SET);
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

/*
 *WaterMCounter：水阀手动关闭次数计算
 *参数：		 无
 *返回值：		 无
*/
void WaterMCounter(void)
{
 RF_Send_Data.Send_Buf[2] = LoRapp_SenSor_States.WaterMCounter&0xff;
 RF_Send_Data.Send_Buf[3] = (LoRapp_SenSor_States.WaterMCounter>>8)&0xff;
 RF_Send_Data.Send_Buf[4] = (LoRapp_SenSor_States.WaterMCounter>>16)&0xff;
 RF_Send_Data.Send_Buf[5] = (LoRapp_SenSor_States.WaterMCounter>>24)&0xff;

 if(LoRapp_SenSor_States.WaterMCounter<0xff)
 {
	 RF_Send_Data.RX_LEN = 3;
 }
 else if(LoRapp_SenSor_States.WaterMCounter>0xff && LoRapp_SenSor_States.WaterMCounter<0xffff)
 {
	 RF_Send_Data.RX_LEN = 4;
 }
 else if(LoRapp_SenSor_States.WaterMCounter>0xffff && LoRapp_SenSor_States.WaterMCounter<0xffffff)
 {
	 RF_Send_Data.RX_LEN = 5;
 }
 else 
 {
	 RF_Send_Data.RX_LEN = 6;
 }
}

/*
 *WaterMOCounter：水阀手动打开次数计算
 *参数：		  无
 *返回值：		  无
*/
void WaterMOCounter(void)
{
 /************************32位数据长度只取实际数据长度发送,节省发送***************************/
 RF_Send_Data.Send_Buf[2] = LoRapp_SenSor_States.WaterMOCounter&0xff;
 RF_Send_Data.Send_Buf[3] = (LoRapp_SenSor_States.WaterMOCounter>>8)&0xff;
 RF_Send_Data.Send_Buf[4] = (LoRapp_SenSor_States.WaterMOCounter>>16)&0xff;
 RF_Send_Data.Send_Buf[5] = (LoRapp_SenSor_States.WaterMOCounter>>24)&0xff;

 if(LoRapp_SenSor_States.WaterMOCounter<0xff)
 {
	 RF_Send_Data.RX_LEN = 3;
 }
 else if(LoRapp_SenSor_States.WaterMOCounter>0xff && LoRapp_SenSor_States.WaterMOCounter<0xffff)
 {
	 RF_Send_Data.RX_LEN = 4;
 }
 else if(LoRapp_SenSor_States.WaterMOCounter>0xffff && LoRapp_SenSor_States.WaterMOCounter<0xffffff)
 {
	 RF_Send_Data.RX_LEN = 5;
 }
 else 
 {
	 RF_Send_Data.RX_LEN = 6;
 }
}


uint8_t GetBattery = 0;

/*
 *OnLoRaHeartDeal：LoRa心跳处理函数
 *参数：		   无
 *返回值：		   无
*/
void OnLoRaHeartDeal(void)
{
	ReportTimerEvent = true;
	GetBattery = CheckBattery(  );

	RF_Send_Data.Send_Buf[0] = GetBattery;
	if (GetBattery<=5)					//电池电量低
	{
		RF_Send_Data.Send_Buf[1] = 'L';
	}
	else
	{
	  RF_Send_Data.Send_Buf[1] = 'H';
	}
					
	RF_Send_Data.RX_LEN = 2;
	do
	{
		/**************************发送心跳包前检查是否有下行控制存在，有则退出**************************/
		if(LoRapp_SenSor_States.Tx_States == RFBATHEART) 
		{
			///发送：执行完成：原数据返回
			User_send(UNCONFIRMED, RF_Send_Data.Send_Buf);	
		}
		else ///发送前状态被更改，直接退出
		{
			DEBUG(2, "go break\r\n");
			LoRapp_SenSor_States.loramac_evt_flag = 0;
			ReportTimerEvent = false;
			break;
		}		
	}while(!LoRapp_SenSor_States.loramac_evt_flag);
	
	/*******************清除发送完成标志*********************/
	__disable_irq();
	LoRapp_SenSor_States.loramac_evt_flag = 0;
	ReportTimerEvent = false;
	__enable_irq();
	
	if (GetBattery<=5)					//电池电量低
	{
		DEBUG(2,"battery extremely low;enter standby\r\n");
		StandbyEnterMode( );					//电池电量特别低，直接重置进入休眠
	}
	LoRapp_SenSor_States.Tx_States = SaveTxState;
}

/*
 *OnLoRaAckDeal：LoRa ACK应答处理
 *参数：		 无
 *返回值：		 无
*/
void OnLoRaAckDeal(void)
{
	TimerStop( &CadTimer ); ///关闭CAD异常保护
	ReportTimerEvent = true;

	memset(RF_Send_Data.Send_Buf, 0, strlen((char *)RF_Send_Data.Send_Buf));
	memcpy(RF_Send_Data.Send_Buf, "AO", strlen("AO")); 

	RF_Send_Data.RX_LEN = strlen("AO");
	do
	{
		DEBUG(2,"RF_Send_Data.Send_Buf = %s \r\n",RF_Send_Data.Send_Buf);
		///发送：执行完成：原数据返回
		User_send(UNCONFIRMED, RF_Send_Data.Send_Buf);	
	}while(!LoRapp_SenSor_States.loramac_evt_flag);
	
	/*******************清除发送完成标志*********************/
	__disable_irq();
	LoRapp_SenSor_States.loramac_evt_flag = 0;
	__enable_irq();
	LoRapp_SenSor_States.Tx_States = WATERAUTOPEN;
	ReportTimerEvent = true;
}

/*
 *OnWaterAutopen：水阀自动控制开操作
 *参数：		  无
 *返回值：		  无
*/
void OnWaterAutopen(void)
{
	if(LoRapp_SenSor_States.Water_Control_State == OPEN) ///开启水阀标志位
	{
		LoRapp_SenSor_States.Water_Control_State = WRITE;
		
		solen_vale_init(  );
		Ctrl_12V_PowerOn(  );
		solen_vale_open( );			
	}
		
	if(ReportTimerEvent)
	{
		DEBUG(2,"line = %d\r\n",__LINE__);
		//获取电量数据 读取当前水压： 水压：WaterSensorsData.temp[1]*1.6Pa/5v
		 SamplingData( WaterSensorsData.temp );
		
		 if(WaterSensorsData.temp[0] < 5) ///电量低于5%，则关闭水阀设备，进入电量过低状态
		 {
			solen_vale_close( );
			Ctrl_12V_PowerOff( ); 
			LoRapp_SenSor_States.Tx_States = RFBATHEART;
			
			return;
		 }
		 
		 DEBUG(2, "WaterSensorsData.pulsecount = %d\r\n",WaterSensorsData.pulsecount);
		 memset(RF_Send_Data.Send_Buf, 0, 8);
		 memcpy(RF_Send_Data.Send_Buf, &WaterSensorsData, sizeof(WaterSensorsData));   //MAC+PHY=56  MAC = 13 
		 RF_Send_Data.RX_LEN = 8;
		do
		{								
			///发送前判断是否接收到关闭命令，是则强制退出
			if(LoRapp_SenSor_States.Water_Control_State != CLOSE)
			{		
				User_send(UNCONFIRMED, RF_Send_Data.Send_Buf);
			}
			else
			{
				DEBUG(2, "brak\r\n");
				LoRapp_SenSor_States.loramac_evt_flag = 0;
				ReportTimerEvent = false;					
				break;
			}				
		}while(!LoRapp_SenSor_States.loramac_evt_flag);  

		/*******************清除发送完成标志*********************/
		__disable_irq();
		LoRapp_SenSor_States.loramac_evt_flag = 0;
		__enable_irq();
		
		/******************数据周期性上报***********************/
		ReportTimerEvent = false;
		TimerStop( &ReportTimer );
		TimerSetValue( &ReportTimer, 16000000-LoRaCad.cad_all_time );
		TimerStart( &ReportTimer );	
		LoRaCad.cad_all_time = 0;
	}
}

/*
 *OnWaterAutClose：水阀自动模式关闭操作
 *参数：		   无
 *返回值：		   无
*/
void OnWaterAutClose(void)
{
	if(LoRapp_SenSor_States.Water_Control_State == CLOSE) /// 关闭标志位 "AC"
	{	
		LoRapp_SenSor_States.Water_Control_State = WRITE;
		
		TimerStop( &CadTimer ); ///关闭CAD异常保护
	
		///如果接收到关闭指令
		///关闭电磁阀+清零水流量脉冲计数
		solen_vale_init(  );
		Ctrl_12V_PowerOn(  );
		Ctrl_12V_PowerOff( );

		WaterSensorsData.pulsecount = 0; 	
	}
	
	TimerStop( &ReportTimer ); ///关闭ReportTimer周期
	ReportTimerEvent = true;
	///发送：执行完成：原数据返回
	memset(RF_Send_Data.Send_Buf, 0, 6);
	memcpy(RF_Send_Data.Send_Buf, "AC", strlen("AC"));  ///自动关闭
	
	RF_Send_Data.RX_LEN = strlen("AC");
	
	do
	{
		User_send(UNCONFIRMED, RF_Send_Data.Send_Buf);	
	}while(!LoRapp_SenSor_States.loramac_evt_flag);
/*******************清除发送完成标志*********************/
	__disable_irq();
	LoRapp_SenSor_States.loramac_evt_flag = 0;
	__enable_irq();		

	LoRapp_SenSor_States.Tx_States = RFREADY;
}

/*
 *WaterManOpen：水阀手动模式打开操作
 *参数：		无
 *返回值：		无
*/
void WaterManOpen(void)
{
	 TimerStop( &ReportTimer ); ///关闭ReportTimer周期
	 ReportTimerEvent = true;
	 memset(RF_Send_Data.Send_Buf, 0, strlen((char *)RF_Send_Data.Send_Buf));
   ///发送：执行完成：原数据返回
	 memcpy(RF_Send_Data.Send_Buf, "MO", strlen("MO")); 

	 WaterMOCounter(  );

	do
	{
		User_send(UNCONFIRMED, RF_Send_Data.Send_Buf);	
	}while(!LoRapp_SenSor_States.loramac_evt_flag);
	/*******************清除发送完成标志*********************/
	__disable_irq();
	LoRapp_SenSor_States.loramac_evt_flag = 0;
	__enable_irq();
	
	LoRapp_SenSor_States.Tx_States = RFREADY;
}

/*
 *WaterManClose：水阀手动模关闭操作
 *参数：		 无
 *返回值：		 无
*/
void WaterManClose(void)
{
	TimerStop( &ReportTimer ); ///关闭ReportTimer周期
	ReportTimerEvent = true;
	memset(RF_Send_Data.Send_Buf, 0, strlen((char *)RF_Send_Data.Send_Buf));
	///发送：执行完成：原数据返回
	memcpy(RF_Send_Data.Send_Buf, "MC", strlen("MC")); 

	WaterMCounter(  );
	
	do
	{
		User_send(UNCONFIRMED, RF_Send_Data.Send_Buf);	
	}while(!LoRapp_SenSor_States.loramac_evt_flag);
	/*******************清除发送完成标志*********************/
	__disable_irq();
	LoRapp_SenSor_States.loramac_evt_flag = 0;
	__enable_irq();
	
	LoRapp_SenSor_States.Tx_States = RFREADY;
}
