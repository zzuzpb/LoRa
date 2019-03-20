/*
 * user_sim.c	sim模块驱动文件
*/

#include "user_sim.h"
#include <string.h>
#include "bootloader_config.h"
#include "user_flash_L072.h"

#include "iap_protocol.h"

extern UART_HandleTypeDef hlpuart1;

char SimReChar[1]={0};								//接收的一个字节
char SimReBuff[SIM_RECIEVE_COUNT_MAX]={0};			//接收到的数据
uint16_t SimReCount=0;								//接收到的字节数

volatile uint8_t G_IsGetData=0;

AtCmdStruct at_cmd[]=
{
	/*指令字符串				期望接收到的字符串		每次超时时间	重试次数	命令执行返回的初始状态}*/
	{"ATE0\r\n",			"OK",					1000,			10,			AT_NONE},	//关闭回显，尝试执行10次每次等待1s,初始的返回状态为AT_NONE,成功返回AT_OK
	{"AT+CIURC=1\r\n",		"SMS Ready",			120000,			1,			AT_NONE},	//等待模块call ready，等待120s
	{NULL,					"CONNECT OK",			20000,			1,			AT_NONE},	//连接服务器,具体服务器由用户指定
	{"AT+CIPSEND\r\n",		">",					2000,			1,			AT_NONE},	//进入发送数据状态
	{NULL,					"SEND OK",				30000,			1,			AT_NONE},	//通过已经建立的连接发送数据，数据及长度、超时时间由程序具体设定
	{"AT+CIPSHUT\r\n",		"SHUT OK",				2000,			1,			AT_NONE},	//关闭连接
	{"AT+CIPSTATUS\r\n",	"CONNECT OK",			2000,			1,			AT_NONE},	//查询TCP连接状态
	{"AT+CGCLASS=\"CC\"\r\n","OK",					2000,			1,			AT_NONE},	//设置工作模式为CC
    
	{NULL,				NULL,					0,				0,			AT_NONE}	//用户自定义命令，一定要放在最后
};

/*
 * SimPowerOn:			打开sim模块电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimPowerOn(void)
{
	HAL_GPIO_WritePin(Out_SIM_Power_ON_GPIO_Port, Out_SIM_Power_ON_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Out_SIM_EN_GPIO_Port, Out_SIM_EN_Pin, GPIO_PIN_SET);
	return 1;
}

/*
 * SimPowerOff:			关闭sim模块电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimPowerOff(void)
{
	HAL_GPIO_WritePin(Out_SIM_EN_GPIO_Port, Out_SIM_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Out_SIM_Power_ON_GPIO_Port, Out_SIM_Power_ON_Pin, GPIO_PIN_RESET);
	return 1;
}

/*
 * SimSendAtCmd:	发送At指令给Sim模块
 * cmd:				At指令，具体可见AtCmd的定义
 * 返回值:			1成功 0失败
*/
uint8_t SimSendAtCmd(AtCmd cmd)
{
    HAL_NVIC_DisableIRQ(RNG_LPUART1_IRQn);
	if(HAL_UART_Transmit(&hlpuart1,(uint8_t *)at_cmd[cmd].Command,strlen(at_cmd[cmd].Command),0xffff)!=HAL_OK)
    {
        HAL_NVIC_EnableIRQ(RNG_LPUART1_IRQn);
        printf("发送失败\r\n");
		return 0;
    }
    HAL_NVIC_EnableIRQ(RNG_LPUART1_IRQn);
    return 1;
}

AtState SimExecuteCmd(AtCmd cmd)
{
	at_cmd[cmd].AtRlplyState=AT_NONE;
	for(uint8_t i=0;i<at_cmd[cmd].RetryCount;i++)
	{
		printf("%s",at_cmd[cmd].Command);//,strlen(at_cmd[cmd].Command));
		uint32_t StartTime=HAL_GetTick();
		if(SimSendAtCmd(cmd)!=1)
			return AT_ERROR;
		while(at_cmd[cmd].AtRlplyState==AT_NONE && HAL_GetTick()-StartTime<at_cmd[cmd].WaitTime);
		if(at_cmd[cmd].AtRlplyState==AT_OK)
			break;
	}
	if(at_cmd[cmd].AtRlplyState==AT_NONE)
		return AT_FAIL;
	HAL_Delay(100);//延时防止模块反应不过来
    if(cmd==AT_CMD_USER)
    {
        at_cmd[AT_CMD_USER].Command=NULL;
	}
    return at_cmd[cmd].AtRlplyState;
}

uint8_t SimHttpInit(void)
{
	at_cmd[AT_CMD_USER].Command="AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n";//设置承载，必须设置
	at_cmd[AT_CMD_USER].ExpectReply="OK";
	at_cmd[AT_CMD_USER].WaitTime=1000;
	at_cmd[AT_CMD_USER].RetryCount=1;
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;
	
//	at_cmd[AT_CMD_USER].Command="AT+SAPBR=3,1,\"APN\",\"CMNET\"\r\n";//设置APN，不设置表示使用默认的APN
//	at_cmd[AT_CMD_USER].ExpectReply="OK";
//	at_cmd[AT_CMD_USER].WaitTime=1000;
//	at_cmd[AT_CMD_USER].RetryCount=1;
//	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
//		return 0;
	
	at_cmd[AT_CMD_USER].Command="AT+SAPBR=1,1\r\n";//激活GPRS，必须激活
	at_cmd[AT_CMD_USER].ExpectReply="OK";
	at_cmd[AT_CMD_USER].WaitTime=30000;
	at_cmd[AT_CMD_USER].RetryCount=1;
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;

	at_cmd[AT_CMD_USER].Command="AT+HTTPINIT\r\n";
	at_cmd[AT_CMD_USER].ExpectReply="OK";
	at_cmd[AT_CMD_USER].WaitTime=1000;
	at_cmd[AT_CMD_USER].RetryCount=1;
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;
	
//	at_cmd[AT_CMD_USER].Command="AT+HTTPPARA=\"CID\",1\r\n";	//可不设置，设置了之后有可能导致设备联网失败
//	at_cmd[AT_CMD_USER].ExpectReply="OK";
//	at_cmd[AT_CMD_USER].WaitTime=1000;
//	at_cmd[AT_CMD_USER].RetryCount=1;
//	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
//		return 0;
	return 1;
}

uint8_t SimHttpSetUrl(char *url)
{
	char command[128]={0};
	sprintf(command,"AT+HTTPPARA=\"URL\",\"%s\"\r\n",url);
	at_cmd[AT_CMD_USER].Command=command;
	at_cmd[AT_CMD_USER].ExpectReply="OK";
	at_cmd[AT_CMD_USER].WaitTime=1000;
	at_cmd[AT_CMD_USER].RetryCount=1;
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;
	return 1;
}

uint8_t SimHttpSetBreak(uint32_t BreakPoint)
{
	char command[64]={0};
	sprintf(command,"AT+HTTPPARA=\"BREAK\",%u\r\n",BreakPoint);
	at_cmd[AT_CMD_USER].Command=command;
	at_cmd[AT_CMD_USER].ExpectReply="OK";
	at_cmd[AT_CMD_USER].WaitTime=1000;
	at_cmd[AT_CMD_USER].RetryCount=1;
    G_IsGetData=0;
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;
	return 1;
}

uint8_t SimHttpSetBreakEnd(uint32_t BreakEnd)
{
	char command[64]={0};
	sprintf(command,"AT+HTTPPARA=\"BREAKEND\",%u\r\n",BreakEnd);
	at_cmd[AT_CMD_USER].Command=command;
	at_cmd[AT_CMD_USER].ExpectReply="OK";
	at_cmd[AT_CMD_USER].WaitTime=1000;
	at_cmd[AT_CMD_USER].RetryCount=1;
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;
	return 1;
}

uint8_t SimHttpGet(void)
{
	at_cmd[AT_CMD_USER].Command="AT+HTTPACTION=0\r\n";
	at_cmd[AT_CMD_USER].ExpectReply="+HTTPACTION:";
	at_cmd[AT_CMD_USER].WaitTime=60000;
	at_cmd[AT_CMD_USER].RetryCount=1;
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;
	return 1;
}

uint8_t SimHttpReadData(void)
{
	at_cmd[AT_CMD_USER].Command="AT+HTTPREAD\r\n";
	at_cmd[AT_CMD_USER].ExpectReply="+HTTPREAD:";
	at_cmd[AT_CMD_USER].WaitTime=30000;
	at_cmd[AT_CMD_USER].RetryCount=1;
	G_IsGetData=1;                                  //开始接收网络数据
	if(SimExecuteCmd(AT_CMD_USER)!=AT_OK)
		return 0;

	return 1;
}
/*
 * SimInit:				初始化sim模块，模块可以
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimInit(void)
{
    G_IsGetData=0;

	if(SimPowerOn()==0)
		return 0;
	if(HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)SimReChar,1)!=HAL_OK)//打开串口接收中断
		return 0;
	if(SimExecuteCmd(AT_CMD_CLOSE_ECHO)!=AT_OK)
		return 0;
	if(SimExecuteCmd(AT_CMD_WAIT_INIT)!=AT_OK)
		return 0;
	if(SimExecuteCmd(AT_CMD_SET_CLASS)!=AT_OK)
		return 0;
    if(SimHttpInit()!=1)
        return 0;
    return 1;
}

/*
 * SimConnectServer:	初始化sim模块
 * ServerIP:			服务器IP地址
 * ServerPort:			服务器端口
 * 返回值:				1成功 0失败
*/
uint8_t SimConnectServer(char *ServerIP,char *ServerPort)
{
	char command[64];
	sprintf(command,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",ServerIP,ServerPort);
	//以下写法是错误的，虽然语法上没有问题
	//sprintf(at_cmd[AT_CMD_CONNECT_SERVER].Command,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",ServerIP,ServerPort);
	at_cmd[AT_CMD_CONNECT_SERVER].Command=command;
	for(uint8_t i=0;i<2;i++)//重连2次，失败则跳出
		if(SimExecuteCmd(AT_CMD_CONNECT_SERVER)==AT_OK)
			return 1;
	return 0;
}

/*
 * SimSendData:			发送数据到已连接的IP地址
 * data:				要发送的数据
 * 返回值:				1成功 0失败
*/
uint8_t SimSendData(char *data)
{
	char temp[256]={0};
	uint8_t len;
	len=sprintf(temp,"%s",data);
	temp[len]=0x1A;			//结束字符
	at_cmd[AT_CMD_SEND_DATA].Command=temp;
	at_cmd[AT_CMD_SEND_DATA].AtRlplyState=AT_NONE;
	if(SimExecuteCmd(AT_CMD_START_TRAN)==AT_OK)
	{
		//printf("send:%s",temp);
		if(SimExecuteCmd(AT_CMD_SEND_DATA)==AT_OK)
			return 1;
	}
	return 0;
}

/*
 * SimCheckReply:	检测Sim模块的回复状态，主要用于修改AtCmdStruct结构体的AtRlplyState
 * 参数:			无
 * 返回值:			无
*/
void SimCheckReply(void)
{
	if(SimReCount==SIM_RECIEVE_COUNT_MAX)			//溢出则丢弃之前的数据
		SimReCount=0;
	SimReBuff[SimReCount++]=SimReChar[0];		//保存接收到的数据
	//printf("%c",SimReChar[0]);
	
	if ( SimReBuff[SimReCount-1]=='>' ) 		// '>'进入数据传输状态，该命令不以"\r\n"为结尾
	{
		for (int8_t i=sizeof(at_cmd)/sizeof(at_cmd[0])-1; i>=0; i--)
		{
			if (*at_cmd[i].ExpectReply=='>')
			{
				at_cmd[i].AtRlplyState=AT_OK;
			}
		}
		SimReCount = 0;
	}
    else if ( SimReCount<4 && SimReBuff[SimReCount-2]=='\r' && SimReBuff[SimReCount-1]=='\n' )
        SimReCount = 0;//将sim模块前面的\r\n去掉，去掉空白行
	else if ( SimReCount>=4 && SimReBuff[SimReCount-2]=='\r' && SimReBuff[SimReCount-1]=='\n' )	// 收到"\r\n"，有新命令回复
	{
        for (int8_t i=sizeof(at_cmd)/sizeof(at_cmd[0])-1; i>=0; i--)
        {
            if (strstr(SimReBuff,at_cmd[i].ExpectReply)!=NULL)
            {
                at_cmd[i].AtRlplyState=AT_OK;
            }
            if (strstr(SimReBuff,"ERROR")!=NULL)
            {
                at_cmd[i].AtRlplyState=AT_ERROR;
            }
        }
        if(G_IsGetData==1)//接收到网络数据
        {
            if(SimReBuff[0]==':')//开头是:的，认为是hex文件
            {
                HandleIAPData(SimReBuff,SimReCount);
            }
        }
		SimReCount = 0;
	}
	HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)SimReChar,1);//重新打开串口接收中断
}

void Lpuart1Send(char *p, uint16_t len)
{	
	HAL_Delay(10);
	HAL_UART_Transmit(&hlpuart1,(uint8_t *)p,len,0xffff);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if(UartHandle==&hlpuart1)		//sim800模块
	{
		SimCheckReply();
	}
}
