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

char Uart1ReChar[1]={0};
char Uart1ReBuff[256]={0};
uint8_t Uart1ReCount=0;

#define STRUCT_VERSION	3			//通信数据版本号

void UserMain(void)
{
//	printf("~!@#$%^&*()_+|}{:?><zxcvbnm12345你好\r\n");
	
/*************充电测试**************///测试通过
//	Bq24195EnableCharge();
//	HAL_Delay(5000);
//	Bq24195DisableCharge();
//	HAL_Delay(5000);
//	Bq24195EnableCharge();
//	for(uint8_t i=0;i<10;i++)
//		AnalysisRegisiter(i);
/*************充电测试**************/

/*************串口转发**************///测试通过
//	if(HAL_UART_Receive_IT(&huart1,(uint8_t *)Uart1ReChar,1)!=HAL_OK)//打开串口接收中断,用于转发指令到sim模块
//		return ;
//	if(SimPowerOn()==0)
//		return ;
//	extern char SimReChar[1];
//	if(HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)SimReChar,1)!=HAL_OK)//打开串口接收中断
//		return ;
/*************串口转发**************/

/****************GPS测试***********/	/*测试通过*/
//	uint8_t index=1,count=0;
//while(1)
//{
//	double Latitude;	//纬度
//	double Longitude;	//经度
//	uint32_t time=300000;//3分钟超时
//	char data[50]={0};
////	if(HAL_UART_Receive_IT(&huart1,(uint8_t *)Uart1ReChar,1)!=HAL_OK)//打开串口接收中断,用于转发指令到GPS模块
////		return;

//	if(GpsGetLocation(&Longitude,&Latitude,&time)==0)
//	{
//		//定位失败
//		sprintf(data,"%u,%u,%f,%f,%u\r\n",index,count,Longitude,Latitude,time);
//		if(SimInit()==0)
//			printf("Sim Init fail\r\n");
//		else
//			printf("Sim Init OK\r\n");
//		if(SimConnectServer("47.90.33.7","8933")==0)
//			printf("Connect Server fail\r\n");
//		else
//			printf("Connect Server OK\r\n");
//		
//		if(SimSendData(data)==0)
//			printf("Send data Fail\r\n");
//		else
//			printf("Send data OK\r\n");
//		GpsPowerOff();
//		HAL_Delay(1000);
////		HAL_NVIC_SystemReset();
//	}
//	else
//	{//定位成功
//		sprintf(data,"index:%u\r\ncount:%u\r\nLongitude:%f\r\nLatitude:%f\r\ntime:%u\r\n", index,count,Longitude,Latitude,time);
//		if(SimInit()==0)
//			printf("Sim Init fail\r\n");
//		else
//			printf("Sim Init OK\r\n");
//		if(SimConnectServer("47.90.33.7","8933")==0)
//			printf("Connect Server fail\r\n");
//		else
//			printf("Connect Server OK\r\n");
//		
//		if(SimSendData(data)==0)
//			printf("Send data Fail\r\n");
//		else
//		printf("Send data OK\r\n");
//		SimPowerOff();
//		HAL_UART_AbortReceive_IT(&hlpuart1);
//		HAL_Delay(10000);//延时10秒再次定位重发服务器
//	}
//	count++;
//}
/****************GPS测试***********/

/**************ADC电压采集*************///测试通过
//	float ADC0Voltage,ADC1Voltage;
//	char data[64]={0};
//	if(AdcInit()==0)
//		printf("AdcInit Fail\r\n");
//	else
//		//while(1)
//		{
//			ADC0Voltage=AdcGetVoltage(ADC_CHANNEL_0)*2;
//			ADC1Voltage=AdcGetVoltage(ADC_CHANNEL_1)*2;
//			printf("ADC0 Voltage:%f\r\n",ADC0Voltage);
//			printf("ADC1 Voltage:%f\r\n",ADC1Voltage);
//			HAL_Delay(2000);
//		}
/**************ADC电压采集*************/

/************服务器连接测试***********///测试通过
//	if(SimInit()==0)
//		printf("Sim Init fail\r\n");
//	else
//		printf("Sim Init OK\r\n");
//	if(SimConnectServer("47.90.33.7","8933")==0)
//		printf("Connect Server fail\r\n");
//	else
//		printf("Connect Server OK\r\n");
//	
//	if(SimSendData("~！@#￥%……&*（）——+|：？》《~!@#$%^&*()_+|}{:?0123456789.qwertyuiopasdfghjklzxcvbnmTest\r\n")==0)
////	sprintf(data,"ADC0Voltage:%f\r\nADC1Voltage:%f\r\n",ADC0Voltage,ADC1Voltage);
////	printf("%s",data);
////	if(SimSendData(data)==0)
//		printf("Send data Fail\r\n");
//	else
//		printf("Send data OK\r\n");
/************服务器连接测试***********/

/**************电磁阀测试*************/	//测试通过
//	_12VPowerOn();
//	HAL_Delay(1000);
//	SolenoidOpen();
//	HAL_Delay(5000);
//	SolenoidClose();
//	_12VPowerOff();
/**************电磁阀测试*************/
	
/**************Flash读写测试*************/	//测试通过
//	uint32_t FLASH_BUF [ FLASH_PAGE_SIZE/4 ];
//	for(uint8_t i=0;i<FLASH_PAGE_SIZE/4;i++)
//		FLASH_BUF[i]=i+1;
//	FlashWrite32(0x0801fd10,FLASH_BUF,FLASH_PAGE_SIZE/4);
//	for(uint8_t i=0;i<FLASH_PAGE_SIZE/4;i++)
//		printf("%x:%x\r\n",0x0801fd10+i*4,FlashRead32(0x0801fd10+i*4));
//	uint16_t FLASH_BUF [ FLASH_PAGE_SIZE/2 ];
//	for(uint16_t i=0;i<FLASH_PAGE_SIZE/2;i++)
//		FLASH_BUF[i]=i+1;
//	FlashWrite16(0x08010000,FLASH_BUF,FLASH_PAGE_SIZE/2);
//	for(uint8_t i=0;i<FLASH_PAGE_SIZE/2;i++)
//		printf("%x:%x\r\n",0x08010000+i*2,FlashRead16(0x08010000+i*2));

//SetFlag(SOFT_RESET);//下行控制标志位
//CleanFlag(SOFT_RESET);
//SetFlag(UPDATE_RESET);//下行控制标志位
//CleanFlag(UPDATE_RESET);
//SetFlag(DOWNLINK_CONFIRM);//下行控制标志位
//CleanFlag(DOWNLINK_CONFIRM);
//SetFlag(CLEAN_FLASH);//下行控制标志位
//CleanFlag(CLEAN_FLASH);
//SetFlag(GET_LOCATION);//下行控制标志位
//CleanFlag(GET_LOCATION);
//SetFlag(UPDATE_SUCCEED);//下行控制标志位
//CleanFlag(UPDATE_SUCCEED);
//SetFlag(UPDATE_FAIL);//下行控制标志位
//CleanFlag(UPDATE_FAIL);
//SetFlag(END_OF_FLAGTYPE);//下行控制标志位
//CleanFlag(END_OF_FLAGTYPE);

/**************Flash读写测试*************/

/**************传感器数据获取测试*************/	//测试通过
	char SendData[128]	={0};				//256太长，堆栈不够，128足够了
	char ServerAdd[SERVER_SIZE]	={0};		//服务器地址
	char ServerPort[SERVER_PORT_SIZE]={0};	//端口地址
	char DeviceID[DEVICE_ID_SIZE]	={0};	//设备ID
	char Stream[STREAM_SIZE]		={0};	//数据流
	int32_t Battery;
	uint16_t len=0;
	SensorData sensor_data[8]={0};			//电量、光照度、空气温湿度加上5个传感器数据，最多有8个数据
	double Latitude,Longitude;
	uint32_t seq_send;
	uint32_t seq_ctrl;
	int8_t ret;
    
    printf("整板测试\r\n");
    
	if(ReadSerialNumber(DeviceID,Stream,ServerAdd,ServerPort)!=0)
    {
        printf("设备序列号读取失败\r\n");
        LedFlash(10,0);//死循环
    }
	printf("Device:%s\r\n",DeviceID);
	printf("Stream:%s\r\n",Stream);
	printf("ServerAdd:%s\r\n",ServerAdd);
	printf("ServerPort:%s\r\n",ServerPort);
    
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
    
	for(uint8_t i=0;i<3;i++)
	{
		LedOpen();
		HAL_Delay(200);
		LedClose();
		HAL_Delay(200);
	}
	seq_ctrl=FlashRead32(RECIEVE_SEQ);
	if(CheckFlag(DOWNLINK_CONFIRM))		//有下行数据需要确认，把确认消息拼接在后面
	{
		char name[CTRL_NAME_SIZE]={0};
		FlashRead16More(CTRL_NAME,(uint16_t *)name,CTRL_NAME_SIZE/2+1);  
		len += sprintf(SendData+len,"n=%s,seq=%u,state=%u\n",name,FlashRead32(RECIEVE_SEQ),FlashRead32(CTRL_STATE));//seq收到什么就回什么
		if(FlashRead16(CTRL_STATE)==1)	//命令执行失败(目前只有更新命令才会执行失败)，需要把保存的seq修改一下,防止服务器重发指令时不执行。
		{
			seq_ctrl +=1;
			FlashWrite32(RECIEVE_SEQ,&seq_ctrl,1);
		}
		printf("Reply:%s",SendData);
		CleanFlag(DOWNLINK_CONFIRM);	//清楚确认标志
	}
	
	//D171001TEST123,stream=PRO2_V3_170930,data=030000000000005900001ab7011b01ee,seq=2
	//D171001TEST123,stream=PRO2_V3_170930,data=030000000000005900001ab7011b01ee,seq=2
	len += sprintf(SendData+len,"D%s,stream=%s,data=",DeviceID,Stream);
	
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
	
	//如果有需要,再把位置信息拼上去
    if(DeviceID[strlen(DeviceID)-1]=='G')//表示带GPS模块
    {
        printf("有GPS模块\r\n");
        if(CheckFlag(GET_LOCATION)||__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))//接收到后台命令，或者上电复位，则进行定位操作
        {
            uint32_t TimeOut=300000;	//5分钟超时
            uint8_t ret;
            if(__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
                __HAL_RCC_CLEAR_RESET_FLAGS();//清除复位标志
        
            if(GpsInit()==0)
                printf("Gps Init fail\r\n");
            else
                printf("Gps Init OK\r\n");	
            LedOpen();//提示开始定位
            ret=GpsGetLocation(&Longitude,&Latitude,&TimeOut);
            if(ret==1)
            {
                printf("GpsGetLocation succeed:%u\r\n",TimeOut);
                LedClose();//提示定位成功
                len += sprintf(SendData+len,"|%.6f_%.6f",Latitude,Longitude);
            }
            else
            {
                if(ret==2)
                {
                    printf("模块错误\r\n");
                    len += sprintf(SendData+len,"|error");
                }
                else
                {
                    printf("定位失败\r\n");
                    len += sprintf(SendData+len,"|0_0");
                }
            }
            GpsPowerOff();
            CleanFlag(GET_LOCATION);	//清楚标志位
            
            /*******************************设置上报GPS位置的时间间隔*********************************/
            if(FlashRead16(SP_GPS_ADDR)==0||FlashRead16(SP_GPS_ADDR)==0xffff)
            {
                //uint16_t temp1=60*60;					//一小时，此处以秒为单位
                uint16_t temp1=60*1;					//一分钟，此处以秒为单位
                FlashWrite16(SP_GPS_ADDR,&temp1,1);
            }
            //SetRtcAlarm(FlashRead16(SP_GPS_ADDR));
            /*******************************设置上报GPS位置的时间间隔*********************************/
        }
        else//不定位
        {
            printf("不定位,保持上一次的定位状态\r\n");
            len += sprintf(SendData+len,"|keep");//保持上一次的定位状态
        }
    }
    else
    {
        printf("无GPS模块\r\n");
        len += sprintf(SendData+len,"|null");
    }
//	Latitude=113.949819;
//	Longitude=-22.550605;
//	len += sprintf(SendData+len,"|%.6f_%.6f",Latitude,Longitude);
	//最后将seq拼上去
	seq_send=FlashRead32(SEND_SEQ);
	if(seq_send==0)	//不发送0
		seq_send=1;
	len += sprintf(SendData+len,",seq=%u\r\n",seq_send);
	
	printf("SendData:%s",SendData);
	if(SimInit()==0)
    {
		printf("Sim Init fail\r\n");
        HAL_NVIC_SystemReset();
	}
    else
	{
        printf("Sim Init OK\r\n");
	}
	//if(SimConnectServer("47.90.33.7","8933")==0)
	if(SimConnectServer(ServerAdd,ServerPort)==0)
    {
		printf("Connect Server fail\r\n");
        HAL_NVIC_SystemReset();
    }
    else
    {
		printf("Connect Server OK\r\n");
	}
    
	for(uint8_t i=0;i<2;i++)
	{
		if(SimSendData(SendData)==0)
			printf("Send data Fail\r\n");
		else
		{
			printf("Send data OK\r\n");
			if(i==0)
			{
				uint32_t temp=seq_send+1;
				FlashWrite32(SEND_SEQ,&temp,1);//发送的seq自增1
			}
			if(IsAck(DeviceID,Stream,&seq_send)==0)//服务器回复成功
			{
				break;
			}
		}
	}
	
	ret=IsControl(DeviceID,Stream,&seq_ctrl);
	if(ret!=-1)
	{
		uint32_t a=ret;
		FlashWrite32(CTRL_STATE,&a,1);
	}
	
	if(FlashRead32(SAMPLE_PERIOD_ADDR)==0||FlashRead32(SAMPLE_PERIOD_ADDR)==0xffffffff)
	{
//		uint32_t time=300;//默认300秒
		uint32_t time=60;//默认300秒
		FlashWrite32(SAMPLE_PERIOD_ADDR,&time,1);
	}
	
	if(CheckFlag(SOFT_RESET))
	{
		CleanFlag(SOFT_RESET);
		printf("复位\r\n");
		HAL_Delay(100);
		HAL_NVIC_SystemReset();
	}
	
/**************传感器数据获取测试*************/
}

/*
 *ResendToSim:将串口接收到的数据转发给SIM模块
*/
void ResendToSim(void)
{
	Uart1ReBuff[Uart1ReCount++]=Uart1ReChar[0];		//保存接收到的数据
	printf("%c",Uart1ReChar[0]);
	
	if ( Uart1ReCount>=4 && Uart1ReBuff[Uart1ReCount-2]=='\r' && Uart1ReBuff[Uart1ReCount-1]=='\n' )	// 收到"\r\n"，有新命令回复
	{
		HAL_UART_Transmit(&hlpuart1,(uint8_t *)Uart1ReBuff,Uart1ReCount,0xffff);
		Uart1ReCount = 0;
	}
	HAL_UART_Receive_IT(&huart1,(uint8_t *)Uart1ReChar,1);//重新开启uart1的中断使能
}

void ResendToGps(void)
{
	Uart1ReBuff[Uart1ReCount++]=Uart1ReChar[0];		//保存接收到的数据
	//printf("%02x",Uart1ReChar[0]);
	
	//if ( Uart1ReCount>=4 && Uart1ReBuff[Uart1ReCount-2]=='\r' && Uart1ReBuff[Uart1ReCount-1]=='\n' )	// 收到"\r\n"，有新命令回复
	{
		HAL_UART_Transmit(&huart5,(uint8_t *)Uart1ReBuff,Uart1ReCount,0xffff);
		Uart1ReCount = 0;
	}
	HAL_UART_Receive_IT(&huart1,(uint8_t *)Uart1ReChar,1);//重新开启uart1的中断使能
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



