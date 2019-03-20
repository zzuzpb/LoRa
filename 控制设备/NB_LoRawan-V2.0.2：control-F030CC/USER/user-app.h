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

#define AESKEY_LEN                  (16)

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

extern volatile bool ReportTimerEvent;

extern TimerEvent_t ReportTimer;

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

/*
 *	应用层状态标志位
 */
typedef struct{
    bool    Ack_Recived;   ///ACK应答标志
		bool 		Into_Low_Power_State; ///低功耗标志
	  uint8_t loramac_evt_flag;          ///lora发送状态位
}LoRapp_State;

/*
*RF发送包配置
*/
typedef struct{
	uint16_t  RX_LEN; ///接收LEN
	uint16_t  Len;	///RF_BUF数组下标
	uint16_t 	TX_Len;	///发送数据长度
	uint8_t   AT_MODULE_PORT; ///FPort
	uint8_t 	RF_BUF[4];
	uint8_t 	*Send_Buf; ///外部获取数据，保留send mode
	uint8_t 	Error_count;   ///RF发送ERROR计数
	uint8_t		ADR_Datarate;   ///获取到ADR时SF更改
	uint8_t   default_datarate;  ///默认datarate
	bool 			RF_Send;			///RF数据发送完成标记
	bool		  Send_again;	///RF发送失败时,防止重新获取数据
  bool			Estab_Communt_State; ///上电建立通信标志位
}RF_Send;

extern RF_Send RF_Send_Data;

typedef struct{
	bool Cad_Mode;
	bool Cad_Done;
	bool Cad_Detect;
	bool Iq_Invert;
	float symbolTime;
	uint8_t UID[4]; ///接收到的设备ID
	uint32_t TimeOnAir; ///空口时间
	int32_t randtime1; ///发送随机时间范围1
	int32_t randtime2; ///发送随机时间范围2
	uint8_t cad_counter; ///重复扫描次数
	uint8_t Channel_Num; ///信道扫描次数：8通道 = 8次
	int16_t Rssi[8];
	uint8_t Channel_Scan[8];
}LoRaCad_t;

extern LoRaCad_t LoRaCad;


/*****************读取flash获取ADR状态********************/

typedef struct{
    bool LoRaMacSetAdrOnState;
    char ReadSetAdar_Addr;
    uint8_t datarate;
		int16_t stop_time;   ///休眠时间默认24S
	  uint8_t min_datarate;
	  uint8_t max_datarate;
		uint8_t channels;
		uint8_t sync_time;
}Get_Flash_Data;

extern Get_Flash_Data Get_Flash_Datas;  ///读取flash参数


extern LoRapp_State LoRapp_SenSor_States;

typedef void (*mac_callback_t) (mac_evt_t mac_evt, void *msg);

void user_app_init(mac_callback_t mac);

int user_app_send( LoRaFrameType_t frametype, uint8_t *buf, int size, int retry);

uint32_t app_get_devaddr(void);

void Into_Low_Power(void);

void Send_Led(void);

void Rx_Led(void);

void OnReportTimerEvent( void );

void SX1278_Send(uint8_t *send_buf);

void User_send(uint8_t *Send_Buf);

void App_Estab_Communt( void );

void Receive_ConTrol_Data( void );


#endif /* __USER_APP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
