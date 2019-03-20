
	
1：硬件IO接口图：

								stm32L072CBT6
SX1278						 _ _ _ _ _ _ _ _ _ _ _ _ _ _ 
SPI     NSS	  --------	PA4 |					  		|
		SCK	  --------	PA5 |    				  		|
		MISO  --------	PA6 |					 		|
		MOSI  --------	PA7 |					  		|
						    |					  		|
EXti	DIO0  --------	PB1 |                    		|
		DIO1  --------	PB2 |					  		|
		DIO2  --------	PB10|					  		|
		DIO3  --------	PB11|					 		|
		DIO4  --------	NC	|					  		|
		DIO5  --------	NC	|					 		|
							|					  		|
CPIO	RESET --------	PB0 |					  		|
		LoRa_Power ---  PB12|					 		|
							|					 		|
							|					 		|
GPS	(UART2)					|					 		|	
		TX	  --------  PA2	|					  		|	
		RX	  --------  PA3	|					  		|	
GPS_Power_ON  --------  PB7	|					  		|									
							|					  		|
485	(UART5)					|					 		|	
		485_TX	------	PB3	|					  		|	
		485_RX	------	PB4	|					  		|	
		485_DE	------	PB5	|					  		|
		12V_ON	------  PA8	|					  		|	
							|					  		|
							|					 		|	
DEBUG(UART1)				|					  		|
		TX   ---------	PA9	|					  		|
		RX	 ---------  PA10|					  		|
							|					  		|
I2C							|					  		|
		I2C2_SDA ----- PB14	|					  		|
		I2C2_SCL ----- PB13	|					  		|
							|					  		|
电源管理使能  -------- PB9	|					  		|
							|					  		|
							|					  		|
							|					  		|
							|_ _ _ _ _ _ _ _ _ _ _ _ _ _|	




1: extern volatile uint8_t datarate注释掉了默认Datarate


3：备份RTC WKUP
    /**Enable the Alarm A 
    */
	
  RTC_AlarmTypeDef sAlarm;
	
  sAlarm.AlarmTime.Hours = 11;
  sAlarm.AlarmTime.Minutes = 20;
  sAlarm.AlarmTime.Seconds = 30;
  sAlarm.AlarmTime.SubSeconds = 0;
  sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
  sAlarm.AlarmMask = RTC_ALARMMASK_ALL;
  sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
  sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
  sAlarm.AlarmDateWeekDay = 1;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }


4: 待添加时隙划分、参数配置

未完成：
1：CAD侦听底层协议更改
2：FLASH读写操作
3：SX1278外设IO配置对应硬件改版

待测试：
1：RTC低功耗唤醒功能
2：空中唤醒深度测试



void HAL_MspInit(void)：缺失配置RTC导致RTC待机唤醒出现乱码


right:
maxN = 123 payloadSize = 115 lenN = 115  fOptsLen = 0
maxN = 123 payloadSize = 5 lenN = 5  fOptsLen = 0
error:
maxN = 59 payloadSize = 117 lenN = 115  fOptsLen = 2
????:?ValidatePayloadLength()??????ADR datarate??????????????,??????

3.8.6:??:LORA_MAX_NB_CHANNELS???8
ScheduleTx( )-----> return SendFrameOnChannel( Channels[Channel] );  
Channel = LORA_MAX_NB_CHANNELS;
Channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];----->SetNextChannel( TimerTime_t* time )
??????? SetNextChannel( TimerTime_t* time )?????:Channel = Get_Flash_Datas.channels; ///????ID????????

3.8.7:?????????:
????????iqInverted,?????????iqInverted,?????iqInverted,?????????iqInverted?

??RX1??????:????,?? OnRadioRxDone()?????case FRAME_TYPE_DATA_CONFIRMED_UP:
NodeAckRequested = false;
OnRxWindow1TimerEvent(  );

LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )

// Send now
Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

Cad_State = CadDetect

关于RTC唤醒参数：
1：alarmStructure.AlarmDateWeekDay = alarmTimer.CalendarDate.Date;   ///当 alarmStructure.AlarmDateWeekDay = 1时 alarmStructure.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY;否则唤醒失败
	alarmStructure.AlarmMask = RTC_ALARMMASK_NONE; 		///唤醒标志位


ReceiveDelay1
RECEIVE_DELAY1

RTC MS级别计数存在问题：不接收网关下行数据，接收数据时间控制不好导致；因此暂时只采用us级别进行计数，后期有足够时间重构链表


38. 关于AT指令设置POWER、DATARATE问题如下所示：

3.8.1：设置DATARATE范围：
  if( ValueInRange( datarate, LORAMAC_MIN_DATARATE, LORAMAC_MAX_DATARATE ) == false )
  LORAMAC_MIN_DATARATE：最小空速即最大SF
  LORAMAC_MAX_DATARATE：最大空速即最小SF
  
  LORAMAC_DEFAULT_DATARATE：默认运行空速-----当上行确认应答数据时，如果没接收到应答数据则DATARATE自动更改
  关于SF设置问题：LORAMAC_MAX_DATARATE 一般设置为 5 最低SF；采用固定SF时 LORAMAC_MIN_DATARATE = LORAMAC_DEFAULT_DATARATE则按规定的最高SF工作
  当使能ADR时则会在最低---最高SF 中选择最佳SF工作
  LORAMAC_DEFAULT_DATARATE则默认设置为最高SF；
  LORAMAC_DEFAULT_DATARATE写入地址为：0x0801F398
 
3.8.2：设置POWER范围：
  if( ValueInRange( txPower, LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) == false )
  LORAMAC_MAX_TX_POWER：最大POWER范围
  LORAMAC_MIN_TX_POWER：最小POWER范围
  LORAMAC_DEFAULT_TX_POWER：默认使用的POWER
 
  设置power:  
  SendFrameOnChannel ---  ChannelsTxPower = LimitTxPower( ChannelsTxPower ); txPower = TxPowers[ChannelsTxPower];
  static int8_t ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;

3.8.3: RXWIN:
MaxRxWindow = MAX_RX_WINDOW;

3.8.4：重复发包次数：
 retry：if(retry<=0); retry=3;
 
3.8.5: 关闭接收窗口：OnRadioTxDone( void )发送完成后打开接收窗口，关闭窗口必须在该发送函数操作：TimerStop( &RxWindowTimer1 ); TimerStop( &RxWindowTimer2 );
	   则可以关闭相应的窗口

分包发送数据后，当接收到ADR时，发送失败原因：该函数中datarate更改，导致数据包长度错误
static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Get the maximum payload length
    if( RepeaterSupport == true )
    {
        maxN = MaxPayloadOfDatarateRepeater[datarate];
    }
    else
    {
        maxN = MaxPayloadOfDatarate[datarate];
    }

    // Calculate the resulting payload size
    payloadSize = ( lenN + fOptsLen );
		printf("lenN = %d  fOptsLen = %d\r\n",lenN, fOptsLen);

    // Validation of the application payload size
    if( ( payloadSize <= maxN ) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) )
    {
        return true;
    }
    return false;
}	

right:
maxN = 123 payloadSize = 115 lenN = 115  fOptsLen = 0
maxN = 123 payloadSize = 5 lenN = 5  fOptsLen = 0
error:
maxN = 59 payloadSize = 117 lenN = 115  fOptsLen = 2
更改方式：将ValidatePayloadLength()函数接收到的ADR datarate返回到发送函数中再次发送数据，则解决该问题

3.8.6：频段：LORA_MAX_NB_CHANNELS设置为8
ScheduleTx( )-----> return SendFrameOnChannel( Channels[Channel] );  
Channel = LORA_MAX_NB_CHANNELS;
Channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];----->SetNextChannel( TimerTime_t* time )
使用固定频段在 SetNextChannel( TimerTime_t* time )函数中添加：Channel = Get_Flash_Datas.channels; ///固定信道ID则可以定频点通信

3.8.7：关于节点间监听功能：
如果发射放使能了iqInverted,那么接收方也要使能iqInverted，发射方失能iqInverted,那么接收方也要失能iqInverted。

开启RX1窗口监听模式：如下所示，同时OnRadioRxDone()函数中添加case FRAME_TYPE_DATA_CONFIRMED_UP:
NodeAckRequested = false;
OnRxWindow1TimerEvent(  );

LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )

// Send now
Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

Cad_State = CadDetect

BUG列表：
1：MCU采用4MHZ以下工作频率存在问题：

1.1：系统时钟最小计数时间为100us，定时过低则会导致系统定时器出错，MCU不工作

1.2：关于使用系统定时器计算问题，以1MS作为最小单位即可，I2C使用的US延时可以采用nop

1.3：OnMacStateCheckTimerEvent触发失败原因

OnMacStateCheckTimerEvent

// Trig OnMacCheckTimerEvent call as soon as possible
TimerSetValue( &MacStateCheckTimer, 10 );  //10ms
TimerStart( &MacStateCheckTimer );
MAC层定时检查时间更改为10MS，原因系统时钟为2MHZ，1MS定时检查则会导致接收数据时不触发MAC层检查中断

1.4：RTC存在严重不稳定性

解决1系列出现的问题，需要将MCU clock设置为4MHZ以上

低功耗问题：
控制设备不采用MCU休眠模式：CAD+多余IO关闭处理+lower-power run mode


LC( 6 ) + LC( 7 ) + LC( 8 );


SetNextChannel
		|
		|
		|
		|
  Channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )]; ///发送前更改信道
  
  信道按设计规定信道ID走：( LC( 1 ) + LC( 2 ) + LC( 3 ) );增加了信道也应保持该模式1 2 3不变此
 Radio.SetTxConfig


存在问题：
1：开启cad会出现异常错误：timer定时频率过高导致不触发，MCU处理不够

2：spi更改为DMA模式，降低MCU损耗

3: TimerSetValue( &MacStateCheckTimer, 1 ); //1ms会出现接收问题：MCU时钟过低，处理数据需要相应多的时间，等待一段时间则完成，或者更改为10ms

4：控制设备需要添加异常保护机制：防止长时间没通信，断开连接


SX1276SetTxConfig

5.RTC时间分析：

#define RTC_ALARM_TIME_BASE                             122.07  //122.07 1000 //

RtcHandle.Init.AsynchPrediv = 1;
RtcHandle.Init.SynchPrediv = 1; 
进行1S分频运算: 32.768/(AsynchPrediv+1)/(SynchPrediv+1) = 8.192  
				1/8.192 = 0.12207ms = 122.07us
RTC分频较低，当MCU运行在MSI模式时RTC容易出现问题，因此进行分频更改如下：同样实现1S计数。


#define RTC_ALARM_TIME_BASE                             244.14 /// 122.07  //122.07 1000000 //

RtcHandle.Init.AsynchPrediv = 3; //3; 127
RtcHandle.Init.SynchPrediv = 1; ///3; 255

进行1S分频运算: 32.768/(AsynchPrediv+1)/(SynchPrediv+1) = 4.096 
				1/4.096 = 0.24414ms = 244.14us
				
6：数据格式：下发开启水阀需要两帧数据：一帧数据唤醒: RF_Send_Data.Send_Buf = "RADIO"; 0x52 0x41 0x44 0x49 0x4f
一帧有效数据: open: 0x01 0xb1   close : 0x02 0xb1
									   
   需要区分好唤醒数据帧、有效数据帧，存在唤醒数据帧数据接收到并且解析完成，则退出休眠进入接收必须确保在有效数据帧时才上报数据
   
   
 7：数据流程：
 接收到唤醒数据帧+有效数据帧 (应答服务器，如服务器在规定时间内没接收到应答则重复唤醒)---- > 每分钟上报一次数据 ----- > 接收到停止数据帧 ----- > 进入CAD mode
   
524144494f

出现 HardFault_Handler error 问题：一般都是数组越界或者指针没分配空间出现野指针 
#include "rs485.h"
#pragma pack(1)
typedef struct rs485Command
{
	uint8_t* data;				//待发送以及返回数据指针，数据返回时将覆盖待发送数据
	uint8_t data_len;			//数据长度
	uint8_t recive_len;			//期望接收数据长度
	uint16_t recive_timeout;	//接收超时时间
}Rs485Command;
导致 HardFault_Handler error ---->  #pragma pack(1) 改为 #pragma pack(2)


8：flash更改SF 
 ChannelsDefaultDatarate = ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;
 
软件复位：
__set_FAULTMASK(uint32_t faultMask);

HAL_NVIC_SystemReset( );

查询CO2时需要多延时5S
HAL_Delay(device.delay);							//某些设备需要额外延时		

PROCESS_YIELD_UNTIL(RF_Tx_Done == s_tRFResult || RF_Tx_Timeout == s_tRFResult);
process_post(&NetworkProcess, NETWORK_EVENT_TX_RF_PACKAGE, NULL);          if (NETWORK_EVENT_TX_RF_PACKAGE == ev) /* post by Comm2Trm-Process */
process_poll(&NetworkProcess);


解读：

1：OnRadioRxDone

RxWindow1Delay = ReceiveDelay1 - RADIO_WAKEUP_TIME;
RxWindow2Delay = ReceiveDelay2 - RADIO_WAKEUP_TIME;

设置接收窗口时间： OnRadioTxDone( void )----> TimerSetValue( &RxWindowTimer1, RxWindow1Delay ); TimerStart( &RxWindowTimer1 );

2：信道：
/*!
 * Current channel index
 */
uint8_t Channel;

配置默认datarate，如果开启ADR/上行应答数据则会自动根据信号强度更改datarate

sync word：只针对FSK有效

RegSyncConfig、RegSyncValue1寄存器配置所得   REG_SYNCCONFIG

RxTimeoutTimer --- SX1276OnTimeoutIrq


if(LoRaCad.Iq_Invert) ///???

3：P2P通讯设置：
3.1：设置IQ：使能：LoRaCad.Iq_Invert

3.2：接收窗口设置：分为RX1 RX2两种模式选择
	 RX2 : 若使用RX2则直接LoRaMacSetDeviceClass( CLASS_C );则可以正常接收，但需要注意长期处于RX mode 会出现异常，需要不定时切换状态；
	 RX1 ：若使用RX1则需要OnRxWindow1TimerEvent()中配置 
	 RxWindowSetup( (Channels[Channel].Frequency), datarate, bandwidth, symbTimeout, false )改为连续接收模式
	 RxWindowSetup( (Channels[Channel].Frequency), datarate, bandwidth, symbTimeout, true )
	 
	 同时更改：
	 OnRadioRxDone 中if( LoRaMacDeviceClass != CLASS_C )  Radio.Sleep( );注释掉否则模块只接收一次后则休眠，不再接收数据

接收窗口设置一次原因： SX1276OnDio0Irq ----->     if( SX1276.Settings.Fsk.RxContinuous == false )
                        {
                            SX1276.Settings.State = RF_IDLE; 标志位重置
                            TimerStart( &RxTimeoutSyncWord );
                        }
3.3：DOWN_LINK必须设置为0， 否则MIC解析出错，该设置后不能再接收大网关数据
	 UpLinkCounter
	 
	
3.4：丢包数：

downLinkCounter默认为0
sequenceCounterPrev = ( uint16_t )downLinkCounter; ///保留上次接收到的数据帧号
sequenceCounterDiff = ( sequenceCounter - sequenceCounterPrev );  ///记录丢包数

3.5：关闭接收窗口
IsRxWindowsEnabled = false;
OnRadioTxDone中将Radio.Sleep( );注释掉否则RX1不接收

3.6：从协议改动最少出发，采用RX2接收数据
使能IQ：LoRaCad.Iq_Invert
IsRxWindowsEnabled = false;
设置RX2信道#define RX_WND_2_CHANNEL                                  { 472100000, DR_1 }  ///505.3  471300000

3.7：while( curMulticastParams != NULL )读取发现curMulticastParams=NULL原因：MulticastChannels指针没分配内存，如下可以解决：
MulticastChannels = (MulticastParams_t *)malloc(sizeof(MulticastChannels));定义为全局否则会重复初始化参数


3.8：OnRadioRxDone  出现fCtrl->Bits.FOptsLen异常原因：多个终端设备同时发送数据，数据间能量碰撞，导致数据OnRadioRxError，解决方式：
 OnRadioRxError：多帧数据发生碰撞时后出现， Radio.Standby( ) --->  OnRxWindow2TimerEvent
设置接收、发送模式必须先设置为Radio.Standby( ); or Radio.Sleep( );否则无效


3.9：该代码版本不兼容大网关原因：
3.9.1：发送、接收前导码长度不一致
Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 20, false, true, 0, 0, false, 3e3 ); ///前导码设置为20

3.9.2：class A模式下接收大网关下行数据出错原因：rx1不同，必须是RX2才可以接收大网关下行数据，因此大小网关不能兼容

3.10：CLASS A模式接收小网关数据失败原因：时钟频率存在差异导致接收失败，解决方式：如下
#define RECEIVE_DELAY2                              2200
#define MAX_RX_WINDOW                               3200
SX1276SetRx ---- 更改为 SX1276SetOpMode( RFLR_OPMODE_RECEIVER );连续1.2S接收数据       
if( rxContinuous == true )
{
    SX1276SetOpMode( RFLR_OPMODE_RECEIVER );
}
else
{
    SX1276SetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );
}



#pragma pack(2)

.\Objects\LoRaWAN_Pro.axf: Error: L6218E: Undefined symbol User_send (referred from gps.o).

LORA_MAX_NB_CHANNELS

    MaxRxWindow = MAX_RX_WINDOW;
    ReceiveDelay1 = RECEIVE_DELAY1;
    ReceiveDelay2 = RECEIVE_DELAY2;
    JoinAcceptDelay1 = JOIN_ACCEPT_DELAY1;
    JoinAcceptDelay2 = JOIN_ACCEPT_DELAY2;

RxWindow2Delay


            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;
            
LoRapp_SenSor_States.loramac_evt_flag = 1;中断调用            
            

OnMacStateCheckTimerEvent添加状态清除？


OnRxWindow1TimerEvent(  );

OnAckTimeoutTimerEvent


AckTimeoutRetry = true;
LoRaMacState &= ~MAC_ACK_REQ;



LoRaMacState &= ~MAC_TX_RUNNING;

NodeAckRequested = false;
McpsConfirm.AckReceived = false;
McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
if( IsUpLinkCounterFixed == false )
{
UpLinkCounter++;
}


AckTimeoutRetry = false;
NodeAckRequested = false;
if( IsUpLinkCounterFixed == false )
{
    UpLinkCounter++;
}
McpsConfirm.NbRetries = AckTimeoutRetriesCounter;



.\Objects\LoRaWAN_Pro.axf: Error: L6218E: Undefined symbol InitSI7021 (referred from si7021.o).