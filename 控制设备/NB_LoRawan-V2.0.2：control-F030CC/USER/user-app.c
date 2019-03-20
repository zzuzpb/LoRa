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
#include "LoRa-cad.h"
#include "LoRaMac.h"
#include "LoRaMac-api-v3.h"


#define APP_DATA_SIZE                                   (43)
#define APP_TX_DUTYCYCLE                                (2000000)     // 5min
#define APP_TX_DUTYCYCLE_RND                            (100000)   // ms


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

static uint8_t DEV[] = {
    0x00, 0x00, 0x00, 0x14
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
LoRapp_State LoRapp_SenSor_States = {false, false, 0};

/************************设置RF分包发送参数************************/
RF_Send RF_Send_Data = {0, 0, 0, 0, {0}, 0, 5, false, false, true}; 

/************************设置CAD模式参数**************************/
LoRaCad_t LoRaCad = {true, false, false, false, 0, {0}, 0, 0, 0, 0, 0, {0}, {0}};

/************************读取flash数据设置射频参数**************************/
Get_Flash_Data Get_Flash_Datas = {false, 0, 2, 285, 0, 2, 0, 0}; //285



/****************************OTAA参数******************************
*********【1】read flash or get buy AT commed.
*********/

uint8_t AppEui[16] = {0};  
uint8_t AppKey[32] = {0};


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
    switch( info->Status ){
        case LORAMAC_EVENT_INFO_STATUS_OK:
            break;
        case LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT:;
            break;
        case LORAMAC_EVENT_INFO_STATUS_ERROR:
            break;
        default:
            break;
    }

    if( flags->Bits.JoinAccept == 1 ){       
        DEBUG(2,"join done\r\n");
        TimerStop( &JoinReqTimer );
        IsNetworkJoined = true;
				Rx_Led( );
        mac_callback_g(MAC_STA_CMD_JOINACCEPT, NULL);
    }  
  
    if( info->TxAckReceived == true ){ /// McpsConfirm.AckReceived = true;接收回调函数
        if(mac_callback_g!=NULL){
            mac_callback_g(MAC_STA_ACK_RECEIVED, &RxInfo); ///相当于调用app_lm_cb函数
						LoRapp_SenSor_States.Ack_Recived = true;    
						DEBUG(2,"ACK Received\r\n");
        }
		}else if((flags->Bits.Rx != 1) && (flags->Bits.JoinAccept != 1) && (LoRaFrameType == CONFIRMED))
			{
        DEBUG(2,"=====NO ACK REPLY=====\r\n");     
        mac_callback_g(MAC_STA_ACK_UNRECEIVED, NULL);        
			}
	
       if( flags->Bits.Rx == 1 ){  ///接收到数据，数据信息保存RxData: 网关应答数据默认不打印接收信息
            RxInfo.size = info->RxBufferSize;
            memcpy(RxInfo.buf, info->RxBuffer, RxInfo.size);
            RxInfo.rssi = info->RxRssi;
            RxInfo.snr = info->RxSnr;
            RxInfo.win = flags->Bits.RxSlot+1;
            RxInfo.port = info->RxPort;
           
						DEBUG(2,"win = %d snr = %d rssi = %d size = %d \r\n",RxInfo.win, RxInfo.snr, RxInfo.rssi, RxInfo.size);
//						Rx_Led( );	
            if(flags->Bits.RxData == 1){
                if(mac_callback_g!=NULL){
                    mac_callback_g(MAC_STA_RXDONE, &RxInfo);
									if( RxInfo.size>0 )
									{
                    DEBUG(2,"RxInfo.buf = ");
                    for( uint8_t i = 0; i < RxInfo.size; i++ )
                    DEBUG(2,"%02x ",RxInfo.buf[i]);
                    DEBUG(2,"\r\n");
									}
                }
            }           				
        }
     
    if( flags->Bits.Tx == 1 )
    {
        if(mac_callback_g!=NULL){
            mac_callback_g(MAC_STA_TXDONE, NULL);
          if( flags->Bits.JoinAccept == 1 ) ///如果是OTAA入网请求应答，则直接发送数据不需要等待，否则等待
          {
            LoRapp_SenSor_States.loramac_evt_flag = 0;
          }else
          LoRapp_SenSor_States.loramac_evt_flag = 1;
              
          DEBUG(2,"Done\r\n");
					Send_Led( );
        }
    }
}
void user_app_init(mac_callback_t mac)
{
    LoRaMacCallbacks.MacEvent = OnMacEvent; ///MAC层数据接口
    LoRaMacCallbacks.GetBatteryLevel = BoardGetBatteryLevel;
    LoRaMacInit( &LoRaMacCallbacks );
    
    IsNetworkJoined = false;
    
    // Random seed initialization
   // srand1( RAND_SEED );
    // Choose a random device address
    // NwkID = 0
    // NwkAddr rand [0, 33554431]
   // DevAddr = randr( 0, 0x01FFFFFF );
  //  DevAddr = 123088097;  ///使用服务器设置参数
#if( OVER_THE_AIR_ACTIVATION == 0 )  
    
    DevAddr  = DEV[3];
    DevAddr |= (DEV[2] << 8);
    DevAddr |= (DEV[1] << 16);
    DevAddr |= (DEV[0] << 24);
    DEBUG(2,"DevAddr = %d\r\n",DevAddr);
    
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
  
    LoRaMacSetAdrOn( true );
    LoRaMacTestSetDutyCycleOn(false);
    
    mac_callback_g = mac;

}

int user_app_send( LoRaFrameType_t frametype, uint8_t *buf, int size, int retry)
{
	int sendFrameStatus;

	if(size == 0 || buf == 0){
		return -3;
	}

	LoRaFrameType = frametype;
	if(frametype == UNCONFIRMED){
		sendFrameStatus = LoRaMacSendFrame( RF_Send_Data.AT_MODULE_PORT, buf, size );
	}else{
		if(retry <= 0){
			retry = 3;
		}
		sendFrameStatus = LoRaMacSendConfirmedFrame( RF_Send_Data.AT_MODULE_PORT, buf, size, retry );
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
 *	返回值: 				无
 */
void Into_Low_Power(void)
{
//	Radio.Sleep( );  ///LoRa进休眠状态

  BoardDeInitMcu(); ///关闭时钟线
    
 /*****************进入停机模式*********************/
	
	/* Enter Stop Mode */
//	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
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
			Send_Led( ); 
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

void Send_Led(void)
{
	for(uint8_t i = 0; i < 3; i++)
	{
		delay_ms(100);
		GpioWrite(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
		delay_ms(100);
		GpioWrite(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
	}
}

void Rx_Led(void)
{
  for(uint8_t i = 0; i < 3; i++)
	{
		delay_ms(400);
		GpioWrite(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);
		delay_ms(400);
		GpioWrite(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);
	}
}


/*******************************定义发送数据************************************/

sys_sta_t sys_sta = SYS_STA_IDLE;

uint8_t AppData[APP_DATA_SIZE];

volatile bool ReportTimerEvent = false;

TimerEvent_t ReportTimer;

extern void SetRtcAlarm(uint16_t time);

void OnReportTimerEvent( void )
{
    ReportTimerEvent = true;
	  DEBUG(2,"%s\r\n",__func__);
}

/*
 *	SX1278_Send:	RF发送函数：具有分包发送功能
 *	返回值: 			无
 */
void SX1278_Send(uint8_t *send_buf)
{	 	      
	switch(sys_sta)
	{
		case SYS_STA_IDLE:
		if(ReportTimerEvent == true)
		{ 
			sys_sta = SYS_STA_TX; 
			if(RF_Send_Data.Len == 0 && RF_Send_Data.Send_again == false) //当前数据为空，同时不是发送失败下获取数据，重新获取数据发送
			{							
				RF_Send_Data.RX_LEN = 2;///sizeof *send_buf
				memcpy(RF_Send_Data.RF_BUF, send_buf, RF_Send_Data.RX_LEN);   //MAC+PHY=56  MAC = 13  
				RF_Send_Data.Error_count = 0;			
			}	
			DEBUG(2,"START len = %d ReportTimerEvent = %d\r\n",RF_Send_Data.RX_LEN,ReportTimerEvent);					
		}
		 break;
		 case SYS_STA_TX:		
						 
			 if( RF_Send_Data.RX_LEN > 0)
			{
				if(RF_Send_Data.Estab_Communt_State==false)
				DEBUG(4,"line = %d\r\n",__LINE__);	

				if(RF_Send_Data.RX_LEN < RF_Send_Data.TX_Len)
				{
					DEBUG(4,"line = %d ReportTimerEvent = %d\r\n",__LINE__,ReportTimerEvent);	
					RF_Send_Data.TX_Len = RF_Send_Data.RX_LEN;
				}				

				if(ReportTimerEvent == true)
				{
					LoRaCad.Cad_Mode = true;
				
					DEBUG(2,"line = %d\r\n",__LINE__);	
					
					while(LoRaCad.Cad_Mode || LoRaCad.Cad_Detect)	//10ms超时处理
					LoRa_Cad_Mode( );		
									
					Channel = Get_max(3,LoRaCad.Rssi); ///获取信道ID

					if( user_app_send(CONFIRMED, &RF_Send_Data.RF_BUF[RF_Send_Data.Len], RF_Send_Data.TX_Len, 1) == 0 )
					{                               							 
//						printf("Wait ACK app_send again count = %d len = %d\r\n", RF_Send_Data.Error_count,RF_Send_Data.Len);
						DEBUG(2,"Wait ACK app_send UpLinkCounter = %d\r\n", LoRaMacGetUpLinkCounter( ));
						ReportTimerEvent = false;
						if(RF_Send_Data.TX_Len == RF_Send_Data.RX_LEN)
						{  ///发送数据结束后判断最后一次发送的数据是否为最后一包是则清除
							memset(AppData, 0, sizeof(AppData));
							RF_Send_Data.RF_Send = true;						
						}
					}
					else 
					{ ///发送失败随机超时发送：发送失败原因datarate发生改变导致数据发送失败
						ReportTimerEvent = false;
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
				{///接收应答完成,设置RTC闹钟事件下次随机发送：握手时才会在此清空
					if( RF_Send_Data.Estab_Communt_State)
					{
						__disable_irq();
						LoRapp_SenSor_States.loramac_evt_flag = 0;
						__enable_irq();
					}
										
					if(RF_Send_Data.RX_LEN > RF_Send_Data.TX_Len)
					{
						RF_Send_Data.Len += RF_Send_Data.TX_Len;   ///数组下标
						RF_Send_Data.RX_LEN -= RF_Send_Data.TX_Len; ///当前待发送Len 
					
						if(RF_Send_Data.RX_LEN<=RF_Send_Data.TX_Len)
						{
							RF_Send_Data.TX_Len = RF_Send_Data.RX_LEN;
						}
						
						ReportTimerEvent = false;
						
						TimerStop( &ReportTimer );
						TimerSetValue( &ReportTimer, APP_TX_DUTYCYCLE_RND + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
						TimerStart( &ReportTimer );
					}
			
					if(RF_Send_Data.RF_Send == true)						
					{
						RF_Send_Data.RF_Send = false;
						RF_Send_Data.Send_again = false;
						RF_Send_Data.TX_Len = RF_Send_Data.Len = 0;
						RF_Send_Data.RX_LEN = 0;
						sys_sta = SYS_STA_IDLE;

						ReportTimerEvent = false;
						RF_Send_Data.Estab_Communt_State = false;
						DEBUG(2, "send_done %d\r\n",LoRapp_SenSor_States.loramac_evt_flag);
//            TimerStop( &ReportTimer );
//            TimerSetValue( &ReportTimer, APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND ) );
//            TimerStart( &ReportTimer );						

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
void User_send(uint8_t *Send_Buf)
{
	switch(RF_Send_Data.default_datarate)
	{
		case 0:  //12 -- 51
		case 1:  //11 -- 51
		case 2:  //10 -- 51
		RF_Send_Data.TX_Len = 51;
		SX1278_Send( Send_Buf );   
		break;
		case 3:  //9 --- 115
		RF_Send_Data.TX_Len = 115;
		SX1278_Send( Send_Buf );   
		break;
		case 4:  //8 --- 222
		case 5:  //7 --- 222
		RF_Send_Data.TX_Len = 222;
		SX1278_Send( Send_Buf );   
		break;
		default: break;
	}
}


/*
 *	App_Estab_Communt:	上电通信前发送一包数据帧与网关建立通信握手，同时具有分包发送功能
 *	返回值: 		    		无
 */
void App_Estab_Communt( void )
{
	 if( (IsNetworkJoined || ( OVER_THE_AIR_ACTIVATION != 1 )) && RF_Send_Data.Estab_Communt_State )
	 {    
		 RF_Send_Data.Send_Buf = "ACK";
		 User_send(RF_Send_Data.Send_Buf);
	 }
	
}

/*
 *	Receive_ConTrol_Data:	接收数据处理
 *	返回值: 		        无
 */
void Receive_ConTrol_Data(void)
{	

 if( Control_States.Auto_Mode == false )///手动模式
 {
	///发送对应控制指令控制设备：手动模式下，全部关闭
	 memset(RF_Send_Data.Send_Buf, 0, strlen((char *)RF_Send_Data.Send_Buf));

	 RF_Send_Data.Send_Buf[0] = 01;
	 RF_Send_Data.Send_Buf[1] = 00;
	 RF_Send_Data.Send_Buf[2] = 04;
	 RF_Send_Data.Send_Buf[3] = 00;
	 Control_Relay(RF_Send_Data.Send_Buf);
		 
	 ///发送：当前处于手动模式：原数据返回
	 TimerStop( &ReportTimer );
	 TimerSetValue( &ReportTimer,100000 );
	 TimerStart( &ReportTimer );
	 DEBUG(2,"TimerStart\r\n");
	 while(ReportTimerEvent != true);		
	 do
	 {
		 ///发送：执行完成：原数据返回
		 User_send(RF_Send_Data.Send_Buf);	
	 }while(LoRapp_SenSor_States.loramac_evt_flag==0);
		
		/*******************清除发送完成标志*********************/
		__disable_irq();
		LoRapp_SenSor_States.loramac_evt_flag = 0;
		__enable_irq();
		 
 }
	else
	{
		if(RxInfo.size > 0)
		{			
			uint8_t data[4] = {0};
			
	  	RxInfo.size = 0;

			///异常原因
			memcpy(data, RxInfo.buf, 2); ///USART5接收缓存copy
			
			Control_Relay(data);
			memset(RxInfo.buf, 0, 2);
		
			TimerStop( &ReportTimer );
			TimerSetValue( &ReportTimer,100000 );
			TimerStart( &ReportTimer );
	    DEBUG(2,"TimerStart\r\n");
			while(ReportTimerEvent != true);		
			do
			{
				///发送：执行完成：原数据返回
				User_send(&Control_States.send_buf[2]);	
			}
			while(LoRapp_SenSor_States.loramac_evt_flag==0);
			
		  DEBUG(2,"TimerStart %d %d\r\n",Control_States.send_buf[2],Control_States.send_buf[3]);
			
			/*******************清除发送完成标志*********************/
			__disable_irq();
			LoRapp_SenSor_States.loramac_evt_flag = 0;
			__enable_irq();
		}	
	}
}
