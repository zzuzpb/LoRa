#ifndef __USER_MAIN__
#define __USER_MAIN__

#include "stm32l0xx_hal.h"
#include "NBI_common.h"
#include "NBI_rs485.h"

typedef enum 
{
	NBI_COMMON_REV_OK = 0,
	NBI_COMMON_RUN_OK,
	NBI_COMMON_RUN_ERROR,	
	NBI_COMMON_ARGS_ERROR,		
	NBI_COMMON_NOT_RUN,
	NBI_COMMON_UP_OK = 5,
}type_emun_COMMON_RPY;

typedef struct
{	
	char 	 commandStatus;
	char     commandType;
	char 	 RS485Flag;
	char     Rs485Index[NBI_RS485_SEND_BUFF_LEN];
	char     Rs485type[NBI_RS485_SEND_BUFF_LEN];
	char 	 gpsStatus;
	int 	 seq;
	uint32_t timeOut;		
}stu_flashData;


extern unsigned char backOneRetemeIp[30];
extern unsigned char backOneRetemeport[10];


extern unsigned char backTwoRetemeIp[30];
extern unsigned char backTwoRetemeport[10];
extern uint32_t g_seq;


extern uint8_t g_tcp_error_count  										__attribute__((at(NBI_TCP_FAILE_COUNT))) ;

extern uint8_t g_deviceID[DEVICE_ID_DATA_LEN]         __attribute__((at(DEVICE_ID_ADDRESS)));
extern uint8_t g_version[VERSION_DATA_LEN ]          __attribute__((at(VERSION_ADDRESS)));
extern uint8_t g_rempteip[REMOTE_IP_DATA_LEN]         __attribute__((at(REMOTE_IP_ADDRESS)));
extern uint8_t g_remoteport[REMOTE_PORT_DATA_LEN]         __attribute__((at(REMOTE_PORT_ADDRESS)));
extern uint32_t g_rs485address[NBI_RS485_SEND_BUFF_LEN]         __attribute__((at(RS485_ADDRESS)));

extern stu_flashData g_flashData;   														//__attribute__((at(SYS_STU_DATA_ADDRESS)));

extern uint8_t controlSendData[50];


extern HAL_StatusTypeDef initsystem(void);

extern void disposeServerCommand(void);
extern void RtcShowTime(RTC_DateTypeDef *Date,RTC_TimeTypeDef *Time);

extern void NBI_PWR_EnterSTOPMode_time(int time);
extern void NBI_sendRs485DataforServer(void);

extern int NBI_SendCommonDataforServer(int commonID,unsigned char *pdata,int len);
extern void gsmcheckCommand(void);
extern void disServerCommand(void);













#endif




