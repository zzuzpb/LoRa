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

#define STRUCT_VERSION	3			//通信数据版本号

extern UART_HandleTypeDef hlpuart1;	//GPRS
extern UART_HandleTypeDef huart1;	//调试
extern UART_HandleTypeDef huart2;	//GPS
extern UART_HandleTypeDef huart5;	//485

void UserMain()
{
    char SendData[128]	={0};				//256太长，堆栈不够，128足够了
    SensorData sensor_data[8]={0};			//电量、光照度、空气温湿度加上5个传感器数据，最多有8个数据
    	uint16_t len=0;
    	int32_t Battery;
        
    	if(AdcInit()==0)
    {
		printf("AdcInit Fail\r\n");
        HAL_NVIC_SystemReset();
    }
    else
    {
		AdcGetBattery(&Battery);
        if(Battery==0)
        {
            printf("电量低休眠\r\n");
            return;
        }
	}
    
    _12VPowerOn();
	SensorInit();
	SernsorGetData(sensor_data);
	_12VPowerOff();

    	len += sprintf(SendData+len,"%02X",STRUCT_VERSION);
	//先把传感器的地址拼上去
	for(uint8_t i=0;i<sizeof(sensor_data)/sizeof(sensor_data[0]);i++)
	{
//		printf("sensor_data[%u].index:%u\r\n",i,sensor_data[i].index);
//		printf("sensor_data[%u].add:%u\r\n",i,sensor_data[i].add);
//		printf("sensor_data[%u].count:%u\r\n",i,sensor_data[i].count);
//		printf("sensor_data[%u].data[0]:%x\r\n",i,sensor_data[i].data[0]); 
//		printf("sensor_data[%u].data[1]:%x\r\n",i,sensor_data[i].data[1]);
		if(i>1)
			len += sprintf(SendData+len,"%02x",sensor_data[i].add);
	}
	
	//再拼上电量
	len += sprintf(SendData+len,"%02x",Battery);
	//然后拼接传感器数据
	for(uint8_t i=0;i<sizeof(sensor_data)/sizeof(sensor_data[0]);i++)
	{
		for(uint8_t j=0;j<sensor_data[i].count;j++)
		{
			len += sprintf(SendData+len,"%04x",sensor_data[i].data[j]);
		}
	}
    printf("data=%s\r\n",SendData);
    
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
