/*
 * user_sim.h	sim模块驱动头文件
*/

#ifndef __USER_SIM_H__
#define __USER_SIM_H__

#include "stm32l0xx_hal.h"

#define SIM_RECIEVE_COUNT_MAX	256

typedef enum AT_CMD
{
	AT_CMD_CLOSE_ECHO 			=0,				// 关闭回显	  
	AT_CMD_WAIT_INIT,							// 等待CALLREADY状态（call ready代表模块准备好了
	AT_CMD_CONNECT_SERVER,						// 连接服务器
	AT_CMD_START_TRAN,							// 进入数据传输状态
	AT_CMD_SEND_DATA,							// 开始传输数据
	AT_CMD_CLOSE_CONNECTION,					// 关闭服务器连接
	AT_CMD_CHECK_CONNECT,						// 查询连接状态
	AT_CMD_SET_CLASS,							// 设置工作模式为
    
	AT_CMD_USER									//用户自定义命令，一定要放在最后
}AtCmd;

typedef enum AT_STATE	//AT指令的执行返回状态
{
	AT_NONE,	//无返回
	AT_OK,		//执行成功
	AT_ERROR,	//执行错误
	AT_FAIL		//执行失败(超时失败)
}AtState;

typedef struct AT_CMD_STRUCT
{
	char *Command;				//指令字符串
	char *ExpectReply;			//期望接收到的字符串
	uint32_t WaitTime;			//超时时间
	uint16_t RetryCount;		//重试次数
	AtState AtRlplyState;		//命令执行返回的状态
}AtCmdStruct;

/*
 * SimInit:				初始化sim模块
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimInit(void);

/*
 * SimPowerOn:			打开sim模块电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimPowerOn(void);

/*
 * SimPowerOff:			关闭sim模块电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimPowerOff(void);

/*
 * SimSendAtCmd:	发送At指令给Sim模块
 * cmd:				At指令，具体可见AtCmd的定义
 * 返回值:			1成功 0失败
*/
uint8_t SimSendAtCmd(AtCmd cmd);

/*
 * SimCheckReply:	检测Sim模块的回复状态，主要用于修改AtCmdStruct结构体的AtRlplyState
 * 参数:			无
 * 返回值:			无
*/
void SimCheckReply(void);

/*
 * SimConnectServer:	初始化sim模块
 * ServerIP:			服务器IP地址
 * ServerPort:			服务器端口
 * 返回值:				1成功 0失败
*/
uint8_t SimConnectServer(char *ServerIP,char *ServerPort);

/*
 * SimExecuteCmd:		执行配置好的AT指令
 * cmd:					配置好的AT指令
 * 返回值:				成功返回AT_OK，具体见AtState定义
*/
AtState SimExecuteCmd(AtCmd cmd);
/*
 * SimSendData:			发送数据到已连接的IP地址
 * data:				要发送的数据
 * ServerPort:			服务器端口
 * 返回值:				1成功 0失败
*/
uint8_t SimSendData(char *data);

/*
 * SimGetIccid:			获取Sim卡的电话号码
 * 参数PhoneNum:		电话号码
 * 返回值:				电话号码
*/
char * SimGetIccid(char *PhoneNum);

uint8_t SimHttpInit(void);
uint8_t SimHttpSetUrl(char *url);
uint8_t SimHttpSetBreak(uint32_t BreakPoint);
uint8_t SimHttpSetBreakEnd(uint32_t BreakEnd);
uint8_t SimHttpGet(void);
uint8_t SimHttpReadData(void);


void Lpuart1Send(char *p, uint16_t len);
#endif /* __USER_SIM_H__ */
