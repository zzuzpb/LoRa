/*
 * user_sim.c	sim模块驱动文件
*/

#include "user_sim3.h"

extern UART_HandleTypeDef hlpuart1;

char SimReChar[1]={0};								//接收的一个字节
char SimReBuff[SIM_RECIEVE_COUNT_MAX]={0};			//接收到的数据
volatile uint16_t SimReCount;						//接收到的字节数

char SIM_REPLY_USER_DATA[SIM_RECIEVE_COUNT_MAX];	//特殊指令中，如短信、GPS位置信息等，SIM模块返回的数据

E_SimState G_SimState       = E_SIM_DISCONNECT;	    //标识SIM模块状态
uint8_t G_IsSendData=0;
T_AtCmd G_Atcmd;

/*
 * SimIsConnectServer:	查询sim模块是否连接着服务器
 * 参数:				无
 * 返回值:				1表示连接上了，0表示未连接
*/
uint8_t SimIsConnectServer(void)
{
	if(G_SimState==E_SIM_CONNECT)
		return 1;
	else
		return 0;
}
/*
 * SimGetState:	获取sim模块当前状态
 * 参数:
 * 返回值:
*/
E_SimState SimGetState(void)
{
	return G_SimState;
}
/*
 * SimPowerOn:			打开sim模块电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimPowerOn(void)
{
	HAL_GPIO_WritePin(Out_SIM_Power_ON_GPIO_Port, Out_SIM_Power_ON_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Out_SIM_EN_GPIO_Port, Out_SIM_EN_Pin, GPIO_PIN_SET);
	HAL_Delay(100);
	return 1;
}
/*
 * SimCheckModle:		检测模块是否通信正常
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimCheckModle(void)
{
	if(SimExecuteCmd("ATE0\r\n","OK",2000,5)!=AT_OK)
    {
        G_SimState = E_SIM_ERROR;
        DEBUG("通讯失败\r\n");
		return 0;
	}
    if(SimExecuteCmd("AT+CPIN?\r\n","+CPIN: READY",3000,5)!=AT_OK)
    {
        DEBUG("SIM卡未插入\r\n");
        G_SimState = E_SIM_ERROR;
        return 0;
    }
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
uint8_t SimSendAtCmd()
{
    uint32_t starttime=HAL_GetTick();
    //DEBUG("%u\r\n",SimReCount);
    while( SimReCount != 0 && HAL_GetTick()-starttime < 500 );
    HAL_NVIC_DisableIRQ(RNG_LPUART1_IRQn);
    uint8_t ret=HAL_UART_Transmit(&hlpuart1,(uint8_t *)G_Atcmd.Command,strlen(G_Atcmd.Command),0xffff);
	if(ret!=HAL_OK)
    {
        DEBUG("串口发送失败:%x\r\n",ret);
		return 0;
    }
    HAL_NVIC_EnableIRQ(RNG_LPUART1_IRQn);
	return 1;
}

E_AtState SimExecuteCmd(char *Cmd,char *Reply,uint32_t TimeOut,uint8_t Retry)
{
    G_Atcmd.Command     =Cmd;
    G_Atcmd.ExpectReply =Reply;
    G_Atcmd.WaitTime    =TimeOut;
    G_Atcmd.RetryCount  =Retry;
    G_Atcmd.AtRlplyState=AT_NONE;
	for(uint8_t i=0;i<G_Atcmd.RetryCount;i++)
	{
        //DEBUG("%s,%d",G_Atcmd.Command,strlen(G_Atcmd.Command));
        DEBUG("%s",G_Atcmd.Command);
		uint32_t StartTime=HAL_GetTick();
		if(SimSendAtCmd()!=1)
		{
			DEBUG("Sim Module Error\r\n");
			return AT_ERROR;
		}
		while(G_Atcmd.AtRlplyState==AT_NONE && HAL_GetTick()-StartTime<G_Atcmd.WaitTime);
		if(G_Atcmd.AtRlplyState==AT_OK)
		{
			break;
		}
	}
    G_Atcmd.Command = NULL;
	if(G_Atcmd.AtRlplyState==AT_NONE)
		return AT_FAIL;
	return G_Atcmd.AtRlplyState;
}
/*
 * SimSetBaud:			设置波特率
 * 参数:				波特率
 * 返回值:				1成功 0失败
*/
uint8_t SimSetBaud(uint32_t Baud)
{
	char command[32];
	sprintf(command,"AT+IPR=%u\r\n",Baud);
	//以下写法是错误的，虽然语法上没有问题
	//sprintf(at_cmd[AT_CMD_CONNECT_SERVER].Command,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",ServerIP,ServerPort);
	if(SimExecuteCmd(command,"OK",3000,2)==AT_OK)
		return 1;
	return 0;
}
/*
 * SimGetIccid:			获取Sim卡的电话号码
 * 参数PhoneNum:		电话号码
 * 返回值:				电话号码
*/
char * SimGetIccid(char *PhoneNum)
{
	if(SimExecuteCmd("AT+CCID\r\n","OK",3000,2)!=AT_OK)
		return NULL;

    if(SIM_REPLY_USER_DATA[0]!='\0')
    {
        for(uint8_t i=0;i<strlen(SIM_REPLY_USER_DATA);i++)
        {
            if(SIM_REPLY_USER_DATA[i]!='\r' && SIM_REPLY_USER_DATA[i+1]=='\r')
            {
                SIM_REPLY_USER_DATA[i+1]='\0';
                break;
            }
        }
        sprintf(PhoneNum,"%s",SIM_REPLY_USER_DATA);
        return PhoneNum;
    }
    else
    {
        return NULL;
    }
}
/*
 * SimGetImei:			获取设备的号码
 * 参数ImeiNum:		模块设备号
 * 返回值:				电话号码
*/
char * SimGetImei(char *ImeiNum)
{
	if(SimExecuteCmd("AT+GSN\r\n","OK",3000,2)!=AT_OK)
		return NULL;
	if(SIM_REPLY_USER_DATA[0]!='\0')
	{
//        DEBUG("接收到:");
//        DEBUG("%s",SIM_REPLY_USER_DATA);
        for(uint8_t i=0;i<strlen(SIM_REPLY_USER_DATA);i++)
        {
                if(SIM_REPLY_USER_DATA[i]!='\r' && SIM_REPLY_USER_DATA[i+1]=='\r')
                {
                        SIM_REPLY_USER_DATA[i+1]='\0';
                        break;
                }
        }
        sprintf(ImeiNum,"%s",SIM_REPLY_USER_DATA);
        return ImeiNum;
	}
	else
	{
        return NULL;
	}
}

/*
 * SimGetCSQ:		查询信号质量
 * Csq:		        信号质量值
 * 返回值:			信号质量值 单位是 -dbm,0表示获取失败
*/
uint8_t SimGetCSQ(uint8_t *Csq)
{
    uint32_t csq,ber;
	if(SimExecuteCmd("AT+CSQ\r\n","OK",5000,2)!=AT_OK)
		return 0;
	if(SIM_REPLY_USER_DATA[0]!='\0')
	{
        //
        for(uint8_t i=0;i<strlen(SIM_REPLY_USER_DATA);i++)
        {
            if(SIM_REPLY_USER_DATA[i]!='\r' && SIM_REPLY_USER_DATA[i+1]=='\r')
            {
                    SIM_REPLY_USER_DATA[i+1]='\0';
                    break;
            }
        }
        //DEBUG("%s",SIM_REPLY_USER_DATA);
        *Csq = 99;
        uint8_t ret = sscanf(SIM_REPLY_USER_DATA,"+CSQ:%u,%u",&csq,&ber);
        if(ret != 2)
        {
            return 0;
        }
        else
        {
            *Csq = csq;
            if(csq>1 && csq<32)
                return 110-(csq-2)*2;
            else
                return 0;
        }
	}
	else
	{
        return 0;
	}
}
/*
 * SimCheckCard:		检测Sim卡时候插入
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t SimSetApn(char * pApn)
{
//    SimSetApn("CMNET");
//    SimSetApn("CMIOT");
//    SimSetApn("UNINET");
//    SimSetApn("3GNET");
    char temp[32]={0};
    //sprintf(temp,"AT+CGDCONT=1,\"IP\",\"%s\"\r\n",pApn);
    sprintf(temp,"AT+CIPCSGP=1,\"%s\"\r\n",pApn);
	if(SimExecuteCmd(temp,"OK",3000,2)!=AT_OK)
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
    char PhoneNum[32]={0};
    char ImeiNum[32]={0};

	SimPowerOn();

	if(HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)SimReChar,1)!=HAL_OK)//打开串口接收中断
		return 0;

    HAL_Delay(10);

    SimReCount      =0;         //打开串口会与一个数据输出到中断
    G_IsSendData    =0;
    G_SimState      =E_SIM_INIT;

	if(SimExecuteCmd("ATE0\r\n","OK",2000,10)!=AT_OK)
    {
        DEBUG("波特率无法同步\r\n");
        G_SimState = E_SIM_ERROR;
		return 0;
    }
    if(SimCheckModle()==0)
        return 0;

    //等待模块初始化完成
	if(SimExecuteCmd("AT+CIURC=1\r\n","SMS Ready",120000,1)!=AT_OK)
		return 0;

    //设置工作模式
	if(SimExecuteCmd("AT+CGCLASS=\"CC\"\r\n","OK",2000,2)!=AT_OK)//所有新的SIM模块都需要设置一次工作模式，否则会出现PDP DEACT
                                              //模式CC是兼容目前所有SIM卡的，联通、移动、物联卡都可以使用
		return 0;

    SimGetImei(ImeiNum);

    SimGetIccid(PhoneNum);

	if(ImeiNum[0] != '\0')
	{
		if(PhoneNum[0]!='\0')
			DEBUG(";ccid:%s,%s;\r\n",PhoneNum,ImeiNum);
		else
			DEBUG(";ccid:error;\r\n");
	}
	else
	{
        DEBUG(";ccid:error;\r\n");
	}
	return 1;
}

/*
 *	SimcomG_SimState:通过SIMCOM模块连接服务器
 *	ServerIP:			服务器地址，可以是ip或者域名，调用者需自行保障地址正确性
 *	ServerPort:			服务器端口，0~65535
 *	返回值：			1成功|0失败
 */
uint8_t SimConnectServer(char *ServerIP,char *ServerPort)
{
	char command[64];
	sprintf(command,"AT+CIPSTART=\"TCP\",\"%s\",\"%s\"\r\n",ServerIP,ServerPort);
	if(SimExecuteCmd(command,"CONNECT OK",25000,1)==AT_OK)
    {
        G_SimState=E_SIM_CONNECT;
        return 1;
    }
	return 0;
}

/*
 * SimSendData:			发送数据到已连接的IP地址
 * data:				要发送的数据
 * len:                 数据长度
 * 返回值:				1成功 0失败
*/
uint8_t SimSendData(char *data,uint16_t len)
{

	data[len]=0x1A;			//结束字符
    data[len+1]=0;
	if(SimExecuteCmd("AT+CIPSEND\r\n","<",2000,1)==AT_OK)
	{
		HAL_Delay(100);
        if(SimExecuteCmd(data,"SEND OK",2000,1)==AT_OK)
		{
			G_SimState=E_SIM_CONNECT;
			return 1;
		}
	}
	G_SimState=E_SIM_DISCONNECT;
	return 0;
}
/*
 * SimEntertSleepMode:	Sim模块进入休眠模式
 * 参数Mode:			1对应休眠模式1，2对应休眠模式2
 * 返回值:				1成功0失败
*/
uint8_t SimEntertSleepMode(uint8_t Mode)
{
	if(Mode!=1 && Mode!=2)
		return 0;
	char command[32]={0};
	sprintf(command,"AT+CSCLK=%u\r\n",Mode);
	if(SimExecuteCmd(command,"OK",2000,2)!=AT_OK)
		return 0;
	return 1;
}
/*
 * SimCloseConnect:	    断开网络连接
 * 参数:      			无
 * 返回值:				1成功0失败
*/
uint8_t SimCloseConnect(void )
{
	if(SimExecuteCmd("AT+CIPSHUT\r\n","SHUT OK",5000,1)!=AT_OK)
		return 0;
    G_SimState = E_SIM_DISCONNECT;
	return 1;
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
	DEBUG("%c",SimReChar[0]);

	if ( SimReBuff[SimReCount-2]=='>' && SimReBuff[SimReCount-1]==' ' ) 		// '>'进入数据传输状态，该命令不以"\r\n"为结尾
	{
		G_Atcmd.AtRlplyState=AT_OK;
        G_IsSendData=1;
		SimReCount = 0;
	}
    else if ( SimReCount<4 && SimReBuff[SimReCount-2]=='\r' && SimReBuff[SimReCount-1]=='\n' )
        SimReCount = 0;//将sim模块前面的\r\n去掉，去掉空白行
	else if ( SimReCount>=4 && SimReBuff[SimReCount-2]=='\r' && SimReBuff[SimReCount-1]=='\n' )	// 收到"\r\n"，有新命令回复
	{
        if (strstr(SimReBuff,G_Atcmd.ExpectReply)!=NULL)
        {
            SimReBuff[SimReCount]='\0';
            sprintf(SIM_REPLY_USER_DATA+strlen(SIM_REPLY_USER_DATA),"%s",SimReBuff);
            G_Atcmd.AtRlplyState=AT_OK;
        }
        else
        {
            if (strstr(SimReBuff,"CONNECT FAIL")!=NULL)
            {
                G_Atcmd.AtRlplyState=AT_ERROR;
            }

            if (strstr(SimReBuff,"CLOSED")!=NULL)
            {
                G_SimState=E_SIM_DISCONNECT;
            }
        }
		SimReCount = 0;
	}
	else if ( SimReCount>15 && SimReBuff[SimReCount-2]!='\r' && SimReBuff[SimReCount-1]=='\n')	// 服务器发送数据过来
	{
        SimReBuff[SimReCount]=0;
        SimRxSvMsgCallback(SimReBuff,SimReCount);
        SimReCount = 0;
	}
    HAL_StatusTypeDef ret=HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)SimReChar,1);//重新打开串口接收中断
    if(ret!=HAL_OK)//重新打开串口接收中断
    {
        DEBUG("hlpuart1总线:%x,%x,%x\r\n",ret,HAL_UART_GetError(&hlpuart1),HAL_UART_GetState(&hlpuart1));
        if (HAL_UART_Init(&hlpuart1) != HAL_OK)
        {
            DEBUG("重启:失败\r\n");
            while(1);//独立看门狗会复位
        }
        do
        {
            ret = HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)SimReChar,1);
        }while(ret!=HAL_OK);//独立看门狗会复位
        DEBUG("重启:成功\r\n");
    }
}
