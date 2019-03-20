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
#define APP_TX_DUTYCYCLE                                (100000000)     // 5min
#define APP_TX_DUTYCYCLE_RND                            (100000)   // ms
#define APP_START_DUTYCYCLE                             (10000000)     // 3S

/*!
 * Join requests trials duty cycle.
 */
#define OVER_THE_AIR_ACTIVATION_DUTYCYCLE           10000000 // 10 [s] value in ms

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
Get_Flash_Data Get_Flash_Datas = {false, 0, 2, 5, 0, 5, 0, 0};


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
		DEBUG(2,"win = %d snr = %d rssi = %d size = %d \r\n",RxInfo.win, RxInfo.snr, RxInfo.rssi, RxInfo.size);
		if(flags->Bits.RxData == 1)
		{
			if(mac_callback_g!=NULL)
			{
				mac_callback_g(MAC_STA_RXDONE, &RxInfo);
			  if( RxInfo.size>0 )
			 {
				 DEBUG(2,"RxInfo.buf = ");
				 for( uint8_t i = 0; i < RxInfo.size; i++ )
				 DEBUG(2,"%02x ",RxInfo.buf[i]);
				 DEBUG(2,"\r\n");									
			 }
						
			  if(RxInfo.buf[0] == '_') ///下行更改休眠时间
              {
                  Get_Flash_Datas.sleep_times = Convert16To10(RxInfo.buf[1]);
                  DEBUG(2,"sleep_times :%d\r\n",Get_Flash_Datas.sleep_times);
              }
              else if(RxInfo.buf[0] == 'G') ///重新获取GPS定位信息
              {
                  Set_Gps_Ack.GPS_DONE = false;
                  Set_Gps_Ack.Get_PATION_Again = true;
                  DEBUG(2,"RxInfo.buf[0] = 'G'");
              }
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
			SendDone_Led( );
		}
	}
}

uint8_t Unique_ID[4] = {0};
    
void user_app_init(mac_callback_t mac)
{
	LoRaMacCallbacks.MacEvent = OnMacEvent; ///MAC层数据接口
	LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
	LoRaMacInit( &LoRaMacCallbacks );

	IsNetworkJoined = false;
	
#if( OVER_THE_AIR_ACTIVATION == 0 )  
    
    BoardUniquedeviceID(Unique_ID);
    
	DevAddr  = DEV[3];
	DevAddr |= (DEV[2] << 8);
	DevAddr |= (DEV[1] << 16);
	DevAddr |= (DEV[0] << 24);
	DEBUG(2,"DevAddr : %02x-%02x-%02x-%02x\r\n",DEV[0],DEV[1],DEV[2],DEV[3]);
    	
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
	 memset(String_Buffer, 33, 0);
	 DEBUG(2,"ADR = %d\r\n",Get_Flash_Datas.LoRaMacSetAdrOnState);
	 
	 STMFLASH_Read(LORAMAC_DEFAULT_DATARATE,(uint16_t*)String_Buffer,2);                 ///DataRate：设置默认空速
	 String_Conversion(String_Buffer, &Get_Flash_Datas.datarate, 2);
	 memset(String_Buffer, 33, 0);
	 
	 RF_Send_Data.ADR_Datarate = RF_Send_Data.default_datarate = Get_Flash_Datas.datarate;

	 DEBUG(2,"LORAMAC_DEFAULT_DATARATE = %d\r\n",Get_Flash_Datas.datarate);

	 
	 STMFLASH_Read(SET_SLEEPT_ADDR,(uint16_t*)String_Buffer,2);                 ///slepptime：设置休眠时间
	 String_Conversion(String_Buffer, &Get_Flash_Datas.sleep_times, 2);
	 memset(String_Buffer, 33, 0);
	 
	DEBUG(2,"sleep_timeS = %d\r\n",Get_Flash_Datas.sleep_times);
 
	Get_Flash_Datas.sync_time = 0;
	Get_Flash_Datas.channels = 0;
	DEBUG(2,"LORAMAC_MIN_DATARATE = %d LORAMAC_MAX_DATARATE = %d\r\n",LORAMAC_MIN_DATARATE,LORAMAC_MAX_DATARATE);
 
  if( OVER_THE_AIR_ACTIVATION == 0 ) 
  { 
	uint8_t Devaddr[16] = {0};
		
    STMFLASH_Read(DEV_ADDR,(uint16_t*)String_Buffer,DEV_ADDR_SIZE/2);         ////DEV
    
    String_Conversion(String_Buffer, Devaddr, DEV_ADDR_SIZE);   
    memset(String_Buffer, 33, 0);	
    
	memcpy(DEV,&Devaddr[4],4);
    
    if(Devaddr[3] == 0x22) ///硬件存在GPS
    {
        LoRapp_SenSor_States.Hardware_Exist_GPS = true;
        DEBUG(2,"Hardware_Exist_GPS : \r\n");
    }
		
	DEBUG(2,"Devaddr : ");
	for(uint8_t i = 0; i < 8; i++)
	DEBUG(2,"%02x",Devaddr[i]);
	DEBUG(2,"\r\n");
			
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
        
    BoardDeInitMcu( ); ///关闭时钟线
    
    // Disable the Power Voltage Detector
    HAL_PWR_DisablePVD( );
    
    SET_BIT( PWR->CR, PWR_CR_CWUF );
    /* Set MCU in ULP (Ultra Low Power) */
    HAL_PWREx_EnableUltraLowPower( );
    
    /* Enable the fast wake up from Ultra low power mode */
    HAL_PWREx_EnableFastWakeUp( );

    /*****************进入停机模式*********************/
    /* Enter Stop Mode */
    __HAL_PWR_CLEAR_FLAG( PWR_FLAG_WU );
    HAL_PWR_EnterSTOPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI );
}

/*
 *	app_loramacjoinreq:	OTAA入网申请
 *	返回值: 		    		无
 */
void app_loramacjoinreq(void)
{
	while( ( IsNetworkJoined == false ) )
	{
		if( JoinReq_flag == true )
		{
			JoinReq_flag = false;
			
			DEBUG(2,"LoRaMacJoinReq \r\n");
		 int  sendFrameStatus = LoRaMacJoinReq( DevEui, AppEui, AppKey );
			DEBUG(2,"sendFrameStatus = %d\r\n",sendFrameStatus);
			switch( sendFrameStatus )
			{
				case 1: // BUSY
						break;
				case 0: // OK
				case 2: // NO_NETWORK_JOINED
				case 3: // LENGTH_PORT_ERROR
				case 4: // MAC_CMD_ERROR
				case 6: // DEVICE_OFF
				default:
					// Relaunch timer for next trial                
					TimerStart( &JoinReqTimer );                  
					break;
			}
		}
	}   
}


void SendDone_Led(void)
{
	for(uint8_t i = 0; i < 5; i++)
	{
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(50);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
		delay_ms(50);
	}
}

void Rx_Led(void)
{
	for(uint8_t i = 0; i < 5; i++)
	{
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(50);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
		delay_ms(50);
	}
}

void Error_Led(void)
{
	for(uint8_t i = 0; i < 3; i++)
	{
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_SET);
		delay_ms(1000);
		GpioWrite(LORA_LED,LORA_LED_PIN,GPIO_PIN_RESET);
		delay_ms(200);
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
/*******************************定义发送数据************************************/

TimerEvent_t ReportTimer;
volatile bool ReportTimerEvent = false;

TimerEvent_t SleepTimer;
volatile bool SleepTimerEvent = false;

void OnReportTimerEvent( void )
{
	ReportTimerEvent = true;
	RF_Send_Data.Get_sensor = true;
	DEBUG(2,"%s\r\n",__func__);

	TimerStop( &ReportTimer );	
}

extern uint32_t sensor_time;

/*
 *	SX1278_Send:	RF发送函数：具有分包发送功能
 *	返回值: 			无
 */
void SX1278_Send(uint8_t LoRaFrameTypes,uint8_t *send_buf)
{	 	      
	switch(sys_sta)
	{
		case SYS_STA_IDLE:
		if(ReportTimerEvent == true)
		{ 
			sys_sta = SYS_STA_TX; 
			if(RF_Send_Data.Len == 0 && RF_Send_Data.Send_again == false) //当前数据为空，同时不是发送失败下获取数据，重新获取数据发送
			{				
				memcpy(RF_Send_Data.RF_BUF, send_buf, RF_Send_Data.RX_LEN);   
			}	
			DEBUG(2,"START RF_Send_Data.RX_LEN = %d\r\n",RF_Send_Data.RX_LEN);					
		}
		 break;
		 case SYS_STA_TX:		
			 while( RF_Send_Data.RX_LEN > 0)
			{
				if(RF_Send_Data.RX_LEN < RF_Send_Data.TX_Len)
				{
					RF_Send_Data.TX_Len = RF_Send_Data.RX_LEN;			
				}
						
				if(ReportTimerEvent == true)
				{
					ReportTimerEvent = false;
					
					LoRaCad.Cad_Mode = true;
					while(LoRaCad.Cad_Mode || LoRaCad.Cad_Detect)	//10ms超时处理
					LoRa_Cad_Mode( );		
									
					Channel = Get_max(5,LoRaCad.Rssi); ///获取信道ID
					DEBUG(3,"Channel_send = %d\r\n",Channel);

					if( user_app_send(LoRaFrameTypes, &RF_Send_Data.Send_Buf[RF_Send_Data.Len], RF_Send_Data.TX_Len,3) == 0 )
					{                               							 
						DEBUG(2,"Wait ACK app_send UpLinkCounter = %d\r\n", LoRaMacGetUpLinkCounter( ));
						if(RF_Send_Data.TX_Len == RF_Send_Data.RX_LEN)
						{  ///发送数据结束后判断最后一次发送的数据是否为最后一包是则清除
							memset(RF_Send_Data.Send_Buf, 0, RF_Send_Data.RX_LEN);
							RF_Send_Data.RF_Send = true;						
						}
					}
					else 
					{ ///发送失败随机超时发送：发送失败原因datarate发生改变导致数据发送失败
						TimerStop( &ReportTimer );
						TimerSetValue( &ReportTimer, APP_TX_DUTYCYCLE_RND + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
						TimerStart( &ReportTimer );
						DEBUG(2,"app_send again\r\n");
						
						RF_Send_Data.Error_count ++;
						RF_Send_Data.Send_again = true;
						RF_Send_Data.default_datarate = RF_Send_Data.ADR_Datarate; ///更新空速
						break;
					}
				}
										
				if(LoRapp_SenSor_States.loramac_evt_flag == 1)
				{										
					if(RF_Send_Data.RX_LEN > RF_Send_Data.TX_Len)
					{
						RF_Send_Data.Len += RF_Send_Data.TX_Len;   ///数组下标
						RF_Send_Data.RX_LEN -= RF_Send_Data.TX_Len; ///当前待发送Len 
					
						if(RF_Send_Data.RX_LEN<=RF_Send_Data.TX_Len)
						{
							RF_Send_Data.TX_Len = RF_Send_Data.RX_LEN;
						}
						
						TimerStop( &ReportTimer );
						TimerSetValue( &ReportTimer, APP_TX_DUTYCYCLE_RND + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
						TimerStart( &ReportTimer );
					}
			
					if(RF_Send_Data.RF_Send == true)						
					{
						RF_Send_Data.RF_Send = false;
						RF_Send_Data.Send_again = false;
                        HAL_Delay(1500);
						
						memset(send_buf, 0, RF_Send_Data.RX_LEN);
						memset(RF_Send_Data.RF_BUF, 0, RF_Send_Data.RX_LEN);

						RF_Send_Data.TX_Len = RF_Send_Data.Len = 0;
						RF_Send_Data.RX_LEN = 0;
						sys_sta = SYS_STA_IDLE;
																		
						if(Set_Gps_Ack.GPS_DONE) ///GPS定位数据发送完成,切换为获取Sensor数据发送，产生一定时延,不直接发送
						{																			
							RF_Send_Data.Sleep_times = Get_Flash_Datas.sleep_times;
							LoRapp_SenSor_States.WKUP_State = true;
							DEBUG(3, "sensor_time = %d Sleep_times = %d\r\n",sensor_time,RF_Send_Data.Sleep_times);
							///休眠时间大于5分钟，则会出现溢出，分为两种休眠模式						
							RtcInitialized = false; ///重新初始化RTC
							RTC_Init();	
										
							sensor_time = HAL_GetTick( ) - system_time -1500;
							sensor_time /= 1000;
							if(Rs485Tag_UNKNOW) 
							{
							  DEBUG(3,"sensor_time = %d\r\n",sensor_time);
							  if(sensor_time>=7)
							  sensor_time -= 6;
							}
							/**********************补偿485休眠采集时间******************/
							
							RF_Send_Data.Sleep_times *= 60;
							RF_Send_Data.Sleep_times -= sensor_time;
							
							if(RF_Send_Data.Sleep_times<10)
								RF_Send_Data.Sleep_times = 30;								
						}											
						break;	
					}
				}						
			}
		 break;
		default : 
		 break;
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

/*
 *	User_Send_Api:	发送数据接口,同时具有分包发送功能
 *	返回值: 		无
 */
void User_Send_Api( void )
{	
    if(RF_Send_Data.Get_sensor && Set_Gps_Ack.GPS_DONE) ///获取485、光照、空气温湿度传感器数据  
    {
        RF_Send_Data.Get_sensor = false;
         
        PowerEnable_Led(  ); 
        SamplingData(  );											//获取传感器数据		
        PowerDisble_Led(  ); 
                 
        /***************************传感器数据存入发送缓存区*********************************/		
        memset(RF_Send_Data.Send_Buf,0,SAMPLE_SIZE);

        RF_Send_Data.Send_Buf[0] = samples.structver;
        RF_Send_Data.Send_Buf[1] = samples.count;
        for(uint8_t i = 0; i <= samples.socket_id; i++)
        {
            RF_Send_Data.Send_Buf[2+i] = samples.sockets[i];
            DEBUG(2,"%02x",RF_Send_Data.Send_Buf[2+i]);
        }
        DEBUG(3,"socket_id = %d\r\n",samples.socket_id);

        uint8_t j = 0;
        for(uint8_t i = 0; i < samples.count; i++, j+=2)
        {
            RF_Send_Data.Send_Buf[samples.socket_id+3+j] = (samples.fields[i] >> 8)&0xff;
            RF_Send_Data.Send_Buf[samples.socket_id+4+j] = (samples.fields[i])&0xff;

            DEBUG(3,"%02x",RF_Send_Data.Send_Buf[samples.socket_id+3+j]);
            DEBUG(3,"%02x",RF_Send_Data.Send_Buf[samples.socket_id+4+j]);
        }
         RF_Send_Data.RX_LEN = SAMPLE_SIZE;  //MAC+PHY=56  MAC = 13  SAMPLE_SIZE
        do
        {								
            User_send(UNCONFIRMED, RF_Send_Data.Send_Buf);
        }while(!LoRapp_SenSor_States.loramac_evt_flag);
        __disable_irq();
        LoRapp_SenSor_States.loramac_evt_flag = 0;
        __enable_irq();

        SetRtcAlarm(RF_Send_Data.Sleep_times-3);///4S误差	
        Into_Low_Power( );	
    }
}

/*
*软件复位
*/
extern void __set_FAULTMASK(uint32_t faultMask);
void SystemReset(void)
{
	DEBUG(2,"hello systemrest\r\n");
	HAL_NVIC_SystemReset( );
}

/*
*设置GPS再次发送时间：计算不同休眠模式每天所发送的包数，达到数据量则再次上报位置
*参数：								无
*返回：								无
*/
void GpsSendAgainTime(void)
{
	if(Get_Flash_Datas.sleep_times == 5)
	{
		LoRapp_SenSor_States.Work_Time = DAY_MIN_5;
	}
	else if(Get_Flash_Datas.sleep_times == 10)
	{
		LoRapp_SenSor_States.Work_Time = DAY_MIN_10;
	}
	else if(Get_Flash_Datas.sleep_times == 15)
	{
		LoRapp_SenSor_States.Work_Time = DAY_MIN_15;
	}
	else if(Get_Flash_Datas.sleep_times == 30)
	{
		LoRapp_SenSor_States.Work_Time = DAY_MIN_30;
	}
}
