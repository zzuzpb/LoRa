/*
**************************************************************************************************************
*	@file	user-app.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	协议采用分层方式：mac app进行区分
***************************************************************************************************************
*/
#include <math.h>
#include "user-app.h"
#include "board.h"
#include "LoRaMac.h"
#include "LoRa-cad.h"
#include "LoRaMac-api-v3.h"


#define APP_DATA_SIZE                                   (43)
#define APP_TX_DUTYCYCLE                                (100000)     // 5min
#define APP_TX_DUTYCYCLE_RND                            (100)   // ms
#define APP_START_DUTYCYCLE                             (10000)     // 3S

/*!
 * Join requests trials duty cycle.
 */
#define OVER_THE_AIR_ACTIVATION_DUTYCYCLE           	10000 // 10 [s] value in ms

mac_callback_t mac_callback_g;

volatile bool IsNetworkJoined = false;

bool JoinReq_flag = true;

/*!
 * Defines the join request timer
 */
TimerEvent_t JoinReqTimer;

uint8_t DevEui[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t DEV[4] = {
   0 
};

static uint8_t NwkSKey[] = {
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};
static uint8_t AppSKey[] = {
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

static uint32_t DevAddr;
//static LoRaMacEvent_t LoRaMacEvents;
static LoRaMacCallbacks_t LoRaMacCallbacks;
LoRaMacRxInfo RxInfo;

/************************射频发送数据状态判断************************/
LoRapp_State LoRapp_SenSor_States = {false, false, false, WRITE, 0, RFWRITE, 0};

/************************设置RF分包发送参数************************/
RF_Send RF_Send_Data = {0, 0, 0, 0, {0}, 0, 5, 0, 0, 0, false, false, true}; 

/************************读取flash数据设置射频参数**************************/
Get_Flash_Data Get_Flash_Datas = {false, 0, 2, 0, 0, 5, 0, 0};


/****************************OTAA参数******************************
*********【1】read flash or get buy AT commed.
*********/

uint8_t AppEui[16] = {0};  
uint8_t AppKey[32] = {0};

sys_sta_t sys_sta = SYS_STA_IDLE;

/*!
 * \brief Function executed on JoinReq Timeout event
 */
void OnJoinReqTimerEvent( void )
{
	TimerStop( &JoinReqTimer );
	JoinReq_flag = true;
	DEBUG(2,"OnJoinReqTimerEvent \r\n");
}

LoRaFrameType_t LoRaFrameType;

//extern void RFTXDONE(void);

/*!
 * \brief Function to be executed on MAC layer event
 */
void OnMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )///MAC层发送、接收状态判断、数据处理
{
	switch( info->Status )
	{
	case LORAMAC_EVENT_INFO_STATUS_OK:
			break;
	case LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT:;
			break;
	case LORAMAC_EVENT_INFO_STATUS_ERROR:
			break;
	default:
			break;
	}

	if( flags->Bits.JoinAccept == 1 )
	{       
		DEBUG(2,"join done\r\n");
		TimerStop( &JoinReqTimer );
		IsNetworkJoined = true;
		Rx_Led( );
		mac_callback_g(MAC_STA_CMD_JOINACCEPT, NULL);
	}  
  
	if( info->TxAckReceived == true )
	{ 
		/// McpsConfirm.AckReceived = true;接收回调函数
		if(mac_callback_g!=NULL)
		{
			mac_callback_g(MAC_STA_ACK_RECEIVED, &RxInfo); ///相当于调用app_lm_cb函数
			LoRapp_SenSor_States.Ack_Recived = true;    
			DEBUG(2,"ACK Received\r\n");
		}
	}
	else if((flags->Bits.Rx != 1) && (flags->Bits.JoinAccept != 1) && (LoRaFrameType == CONFIRMED))
	{
		DEBUG(2,"=====NO ACK REPLY=====\r\n");     
		mac_callback_g(MAC_STA_ACK_UNRECEIVED, NULL);        
	}
	
	if( flags->Bits.Rx == 1 )
  {  ///接收到数据，数据信息保存RxData: 网关应答数据默认不打印接收信息
		RxInfo.size = info->RxBufferSize;
		memcpy(RxInfo.buf, info->RxBuffer, RxInfo.size);
		RxInfo.rssi = info->RxRssi;
		RxInfo.snr = info->RxSnr;
		RxInfo.win = flags->Bits.RxSlot+1;
		RxInfo.port = info->RxPort;
		Rx_Led( );
		DEBUG(3,"win = %d snr = %d rssi = %d size = %d \r\n",RxInfo.win, RxInfo.snr, RxInfo.rssi, RxInfo.size);
		if(flags->Bits.RxData == 1)
		{
			if(mac_callback_g!=NULL)
			{
			  mac_callback_g(MAC_STA_RXDONE, &RxInfo);
			  if( RxInfo.size>0 )
              {
				 DEBUG(3,"RxInfo.buf:");
				 for( uint8_t i = 0; i < RxInfo.size; i++ )
				 DEBUG(3,"%02x",RxInfo.buf[i]);
				 DEBUG(3,";\r\n");									
			  }
              
              if(strstr((char *)RxInfo.buf, "deves") != NULL)
              {
                DEBUG(2,"recever ok\r\n");
              }
						
			  if(RxInfo.buf[0] == '_')
			  Get_Flash_Datas.sleep_times = Convert16To10(RxInfo.buf[1]);
		  }
			memset(RxInfo.buf, 0, strlen((char *)RxInfo.buf));
		}     
	}
	 
	if( flags->Bits.Tx == 1 )
	{
		if(mac_callback_g!=NULL)
		{
			mac_callback_g(MAC_STA_TXDONE, NULL);
			if( flags->Bits.JoinAccept == 1 ) ///如果是OTAA入网请求应答，则直接发送数据不需要等待，否则等待
			{
				LoRapp_SenSor_States.loramac_evt_flag = 0;
			}else
			LoRapp_SenSor_States.loramac_evt_flag = 1;

			DEBUG(2,"Done\r\n");
			RFTXDONE(  );		
			Send_Led( );
		}
	}
}
void user_app_init(mac_callback_t mac)
{
	LoRaMacCallbacks.MacEvent = OnMacEvent; ///MAC层数据接口
	LoRaMacCallbacks.GetBatteryLevel = NULL;
	LoRaMacInit( &LoRaMacCallbacks );

	IsNetworkJoined = false;
	
#if( OVER_THE_AIR_ACTIVATION == 0 )  
    
	DevAddr  = DEV[3];
	DevAddr |= (DEV[2] << 8);
	DevAddr |= (DEV[1] << 16);
	DevAddr |= (DEV[0] << 24);
	DEBUG(3,"DevAddr : %02x-%02x-%02x-%02x\r\n",DEV[0],DEV[1],DEV[2],DEV[3]);
	
	LoRaMacInitNwkIds( 0x000000, DevAddr, NwkSKey, AppSKey );
	IsNetworkJoined = true;

#else
     // Initialize LoRaMac device unique ID : 空中激活时作为激活参数
	BoardGetUniqueId( DevEui );
	for(uint8_t i = 0; i < 8; i++)
	DEBUG(2,"%02x ", DevEui[i]);
	DEBUG(2,"\r\n");

	
	 // Sends a JoinReq Command every OVER_THE_AIR_ACTIVATION_DUTYCYCLE
	// seconds until the network is joined
	TimerInit( &JoinReqTimer, OnJoinReqTimerEvent );
	TimerSetValue( &JoinReqTimer, OVER_THE_AIR_ACTIVATION_DUTYCYCLE );
	
#endif
  
	LoRaMacSetAdrOn( Get_Flash_Datas.LoRaMacSetAdrOnState );
	LoRaMacTestSetDutyCycleOn(false);
	
	mac_callback_g = mac;

}

char String_Buffer[33]; ///读取flash写入字符串

int PowerXY(int x, int y)
{
	if(y == 0)
	return 1 ;
	else
	return x * PowerXY(x, y -1 ) ;
}
/*!
*Convert16To10：16进制转化为10进制
*返回值: 		     		  10进制数值
*/
int Convert16To10(int number)
{
	int r = 0 ;
	int i = 0 ;
	int result = 0 ;
	while(number)
	{
		r = number % 16 ;
		result += r * PowerXY(16, i++) ;
		number /= 16 ;
	}
	return result ;
}

/*!
*Read_DecNumber：字符串中的数字转化为10进制
*返回值: 		     10进制数值
*/
uint32_t Read_DecNumber(char *str)
{
	uint32_t value;

	if (! str)
	{
			return 0;
	}
	value = 0;
	while ((*str >= '0') && (*str <= '9'))
	{
			value = value*10 + (*str - '0');
			str++;
	}
	return value;
}

/*!
*String_Conversion：字符串转换为16进制
*返回值: 		    无
*/
void String_Conversion(char *str, uint8_t *src, uint8_t len)
{
 volatile int i,v;
			
 for(i=0; i<len/2; i++)
 {
	sscanf(str+i*2,"%2X",&v);
	src[i]=(uint8_t)v;
}
}

/*!
*Read_Flash_Abp_Data：读取ABP入网参数
*返回值: 		      无
*/
uint8_t Read_Flash_Data(void)
{	
	STMFLASH_Read(SET_ADR_ADDR,(uint16_t*)String_Buffer,2);                 ///ADR
	String_Conversion(String_Buffer, (uint8_t *)&Get_Flash_Datas.ReadSetAdar_Addr, 2);
	if(Get_Flash_Datas.ReadSetAdar_Addr < 1)
	{
		Get_Flash_Datas.LoRaMacSetAdrOnState = false;
	}else
	Get_Flash_Datas.LoRaMacSetAdrOnState = true;
	Get_Flash_Datas.LoRaMacSetAdrOnState = false;
	memset(String_Buffer, 33, 0);
	DEBUG(2,"ADR = %d\r\n",Get_Flash_Datas.LoRaMacSetAdrOnState);

	STMFLASH_Read(LORAMAC_DEFAULT_DATARATE,(uint16_t*)String_Buffer,2);                 ///DataRate：设置默认空速
	String_Conversion(String_Buffer, &Get_Flash_Datas.datarate, 2);
	memset(String_Buffer, 33, 0);

	RF_Send_Data.ADR_Datarate = RF_Send_Data.default_datarate = Get_Flash_Datas.datarate = 5;

	DEBUG(2,"LORAMAC_DEFAULT_DATARATE = %d\r\n",Get_Flash_Datas.datarate);

	STMFLASH_Read(SET_SLEEPT_ADDR,(uint16_t*)String_Buffer,2);                 ///slepptime：设置休眠时间
	String_Conversion(String_Buffer, &Get_Flash_Datas.sleep_times, 2);
	memset(String_Buffer, 33, 0);

	DEBUG(2,"sleep_timeS = %d\r\n",Get_Flash_Datas.sleep_times);

//	LORAMAC_MIN_DATARATE = RF_Send_Data.default_datarate; ///默认datarate = min dataate
//	LORAMAC_MAX_DATARATE = 5;  ///MAX DataRate：固定SF进行通信：最小SF：不开启ADR则不起作用
	Get_Flash_Datas.sync_time = 0;
	Get_Flash_Datas.channels = 0;
	DEBUG(3,"LORAMAC_MIN_DATARATE = %d LORAMAC_MAX_DATARATE = %d\r\n",LORAMAC_MIN_DATARATE,LORAMAC_MAX_DATARATE);

	if( OVER_THE_AIR_ACTIVATION == 0 ) 
	{ 
		uint8_t Devaddr[16] = {0};

		STMFLASH_Read(DEV_ADDR,(uint16_t*)String_Buffer,DEV_ADDR_SIZE/2);         ////DEV

		String_Conversion(String_Buffer, Devaddr, DEV_ADDR_SIZE);   
		memset(String_Buffer, 33, 0);	
		memcpy(DEV,&Devaddr[4],4);

		DEBUG(3,"Devaddr : ");
		for(uint8_t i = 0; i < 8; i++)
		DEBUG(3,"%02x",Devaddr[i]);
		DEBUG(3,"\r\n");

		if (strlen((const char*)DEV)==0
		||strlen((const char*)DEV)>5
		||Get_Flash_Datas.datarate > 5)
		return FAIL;
	}   

	return SUCCESS;
}

int user_app_send( LoRaFrameType_t frametype, uint8_t *buf, int size, int retry)
{
	int sendFrameStatus;

	if(size == 0 || buf == 0){
		return -3;
	}

	LoRaFrameType = frametype;
	if(frametype == UNCONFIRMED){
		sendFrameStatus = LoRaMacSendFrame( RF_Send_Data.AT_PORT, buf, size );
	}else{
		if(retry <= 0){
			retry = 3;
		}
		sendFrameStatus = LoRaMacSendConfirmedFrame( RF_Send_Data.AT_PORT, buf, size, retry );
	}

	switch( sendFrameStatus )
	{
		case 1: // LoRaMac is Busy
		return -1;
		case 2:
		case 3: // LENGTH_PORT_ERROR
		case 4: // MAC_CMD_ERROR
		case 5: // NO_FREE_CHANNEL
		case 6:
			return -2;
		default:
			break;
	}
	return 0;
}

uint32_t app_get_devaddr(void)
{
	return DevAddr;
}


/*
 *	Into_Low_Power:	进入低功耗模式：停机
 *	返回值: 		无
 */
void Into_Low_Power(void)
{	
  Radio.Sleep( );  ///LoRa进休眠状态
    
 /*****************进入停机模式*********************/

  /* Disable the Power Voltage Detector */
  HAL_PWR_DisablePVD( );
	
  SET_BIT( PWR->CR, PWR_CR_CWUF );


  /* Set MCU in ULP (Ultra Low Power) */
  HAL_PWREx_EnableUltraLowPower( );
	
  /*Disable fast wakeUp*/
  HAL_PWREx_DisableFastWakeUp( );

  /* Enter Stop Mode */
  HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
}


void Send_Led(void)
{
	for(uint8_t i = 0; i < 5; i++)
	{
		delay_ms(50);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(50);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
	}
}

void PowerEnable_Led(void)
{
	GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
}

void PowerDisble_Led(void)
{
	GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
}

void Rx_Led(void)
{
  for(uint8_t i = 0; i < 5; i++)
	{
		delay_ms(300);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(300);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
	}
}

/*******************************定义发送数据************************************/

uint8_t AppData[APP_DATA_SIZE];

TimerEvent_t ReportTimer;
volatile bool ReportTimerEvent = false;

TimerEvent_t SleepTimer;
volatile bool SleepTimerEvent = false;

void OnReportTimerEvent( void )
{
	ReportTimerEvent = true;
	RF_Send_Data.Get_sensor = true;
	DEBUG(2,"%s\r\n",__func__);

	HAL_GPIO_TogglePin(LORA_LED,LORA_LED_PIN);

	TimerStop( &ReportTimer );	
	TimerSetValue( &ReportTimer, 500 );
    TimerStart( &ReportTimer );
}


/*
 *	SX1278_Send:	RF发送函数：具有分包发送功能
 *	返回值: 			无
 */
void SX1278_Send(uint8_t LoRaFrameTypes,uint8_t *send_buf)
{	 	      
	if( user_app_send(LoRaFrameTypes, &RF_Send_Data.Send_Buf[RF_Send_Data.Len], RF_Send_Data.TX_Len, 2) == 0 )
	{                               							 
		DEBUG(2,"Wait ACK app_send UpLinkCounter = %d\r\n", LoRaMacGetUpLinkCounter( ));
		ReportTimerEvent = false;
		if(RF_Send_Data.TX_Len == RF_Send_Data.RX_LEN)
		{  ///发送数据结束后判断最后一次发送的数据是否为最后一包是则清除
			memset(AppData, 0, sizeof(AppData));
			RF_Send_Data.RF_Send = true;						
		}
	}
										
		if(LoRapp_SenSor_States.loramac_evt_flag == 1)
		{										
			
			
		}									
}

/*
 *	User_send:	用户发送数据函数
 *	返回值: 		无
 */
void User_send(uint8_t LoRaFrameTypes,uint8_t *Send_Buf)
{
	switch(RF_Send_Data.default_datarate)
	{
		case 0:  //12 -- 51
		case 1:  //11 -- 51
		case 2:  //10 -- 51
		RF_Send_Data.TX_Len = 51;
		SX1278_Send( LoRaFrameTypes, Send_Buf );   
		break;
		case 3:  //9 --- 115
		RF_Send_Data.TX_Len = 115;
		SX1278_Send( LoRaFrameTypes, Send_Buf );   
		break;
		case 4:  //8 --- 222
		case 5:  //7 --- 222
		RF_Send_Data.TX_Len = 222;
		SX1278_Send( LoRaFrameTypes, Send_Buf );   
		break;
		default: break;
	}
}

/*--------------------------------------------------------------------------
                                                     0ooo
                                          ooo0      (   )
                                          (   )      ) /
                                           \ (      (_/
                                            \_)
----------------------------------------------------------------------------*/

