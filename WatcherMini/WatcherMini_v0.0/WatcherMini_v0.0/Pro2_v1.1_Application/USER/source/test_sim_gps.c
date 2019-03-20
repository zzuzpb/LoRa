#include "user_main.h"
#include "user_bq24195.h"
#include "user_sim.h"
#include "user_led.h"
#include "user_adc.h"
#include "user_flash_L072.h"
#include "user_sensor_pro2.h"
#include "user_gps.h"
#include "bootloader_config.h"
#include <string.h>

extern UART_HandleTypeDef hlpuart1;	//GPRS
extern UART_HandleTypeDef huart1;	//调试
extern UART_HandleTypeDef huart2;	//GPS
extern UART_HandleTypeDef huart5;	//485

extern char SimReChar[1];

void UserMain(void)
{
    double Latitude,Longitude;
    uint8_t ret;
    uint32_t timeout=3000;
    
    printf("检测Sim800模块和GPS模块\r\n\r\n");
    
    if(GpsInit()==0)
        printf("GPS模块：错误error\r\n\r\n");
        
    LedOpen();//提示开始定位
    ret=GpsGetLocation(&Longitude,&Latitude,&timeout);
    
    if(ret==1)
    {
        printf("GPS模块：OK\r\n\r\n");
    }
    else
    {
        if(ret==2)
        {
            printf("GPS模块：无GPS模块？？？GPS模块错误？？？\r\n\r\n");
        }
        else
        {
            printf("GPS模块：正常OK\r\n\r\n");
        }
    }
    GpsPowerOff();
    SimPowerOn();
	HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)SimReChar,1);//打开串口接收中断,此时中断会接收到一个字符
	if(SimExecuteCmd(AT_CMD_CLOSE_ECHO)!=AT_OK)
    {
		printf("Sim800模块：错误error\r\n\r\n");
	}
    else
	{
        printf("Sim800模块：正常OK\r\n\r\n");
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if(UartHandle==&huart1)				//与电脑连接，串口调试
    {
		//ResendToSim();
		//ResendToGps();
	}
	else if(UartHandle==&hlpuart1)		//sim800模块
	{
		SimCheckReply();
	}
	else if(UartHandle==&huart2)		//GPS模块
	{
		GpsCheckReply();
	}
	else if(UartHandle==&huart5)		//Rs485模块
	{
		Rs485CheckReply();
	}
}
