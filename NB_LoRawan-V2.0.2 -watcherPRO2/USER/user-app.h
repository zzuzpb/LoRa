/*
**************************************************************************************************************
*	@file	user-app.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief	应用层头文件：连接MAC层
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USER_APP_H
#define __USER_APP_H

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include <stdio.h>
#include <stdint.h>

#define AT_MODULE_PORT							(10)    ///FPort

#define AESKEY_LEN                  			(16)

#define DAY_MIN_5								288    
#define DAY_MIN_10								144
#define DAY_MIN_15								96
#define DAY_MIN_30  							48

///128KB==0X1F400    0x0800000 ---- 0x0801F400
#define DEV_ADDR                       			0x0801F3E8        ////UID 0x0801F3E8 ---- 0x0801F3FA
#define DEV_ADDR_SIZE                   		0x12

/*!
 * Default datarate used by the node
 */
#define LORAMAC_DEFAULT_DATARATE        		0x0801F3D8

#define SET_SLEEPT_ADDR							0x0801F3E0  		///设置休眠时间 0x0801F3E0 ---- 0x0801F3E3
#define SET_ADR_ADDR                    		0x0801F3DC	    ///ADR



/*!
 * When set to 1 the application uses the Over-the-Air activation procedure
 * When set to 0 the application uses the Personalization activation procedure
 */
#define OVER_THE_AIR_ACTIVATION                     0

/*!
 * Application IEEE EUI (big endian)
 */
#define LORAWAN_APPLICATION_EUI                     { 0x12,0x34,0x56,0x78,0x90,0xAB,0xCD,0xEF }


/*!
 * AES encryption/decryption cipher application key
 */
#define LORAWAN_APPLICATION_KEY                     { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C }

// report status in period: 周期内当前状态
typedef enum{
    SYS_STA_IDLE,     ///空闲  
    SYS_STA_TX,
    SYS_STA_WAIT,
}sys_sta_t;

extern TimerEvent_t ReportTimer;
extern volatile bool ReportTimerEvent;

extern TimerEvent_t SleepTimer;
extern volatile bool SleepTimerEvent;

extern bool McuInitialized;
extern bool RtcInitialized;

typedef enum{
	IDLE,
	BUSY,
	DONE,
}LoRaStatus_t;

typedef enum{
	CONFIRMED,
	UNCONFIRMED
}LoRaFrameType_t;

typedef enum{
	INVALID,
	RECEIVED,
	UNRECEIVED,
}LoRaTxAckReceived_t;

typedef enum{
    LORAMAC_NWKSKEY=0,
    LORAMAC_APPSKEY=1,
    LORAMAC_APPKEY=2,
}LoRaMacKey_t;

typedef struct{
    int16_t rssi;
    uint8_t snr;
    uint8_t win;
    uint8_t port;
    uint16_t size;
    uint8_t buf[256];
}LoRaMacRxInfo;

typedef enum{
    MAC_STA_TXDONE,
    MAC_STA_RXDONE,
    MAC_STA_RXTIMEOUT,
    MAC_STA_ACK_RECEIVED,
    MAC_STA_ACK_UNRECEIVED,
    MAC_STA_CMD_JOINACCEPT,
    MAC_STA_CMD_RECEIVED,
}mac_evt_t;

typedef enum Rx_State{
	RFWRITE = 0, ///等待状态
	RADIO,       ///非正常唤醒状态：一帧数据唤醒与解析：唤醒数据，必须再次接收数据
	RFSTART,     ///正常唤醒状态
	RFTX,        ///数据上报
	RFREADY,     ///准备状态,切换位等待
}Rx_State_t;

typedef enum Water_Control{
	WRITE = 0,
	OPEN,
	CLOSE,
}Water_Control_t;


/*
 *	应用层状态标志位
 */
typedef struct{
	bool   				Ack_Recived;   ///ACK应答标志
	bool 				WKUP_State; ///低功耗标志
    bool                Hardware_Exist_GPS;
	Water_Control_t 	Water_Control_State;
	uint8_t 			loramac_evt_flag;          ///lora发送状态位
	Rx_State_t 			Rx_States;     ///射频发送状态
	uint32_t 			Work_Time;
}LoRapp_State;

/*
*RF发送包配置
*/
typedef struct{
	uint16_t  RX_LEN ;
	uint16_t  Len;	///RF_BUF数组下标
	uint16_t  TX_Len;	///发送数据长度
	uint8_t   AT_PORT; ///FPort
	uint8_t   RF_BUF[64];
	uint8_t   *Send_Buf;
	uint8_t   Error_count;   ///RF发送ERROR计数
	uint8_t	  ADR_Datarate;   ///获取到ADR时SF更改
	uint8_t   default_datarate;  ///默认datarate
	uint8_t   Rechargeing;  ///是否充电标志
	uint8_t   Battery;  ///电量
	uint8_t   GPS;    ///GPS是否存在标志
	uint16_t  Sleep_times;
	bool 	  RF_Send;			///RF数据发送完成标记
	bool	  Send_again;	///RF发送失败时,防止重新获取数据
	bool	  Get_sensor; ///获取传感器数据状态
}RF_Send;


extern RF_Send RF_Send_Data;


/*****************读取flash获取ADR状态********************/

typedef struct{
    bool 		LoRaMacSetAdrOnState;
    char 		ReadSetAdar_Addr;
    uint8_t	 	datarate;
	uint8_t 	sleep_times;   ///休眠时间默认24S
	uint8_t	 	min_datarate;
	uint8_t 	max_datarate;
	uint8_t 	channels;
	uint8_t 	sync_time;
}Get_Flash_Data;

extern Get_Flash_Data Get_Flash_Datas;  ///读取flash参数

extern LoRapp_State LoRapp_SenSor_States;

typedef void (*mac_callback_t) (mac_evt_t mac_evt, void *msg);

int PowerXY(int x, int y);

int Convert16To10(int number);

uint32_t Read_DecNumber(char *str);

void String_Conversion(char *str, uint8_t *src, uint8_t len);

uint8_t Read_Flash_Data(void);

void user_app_init(mac_callback_t mac);

int user_app_send( LoRaFrameType_t frametype, uint8_t *buf, int size, int retry);

uint32_t app_get_devaddr(void);

void Into_Low_Power(void);

void SendDone_Led(void);

void Rx_Led(void);

void Error_Led(void);

void PowerEnable_Led(void);

void PowerDisble_Led(void);

void OnReportTimerEvent( void );

void SX1278_Send(uint8_t LoRaFrameTypes,uint8_t *send_buf);

void User_send(uint8_t LoRaFrameTypes,uint8_t *Send_Buf);

void User_Send_Api( void );

void SystemReset(void);

void GpsSendAgainTime(void);


#endif /* __USER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
