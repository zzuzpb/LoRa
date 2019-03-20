
#include "adc.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
/* USER CODE BEGIN Includes */
#include "flash.h"
#include "FIFO_Uart.h"
#include "user_bq24195.h"
#include "user_mian.h"
#include "NBI_rs485.h"

uint8_t g_tcp_error_count  										__attribute__((at(NBI_TCP_FAILE_COUNT))) ;

uint8_t g_deviceID[DEVICE_ID_DATA_LEN]         __attribute__((at(DEVICE_ID_ADDRESS))) ;
uint8_t g_version[VERSION_DATA_LEN ]        __attribute__((at(VERSION_ADDRESS)))  ;
uint8_t g_rempteip[REMOTE_IP_DATA_LEN]      __attribute__((at(REMOTE_IP_ADDRESS)))  ;
uint8_t g_remoteport[REMOTE_PORT_DATA_LEN]      __attribute__((at(REMOTE_PORT_ADDRESS)));
uint32_t g_rs485address[NBI_RS485_SEND_BUFF_LEN]         __attribute__((at(RS485_ADDRESS)));

stu_flashData g_flashData = {0};   													//	__attribute__((at(SYS_STU_DATA_ADDRESS)));

uint32_t g_seq = 1;         //全局变量seq


HAL_StatusTypeDef initsystem()
{
	//读取设备ID
	//读取版本号
	//读取ip地址
	//读取port
	//读取开机方式
	//读取重启次数
	int version = 0x01;
	STMFLASH_WriteStruct(DEVICE_ID_ADDRESS,"0101171165290020",16);
	STMFLASH_WriteStruct(VERSION_ADDRESS,&version,4);
	STMFLASH_WriteStruct(REMOTE_IP_ADDRESS,"45.77.178.244",16);
	STMFLASH_WriteStruct(REMOTE_PORT_ADDRESS,"1314",4);
	printf("device:%s\r\n",g_deviceID);
	printf("version:%s\r\n",g_version);
	printf("remoteip:%s\r\n",g_rempteip);
	printf("rempteport:%s\r\n",g_remoteport);
	printf("device = %x\r\n",DEVICE_ID_ADDRESS);
	printf("g_version = %x\r\n",VERSION_ADDRESS);
	
	printf("REMOTE_IP_ADDRESS = %x\r\n",REMOTE_IP_ADDRESS);
	
	printf("REMOTE_PORT_ADDRESS = %x\r\n",REMOTE_PORT_ADDRESS);
	
	printf("NBI_TCP_FAILE_COUNT = %x\r\n",SYS_STU_DATA_ADDRESS);
		
	printf("sys:%d\r\n",__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST));
	printf("sys:%d\r\n",__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST));
	HAL_Delay(1000);
//	STMFLASH_ReadStruct(SYS_STU_DATA_ADDRESS,&g_flashData,sizeof(stu_flashData));
	g_flashData.timeOut = 600;
	if(__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) ||__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST)) //手动或者软件复位
	{		
		//记录rs485的下标   在初始化函数里面实现，此处不做处理		
	}
	else
	{
		if(g_flashData.timeOut == 0)
			g_flashData.timeOut = 6000;
		g_flashData.RS485Flag = HAL_ERROR;
	}
	g_seq = g_flashData.seq;
	return HAL_ERROR;
}


void RtcShowTime(RTC_DateTypeDef *Date,RTC_TimeTypeDef *Time)
{
  HAL_RTC_GetTime(&RtcHandle, Time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&RtcHandle, Date, RTC_FORMAT_BIN);//¹æ¶¨¸ñÊ½
  //printf("¿ª»úÊ±¼ä:%02d:%02d:%02d\r\n",Time->Hours, Time->Minutes, Time->Seconds);
}

void NBI_closeSensorPower()
{
	_12VPowerOff();
//	cleanCommandFlag(NBI_SERVER_FLAG);
}
void NBI_openSensorPower()
{
	_12VPowerOn();
}

void NBI_PWR_EnterSTOPMode_time(int time)
{
	// 0.488ms 
	static int time_index = 0;
	time_index = (int)(time/10);
	if(time_index == 0)
		time_index = 1;
	NBI_closeSensorPower();
	
	printf("start switch stop mode\r\n");
	HAL_Delay(2000);	
	//关闭串口	
	HAL_UART_DeInit(&huart1);
	HAL_UART_DeInit(&huart2);
	HAL_UART_DeInit(&huart4);	
	while(time_index)
	{				
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON,PWR_STOPENTRY_WFE);		
		//
		time_index --;		
	}		
	HAL_Delay(3000);
	//打开串口
	HAL_UART_Init(&huart1);
	HAL_UART_Init(&huart2);
	HAL_UART_Init(&huart4);	
	initFifo();
	printf("had wake up for stop\r\n");
	//HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
	time_index = 0;
	NBI_openSensorPower();
	
}

int NBI_allowAddress(uint8_t *sendData,char addr,int index)
{
	int i = 0;
	if(addr == 0x00)//空气温湿度和光照度
	{
		sendData[i] = 0x01; //温度
		sendData[i+1] = index;
		i+=2;
		sendData[i] = 0x02; //湿度
		sendData[i+1] = index;
		i+=2;
		sendData[i] = 0x03; //光照
		sendData[i+1] = index;
		return i + 2;		
	}
	if(addr == 0x00)//啥啥啥
	{
		//dadsadad
		return 0;
	}
	sendData[i] = addr; //光照
	sendData[i+1] = index;
    DEBUG_NORMAL("addr=%02x,index=%02x connect sensor\r\n",addr,index);
	return i + 2;		
}

static uint8_t g_sendData[100] = {0};
int NBI_sendRs485AddressforServer()
{	
	uint32_t sendLen = 2;
    int k = 0 ;
	for(int i = 0 ; i < NBI_RS485_PIN_COUNT ; i ++)
	{
		if(g_rs485mainportbuff[i].type == RS485_EXPAND_BOX)
		{
			for(k = i * 5; k < (i * 5) + 5; k ++)
			{				
                DEBUG(3,"---expendbox: k = %d\r\n",k);
                if(g_rs485exboxbuff[k].expendboxlive == HAL_OK)
				sendLen += NBI_allowAddress(g_sendData + sendLen,g_rs485exboxbuff[k].data->addr,g_rs485exboxbuff[k].index);  
			}//expend box 		
		}
		else if(g_rs485mainportbuff[i].type == RS485_SIGNAL && g_rs485mainportbuff[i].mainportlive == HAL_OK)
		{
			  sendLen += NBI_allowAddress(g_sendData + sendLen,g_rs485mainportbuff[i].data->addr,g_rs485mainportbuff[i].index); 
		}
	}
	return sendLen;
}

//0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80 0x90 0xA0 0xB0 0xC0 0xD0 0xE0 0xF0
// 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x07 0x08 0x09
//高位代表接口，地位代表扩展盒接口
void NBI_sendRs485DataforServer()
{
	uint32_t sendLen = 0;
	sendLen += NBI_sendRs485AddressforServer(); ///获取485地址、接口ID
	g_sendData[0] = CheckBattery();	//电量％
	g_sendData[1] = (sendLen-2)/2;  // 传感器个数
	
	Rs485_GetData();    //获取rs485传感器的数据   
    
	for(int i = 0 ; i < NBI_RS485_PIN_COUNT ; i ++)
	{
		if(g_rs485mainportbuff[i].type == RS485_EXPAND_BOX)
		{
            for(int k = i * 5 ; k < (i * 5) + 5; k ++)
            {					
                if( g_rs485exboxbuff[k].expendboxlive == HAL_OK )
                {
                    DEBUG(2, "---expendboxlive: %d  ",g_rs485exboxbuff[k].data->regDatalen );
                    for(int j = 0 ;j < g_rs485exboxbuff[k].data->regDatalen; j ++)
                    {                            
                        g_rs485exboxbuff[k].Rs485Data[j] = U2rever(g_rs485exboxbuff[k].Rs485Data[j]);
                        memcpy(g_sendData+sendLen,&(g_rs485exboxbuff[k].Rs485Data[j]), 2); ///数据
                        sendLen += 2;
                        DEBUG(2, "0x%04x ", g_rs485exboxbuff[k].Rs485Data[j]);
                    }
                    DEBUG(2, "\r\n");
                }
            }//expend box 							
		}
		else if(g_rs485mainportbuff[i].type == RS485_SIGNAL && g_rs485mainportbuff[i].mainportlive == HAL_OK)
		{
            DEBUG(2, "---mainboxlive: ");
			for(int j = 0 ;j < g_rs485mainportbuff[i].data->regDatalen ; j ++)
			{							
                g_rs485mainportbuff[i].Rs485Data[j] = U2rever(g_rs485mainportbuff[i].Rs485Data[j]); ///小端转大端
                memcpy(g_sendData+sendLen,&(g_rs485mainportbuff[i].Rs485Data[j]), 2); ///数据
                sendLen += 2;
                DEBUG(2, "0x%04x ", g_rs485mainportbuff[i].Rs485Data[j]);
			}
            DEBUG(2, "\r\n");
		} // signal
	}	
}

