
#ifndef __SIM7600_H
#define __SIM7600_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "main.h"

#define     OPEN                            GPIO_PIN_SET
#define     CLOSE                           GPIO_PIN_RESET     
 
#define 	INITSIMDONE			            0
#define 	INITSIMFAIL			            1	 
#define 	SERVER_ADDR			            "45.77.178.244" ///"60.205.184.237"  //
#define 	SERVER_PORT			            44444///3389            //  

#define 	SimcomSendCmd		            Usart2SendData_DMA

// 命令返回状态
#define     REPLY_NONE						0
#define     REPLY_OK						1
#define     REPLY_ERROR						2
#define     REPLY_CPIN						3
#define     REPLY_CSQ						4
#define     REPLY_CGREG						5
#define     REPLY_NETOPEN					6
#define     REPLY_CIPCLOSE					7
#define     REPLY_NETCLOSE					8
#define     REPLY_INCFUN					9
#define     REPLY_OUTCFUN					10

#define     REPLY_NETOPEN_ERROR				11
#define     REPLY_CIPOPEN_ERROR				12
#define     REPLY_CIPOPEN					13
#define     REPLY_SEND_START				14
#define     REPLY_SEND_DONE					15
#define     REPLY_SENDERROR					16
#define     REPLY_REVER						17

typedef uint8_t commandid_t;

extern commandid_t commandid_reply;

/**********Simcom7600 AT指令ID号************/
enum
{
	AT,
	CHECK_SIM,
	CSQ,
	CGREG, //查询网络是否附着上：到处初始化检测通过
	NETOPEN,
	CIPCLOSE,
	NETCLOSE,
	INCFUN,
	OUTCFUN,
    CTCPKA,
    CIPMODE,
    COMMODE,
    DATAMODE
};

typedef struct simcom_cmd_struct
{
	char *data;				// 命令内容
	uint16_t len;			// 命令长度，不包括结束字符'\0'
	uint32_t timeout;  		// 命令超时时间
	int8_t retry_count;   	// 重试次数
	int8_t expect_retval;  	// 期望的返回值
}simcom_cmd_t;

extern simcom_cmd_t cmds[];

void UART2_RxDmaCallback(void);

int8_t InitSimcom(void);

void SimcomPower(uint8_t State);

int8_t SimcomExecuteCmd(simcom_cmd_t cmd);

void SimcomOpenNet(void);

int8_t SimcomSetServer(char *server, uint16_t port, uint16_t waitMs, int8_t retryCount);

void SimcomConnectServer(void);

void SimcomSendData(char *buffer, uint8_t len);

void SimcomCloseNet(void);

void SpliceSend(uint8_t *buf, uint8_t len);

#ifdef __cplusplus
}
#endif
#endif
