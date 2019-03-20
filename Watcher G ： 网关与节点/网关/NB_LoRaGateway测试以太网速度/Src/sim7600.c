/*
*Function:
*Programed by:Ysheng
*Complete date:
*Modified by:
*Modified date:
*Remarks:
*/

#include "sim7600.h"
#include "user-app.h"
#include "usart.h"
#include "debug.h"

#include <stdint.h>
#include "string.h"

#define     UID_0   0x1FFF7A10
#define     UID_1   0x1FFF7A10+0x04
#define     UID_2   0x1FFF7A10+0x08  //uiq id

commandid_t commandid_reply = REPLY_NONE;

simcom_cmd_t cmds[] = {
	/*{command						len,		timeout,		retry,  expect_reply}*/
	{"AT\r\n",						4,			500,			10,			REPLY_OK},	//测试串口是否通
	{"AT+CPIN?\r\n",				10,			500,			20,			REPLY_CPIN},	//查询SIM卡是否已准备就绪
	{"AT+CSQ\r\n",					8,			500,			60,			REPLY_CSQ},	//查询信号强度
	{"AT+CGREG?\r\n",				11,			500,			60,			REPLY_CGREG},	//查询网络是否附着上：到处初始化检测通过
	{"AT+NETOPEN\r\n",				12,			500	,			1,			REPLY_NETOPEN},		//打开网络
	{"AT+CIPCLOSE=1\r\n",			15,			500,			1,			REPLY_CIPCLOSE},		///关闭网络是需要延时5S
	{"AT+NETCLOSE\r\n",				13,			500,			1,			REPLY_NETCLOSE},		//关闭Soucket 10S
	{"AT+CFUN=0\r\n",				11,			5000,			1,			REPLY_INCFUN},  	//进入飞行模式
	{"AT+CFUN=1\r\n",				11,			0,				1,			REPLY_OUTCFUN},  	//退出飞行模式
    {"AT+CTCPKA=1,5,5\r\n",         17,         50,             1,          REPLY_OK},
    {"AT+CIPMODE=1\r\n",            14,         500,            1,          REPLY_OK},
    {"+++",                         3,          1000,           1,          REPLY_OK},
    {"ATO",                         3,          500,            1,          REPLY_OK}
//    {"AT+CIPMODE?\r\n",             13,         500,            1,          REPLY_OK}
};


/*SimcomPower：SIM7600电源模式
*参数：		   OPEN/CLOSE
*返回值：	   无
*/
void SimcomPower(uint8_t State)
{
    HAL_GPIO_WritePin(GPIOE, SIM_EN_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_SET);
}


/*InitSimcom：初始化SIM7600
*参数：		  无
*返回值：	  成功返回0， 失败返回1
*/
int8_t InitSimcom(void)
{
	///注意：模块上电需要延时足够时间再开始初始化
    HAL_Delay(2000);
	
	///串口测试
	if(REPLY_OK != SimcomExecuteCmd(cmds[AT]))
	{
		DEBUG(2,"Send AT is Error\r\n");
		HAL_Delay(5000);
		///模块断电，重新上电使能
	}

#if 1	
	if(REPLY_CPIN != SimcomExecuteCmd(cmds[CHECK_SIM]))
	{
		HAL_Delay(1000);
		printf("line = %d\r\n",__LINE__);
		
		DEBUG(2,"11Send CPIN  is Error Go To CFUN Mode\r\n");
		///备注飞行模式还需要测试
		SimcomExecuteCmd(cmds[INCFUN]);
		HAL_Delay(6000);
		if(REPLY_OUTCFUN != SimcomExecuteCmd(cmds[OUTCFUN]))
		{
		 ///模块断电，重新上电使能
            HAL_GPIO_WritePin(GPIOE, SIM_EN_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_RESET);
            HAL_Delay(10000);
            HAL_GPIO_WritePin(GPIOE, SIM_EN_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_SET);
            HAL_Delay(12000);
		}
	}
	
	if(REPLY_CSQ != SimcomExecuteCmd(cmds[CSQ]))
	{
		DEBUG(2,"22Send CPIN  is Error Go To CFUN Mode\r\n");
		SimcomExecuteCmd(cmds[INCFUN]);
		HAL_Delay(6000);
		if(REPLY_OUTCFUN != SimcomExecuteCmd(cmds[OUTCFUN]))
		{
		 ///模块断电，重新上电使能
            HAL_GPIO_WritePin(GPIOE, SIM_EN_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_RESET);
            HAL_Delay(10000);
            HAL_GPIO_WritePin(GPIOE, SIM_EN_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_SET);
            HAL_Delay(12000);
		}
		///获取CSQ信号强度上报服务器
	}
	
	if(REPLY_CGREG != SimcomExecuteCmd(cmds[CGREG]))
	{
		DEBUG(2,"33Send CPIN  is Error Go To CFUN Mode\r\n");
		SimcomExecuteCmd(cmds[INCFUN]);
		HAL_Delay(6000);
		if(REPLY_OUTCFUN != SimcomExecuteCmd(cmds[OUTCFUN]))
		{
		 ///模块断电，重新上电使能
            HAL_GPIO_WritePin(GPIOE, SIM_EN_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_RESET);
            HAL_Delay(10000);
            HAL_GPIO_WritePin(GPIOE, SIM_EN_Pin|SIM_PWR_KEY_Pin, GPIO_PIN_SET);
            HAL_Delay(12000);
		}
	}
#endif		
	if(commandid_reply == REPLY_ERROR)
	return INITSIMFAIL;	
	else
	return INITSIMDONE;
}

int8_t SimcomExecuteCmd(simcom_cmd_t cmd)
{		
	uint32_t retry_time = 0;
	do
	{
		DEBUG(3,"line = %d\r\n",__LINE__);
        
        commandid_reply = REPLY_NONE;
        
		retry_time = HAL_GetTick();
		
		SimcomSendCmd(cmd.data,cmd.len);
		
		while( commandid_reply !=  cmd.expect_retval && commandid_reply != REPLY_ERROR && HAL_GetTick()-retry_time<cmd.timeout);
		
		if( cmd.expect_retval == REPLY_CIPCLOSE)
		HAL_Delay(5000);
		else if(cmd.expect_retval == REPLY_NETCLOSE) ///关闭网络是需要延时8S
		HAL_Delay(8000);
		
		if(commandid_reply == cmd.expect_retval)
		{
			DEBUG(3,"----reply----%d\r\n",commandid_reply);

			return commandid_reply;	
		}
				
	}while( --cmd.retry_count>0 );
		
	DEBUG(3,"----reply----%d\r\n",commandid_reply);
	return commandid_reply;
}

/*SimcomOpenNet：打开网络
*参数：			 无
*返回值：		 无
*/
void SimcomOpenNet(void)
{
	///使能网络
	if(REPLY_NETOPEN != SimcomExecuteCmd(cmds[NETOPEN]))
	{
		if(commandid_reply == REPLY_ERROR)
		{
			DEBUG(2,"+IP ERROR: Network is already opened need to close\r\n");
		}
	}
}

/*SimcomConnectServer：连接服务器
*参数：			       无
*返回值：		       无
*/
void SimcomConnectServer(void)
{
	///打开网络失败则强制关闭网络，再次打开网络，两次打开失败则重新上电
	if(REPLY_CIPOPEN != SimcomSetServer(SERVER_ADDR, SERVER_PORT, 1000, 1))
	{
        DEBUG(2,"line = %d\r\n",__LINE__);
        
        HAL_Delay(1000);
        
        if( commandid_reply == REPLY_ERROR || commandid_reply == REPLY_CIPOPEN_ERROR) ///联网失败
        {
            SimcomExecuteCmd(cmds[COMMODE]); ///退出透传模式
            HAL_Delay(1000);
            if(REPLY_CIPCLOSE != SimcomExecuteCmd(cmds[CIPCLOSE])) ///快速关闭联网
            {
                DEBUG(2,"CIP IS CLOSE\r\n"); 
            }
            HAL_Delay(1000);
            if(REPLY_CIPOPEN != SimcomSetServer(SERVER_ADDR, SERVER_PORT, 2000, 3)) ///再次联网
            {
                ///模块断电，重新上电使能
                HAL_GPIO_WritePin(GPIOE, SIM_PWR_KEY_Pin, GPIO_PIN_RESET);
                HAL_Delay(10000);
                HAL_GPIO_WritePin(GPIOE, SIM_PWR_KEY_Pin, GPIO_PIN_SET);
                HAL_Delay(10000);
                if(INITSIMDONE == InitSimcom(  ))
                {
                    DEBUG(2,"-----Init Simcom Done-----\r\n");
                    SimcomExecuteCmd(cmds[CTCPKA]); ///设置心跳包
                    
                    SimcomExecuteCmd(cmds[CIPMODE]); ///透传模式        

                    SimcomOpenNet(  );
                    
                    if(REPLY_CIPOPEN != SimcomSetServer(SERVER_ADDR, SERVER_PORT, 1000, 1))
                    {        
                        return;                             
                    }                        
                }
            }
        }
     }
}

/*
 *	SimcomConnectServer:通过SIMCOM模块连接服务器
 *	server:				服务器地址，可以是ip或者域名
 *	port:				服务器端口，0~65535						
 *	waitMs:				每轮发送等待事件，单位ms
 *	retryCount:			发送失败重试次数
 *	返回值：			>0成功|0失败	
 */
int8_t SimcomSetServer(char *server, uint16_t port, uint16_t waitMs, int8_t retryCount)
{
	char startConnect[64];
	uint32_t retry_time = 0;

	uint16_t len = sprintf(startConnect,"AT+CIPOPEN=0,\"TCP\",\"%s\",%d\r\n",server,port); ///后期0更改为随机数0~9
	do
	{
        commandid_reply = REPLY_NONE;
		retry_time = HAL_GetTick();
		
		SimcomSendCmd(startConnect,len);
		
	  	while( commandid_reply !=  REPLY_CIPOPEN && commandid_reply != REPLY_ERROR && HAL_GetTick()-retry_time<waitMs);
		if(commandid_reply ==  REPLY_CIPOPEN)
				return commandid_reply;
        DEBUG(2,"retryCount = %d\r\n",retryCount);
	}while( --retryCount>0 );

	return commandid_reply;
}

/*SimcomSendData：SIM发送数据
*参数：			  无
*返回值：		  无
*/
void SimcomSendData(char *buffer, uint8_t len)
{
	SimcomSendCmd(buffer, len);
}

/*SpliceSend：拼接发送数据函数
*参数：		   无
*返回值：	   无
*/
void SpliceSend(uint8_t *buf, uint8_t len)
{
    len = 0;
    memcpy(buf, Net_Buffers.Device_Id, 16);
    len += 16;
    memcpy(&buf[len++], &Net_Buffers.Versions, 1);
    memcpy(&buf[len++], &Net_Buffers.Type, 1);
    memcpy(&buf[len++], &Net_Buffers.Command, 1);

    memcpy(&buf[len], &Net_Buffers.Len, 1);
//    DEBUG(2,"Net_Buffer.Len : %d\r\n",buf[len++]); 
        
    memcpy(&buf[len], Net_Buffers.SensorBuf, Net_Buffers.Len);
//    uint8_t temp = len;        
//    for(uint8_t i = 0; i<Net_Buffers.Len; i++)
//    DEBUG(2,"%02x",buf[temp + i]);
//    DEBUG(2,"\r\n");
    
    len += Net_Buffers.Len;
    
    memcpy(&buf[len], &Net_Buffers.Seq, 4);
//    uint8_t temp2 = len; 
//    for(uint8_t i = 0; i<4; i++)
//    DEBUG(2,"%02x",buf[temp2 + i]);
//    DEBUG(2,"\r\n");
    
    len += 4;
    
    Net_Buffers.Crc_Data = CalcCRC16(buf, len);
    
    memcpy(&buf[len], &Net_Buffers.Crc_Data, 2);
//    uint8_t temp3 = len; 
//    for(uint8_t i = 0; i<2; i++)
//    DEBUG(2,"%02x",buf[temp3 + i]);
//    DEBUG(2,"\r\n");
    
    len += 2;
    memcpy(&buf[len], &Net_Buffers.EndState, 1);
    
    len ++;
    
//    DEBUG(2,"Net_Buffers : %d\r\n",len);
//    DEBUG(2,"Net_Buffers.Versions : %02x\r\n",Net_Buffers.Versions);
//    DEBUG(2,"Net_Buffers.Command  : %02x\r\n",Net_Buffers.Command);
//    DEBUG(2,"---Net_Buffers.buf--- ");    
//    for(uint8_t i = 16; i<len; i++)
//    DEBUG(2,"%02x",buf[i]);
//    DEBUG(2,"\r\n");

    Net_Buffers.Seq ++;
}

char uartrx1_buffer[64] = {0};

void UART2_RxDmaCallback(void)
{
	uint8_t tmp_flag = 0;
	uint32_t temp;
			
	tmp_flag =  __HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE);   //空闲中断中将已收字节数取出后，停止DMA
	if((tmp_flag != RESET))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&huart2);
		
		HAL_UART_DMAStop(&huart2);
		
		/*       获取DMA当前还有多少没填充       */
		temp  = __HAL_DMA_GET_COUNTER(&hdma_usart2_rx);
		UART_RX_DATA2.USART_RX_Len =  BUFFER_SIZE - temp; 
		
		char Len = UART_RX_DATA2.USART_RX_Len;
				
		memset(uartrx1_buffer,0, Len);
		memcpy(uartrx1_buffer, UART_RX_DATA2.USART_RX_BUF, Len);
		
		SIMCOM_RECIVE(  ) ;
		
		HAL_UART_Receive_DMA(&huart2,(uint8_t *)UART_RX_DATA2.USART_RX_BUF,BUFFER_SIZE);
		
		for(uint8_t i = 0; i<Len;i++)
			DEBUG(2,"%c",uartrx1_buffer[i]);
        
        DEBUG(2,"Len : %d\r\n",Len);

		if ( uartrx1_buffer[Len-1]=='>' ) 	// '>'进入数据传输状态，该命令不以"\r\n"为结尾
		{
			///开始发送数据
			commandid_reply = REPLY_SEND_START;
		}
		else if ( Len>=4 && uartrx1_buffer[Len-2]=='\r' && uartrx1_buffer[Len-1]=='\n' )	// 收到"\r\n"，有新命令回复
		{
			DEBUG(3,"len : %d\r\n",Len);
			// AT串口PING接测试
			if ( (strncmp(uartrx1_buffer+Len-9, "AT", 2)) == 0 && strncmp(uartrx1_buffer+Len-4, "OK", 2) == 0 ) // 返回的是OK
			{
				commandid_reply = REPLY_OK;
				DEBUG(3,"at\r\n");
			}
				
			else if ( Len>=7 && Len<66 && strncmp(uartrx1_buffer+Len-7, "ERROR", 5) == 0 ) // 返回的是ERROR
			{
				commandid_reply = REPLY_ERROR;
			}
				
			///CPIN
			if ( Len>=31 && strncmp(uartrx1_buffer+Len-20, "+CPIN:", 6) == 0 ) 
			{
				if(strstr(uartrx1_buffer, "READY")!=NULL)
				commandid_reply = REPLY_CPIN;
				else
					commandid_reply = REPLY_ERROR;
				
				DEBUG(2,"--- CPIN --- \r\n");
			}
			
			///CSQ
			if ( Len>=21 && strncmp(uartrx1_buffer+Len-19, "+CSQ:", 5) == 0 ) //27
			{
				commandid_reply = REPLY_CSQ;
				
				DEBUG(3,"--- CSQ --- %s",uartrx1_buffer);
                DEBUG(2,"--- CSQ --- \r\n");

				///开辟缓存回传信号强度  上报上行数据
				///memcpy(buffer, UART_RX_DATA2.USART_RX_BUF+UART_RX_DATA2.USART_RX_Len-18, 5);
			}
			
			///CGREG 
			if ( Len>=31 && strncmp(uartrx1_buffer+Len-19, "+CGREG:", 7) == 0 ) 
			{
				if(strstr(uartrx1_buffer,"0,1")!=NULL || strstr(uartrx1_buffer,"0,5")!=NULL)
				commandid_reply = REPLY_CGREG;
				DEBUG(2,"--- CGREG --- \r\n");
			}
			///NETOPEN
			if ( Len>=9 && Len<16 && strstr(uartrx1_buffer, "+NETOPEN:") != NULL ) 
			{
				if(uartrx1_buffer[Len-3] == '0')
				commandid_reply = REPLY_NETOPEN;
				else
					commandid_reply = REPLY_NETOPEN_ERROR;				
				DEBUG(2,"--- NETOPEN --- \r\n");
			}
			
			///NETCLOSE：手动关闭
			else if ( Len>=16 && strstr(uartrx1_buffer, "+NETCLOSE: 0") != NULL ) 
			{
				DEBUG(2,"---- NETCLOSE --- %c\r\n", uartrx1_buffer[Len-3]);
				commandid_reply = REPLY_NETCLOSE;
			}
			
			///CIPOPEN ----联网成功 ///通道0~9随机数产生
            if ( Len>=18 && (strstr(uartrx1_buffer, "CONNECT") != NULL) ) 
			{
				DEBUG(2,"---- CIPOPEN --- \r\n");
				commandid_reply = REPLY_CIPOPEN;
			}
			
			///CIPOPEN ----联网失败
			else if ( Len>=66 && strncmp(uartrx1_buffer+Len-24, "+CIPOPEN:", 9) == 0) 
			{						
				if(uartrx1_buffer[Len-12] == '4')
				{
					commandid_reply = REPLY_CIPOPEN_ERROR;
					DEBUG(2,"---- CIPOPEN_ERROR --- %c\r\n",uartrx1_buffer[Len-24]);
				}
				else
				commandid_reply = REPLY_ERROR;	
			}	///CIPOPEN ----联网失败
//            else if ( Len>=26 && (strstr(uartrx1_buffer, "+CIPOPEN: 0,1") != NULL)) 
//            {
//                commandid_reply = REPLY_CIPOPEN_ERROR;
//            }
			
			///REPLY_CIPCLOSE ---- 关闭联网 
			if ( Len>=18 && (strstr(uartrx1_buffer, "+CIPCLOSE") != NULL) ) 
			{
				
				if(uartrx1_buffer[Len-3] == '0')
				{
					DEBUG(2,"---+CIPCLOSE:---\r\n");
					commandid_reply = REPLY_CIPCLOSE;
				}
			}
			
			///CIPSEND ---数据发送完成状态
			if (strstr(uartrx1_buffer, "+CIPSEND:")!=NULL ) 
			{
				DEBUG(2,"CIPSEND\r\n");
				commandid_reply = REPLY_SEND_DONE;
			}
			
//			///飞行模式
//			 if ( strstr(uartrx1_buffer, "+SIMCARD: NOT AVAILABLE")!=NULL ) 
//			{
//				commandid_reply = REPLY_INCFUN;
//			}
//			
//			///退出飞行模式
//		     if ( strstr(uartrx1_buffer, "PB DONE")!=NULL ) 
//			{
//				commandid_reply = REPLY_OUTCFUN;
//			}					
					
			///网络被关闭 +IPCLOSE: 0,1//非透传模式， 透传模式 CLOSED
			if ( Len>=10 && strstr(uartrx1_buffer, "CLOSED")!=NULL ) 
			{
				DEBUG(2,"REPLY_CIPCLOSE\r\n");
				commandid_reply = REPLY_CIPCLOSE;			
			}					
		}
		
		else if(Len>=40 && uartrx1_buffer[Len-1]=='\n') ///非透传模式：接收数据'\n'结尾
		{		
			commandid_reply = REPLY_REVER;						
			if (strstr(uartrx1_buffer,"RECV FROM:") != NULL) ///ok
			{
				char *p = strstr(uartrx1_buffer,"+IPD23");
				
				char data = p-&uartrx1_buffer[0];
												
				DEBUG(2,"--RECV---%s",&uartrx1_buffer[data+strlen("+IPD23\r\n")]);///有效数据
			}		
		}
	}	
}
