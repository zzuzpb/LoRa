/*
 * user_sim.h	sim模块驱动头文件
*/

#ifndef __USER_SIM3_H__
#define __USER_SIM3_H__

#include "user_config.h"

#define SIM_RECIEVE_COUNT_MAX	128


typedef enum AT_STATE	//AT指令的执行返回状态
{
	AT_NONE=0,	    //无返回
	AT_OK,		    //回复成功
	AT_ERROR,	    //回复错误
	AT_FAIL		    //执行失败(超时失败)
}E_AtState;

typedef struct AT_CMD_STRUCT
{
	char *Command;				//指令字符串
	char *ExpectReply;			//期望接收到的字符串
	uint32_t WaitTime;			//超时时间
	uint16_t RetryCount;		//重试次数
	E_AtState AtRlplyState;		//命令执行返回的状态
}T_AtCmd;

typedef enum            	    //SIM模块状态
{
    E_SIM_INIT      =(0x01U<<0),	// 初始化
	E_SIM_CONNECT	=(0x01U<<1),	// 连接上网络
	E_SIM_DISCONNECT=(0x01U<<2),	// 未连接上网络
	E_SIM_ERROR	    =(0x01U<<3)		// 模块硬件物理错误（通讯失败，SIM卡未插入）
}E_SimState;

uint8_t SimInit(void);

uint8_t SimSetBaud(uint32_t Baud);

uint8_t SimCheckModle(void);

E_SimState SimGetState(void);

uint8_t SimCloseConnect(void);

uint8_t SimPowerOn(void);

uint8_t SimPowerOff(void);

void SimCheckReply(void);

char * SimGetImei(char *ImeiNum);

char * SimGetIccid(char *PhoneNum);

uint8_t SimConnectServer(char *ServerIP,char *ServerPort);

E_AtState SimExecuteCmd(char *Cmd,char *Reply,uint32_t TimeOut,uint8_t Retry);

uint8_t SimSendData(char *data,uint16_t len);

uint8_t SimEntertSleepMode(uint8_t Mode);

uint8_t SimIsConnectServer(void);

uint8_t SimQSendInit(void);

uint8_t SimQSendData(char *data,uint16_t len);

uint8_t SimReConnect(uint32_t TimeOut);

uint8_t SimConnectHeartbeat(void);

uint8_t SimGetCSQ(uint8_t *Csq);

/*
 * SimRxServerMsgCallback,服务器接收数据处理函数
 *
 * 此函数只在此处定义，由用户实现具体处理
*/
void SimRxSvMsgCallback(char *RxMsg,uint16_t len);

#endif /* __USER_SIM2_H__ */
