#include "user_main.h"
#include "user_sim.h"
#include "user_led.h"
#include "user_adc.h"
#include "user_flash_L072.h"
#include "user_sensor_pro2.h"
#include "user_gps.h"
#include "user_server.h"
#include "user_battery.h"
#include "user_spi_flash.h"
#include "time.h"

#define         DEVICEID            0xc2201300

extern UART_HandleTypeDef hlpuart1;	//GPRS
extern UART_HandleTypeDef huart1;	//调试
//extern UART_HandleTypeDef huart2;	//GPS
//extern UART_HandleTypeDef huart5;	//485
extern RTC_HandleTypeDef hrtc;
extern IWDG_HandleTypeDef hiwdg;
extern TIM_HandleTypeDef htim2;     //喂狗和切换LED时间

char Uart1ReChar[1]={0};
char Uart1ReBuff[128]={0};
uint8_t Uart1ReCount=0;

char TimeNow[20]                    ={0};   //保存的时间当前时间
char SendData[127+1]	            ={0};	//最大长度为127个字符，留一个给发送结束符0x0A
char ServerAdd[SERVER_SIZE]	        ={0};	//服务器地址
char ServerPort[SERVER_PORT_SIZE]   ={0};	//端口地址
char DeviceID[DEVICE_ID_SIZE]	    ={0};	//设备ID
char Stream[STREAM_SIZE]		    ={0};	//数据流

uint32_t    G_htim2TimeOut;                 //定时器2的超时时间
uint32_t    G_SendAddr;                     //指向已经发送了的数据的最大地址
uint32_t    G_SaveAddr;                     //未发送的数据保存在SPIflash中的地址

T_SensorData sensor_data[2]={0};			//电量、光照度、空气温湿度加上5个传感器数据，最多有8个数据

/*
 * 获取当前系统RTC时间，
*/
void RtcGetTime(char *str)
{
    RTC_DateTypeDef Date;
    RTC_TimeTypeDef Time;
	HAL_RTC_GetTime(&hrtc, &Time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &Date, RTC_FORMAT_BIN);//规定格式
    sprintf(str,"20%02d-%02d-%02d %02d:%02d:%02d",\
    Date.Year,Date.Month,Date.Date, Time.Hours, Time.Minutes, Time.Seconds);
	DEBUG("开机时间:%s\r\n",str);
}


/*
 * 保存数据
*/
void SaveSensorData(uint8_t *Bat)
{    
    G_SaveAddr = FlashRead32(SAVE_FLASH_ADD);
//    G_SaveAddr = 8200;
    if(G_SaveAddr==0xFFFFFFFF)
    {
        G_SaveAddr = 0;
    }
    uint32_t temp =sizeof(sensor_data)+sizeof(TimeNow)+1;//要写入的字节数
    if(G_SaveAddr+temp>SPI_FLASH_SIZE||G_SaveAddr==0)
    {
        DEBUG("擦除首页\r\n");
        SpiFlashSectorErase(0*4096);
        G_SaveAddr = 0;
    }
    uint32_t a= G_SaveAddr / SPI_FLASH_SECTOR_SIZE;
    
    uint32_t b= (G_SaveAddr+temp) / SPI_FLASH_SECTOR_SIZE;
    
    DEBUG("a= %d, b=%d,G_SaveAddr=%d, %d\r\n", a,b,G_SaveAddr,(G_SaveAddr+temp));
    if(a<b)//数据跨页
    {
        SpiFlashSectorErase(b*4096);
        DEBUG("数据跨页\r\n");
    }
    
    DEBUG("11G_SaveAddr:%u len : %d \r\n",G_SaveAddr, sizeof(sensor_data));
    
    for(uint8_t i = 0; i < 2; i++)
    {
        for(uint8_t j = 0; j < 2; j++)
        {
            DEBUG("0x%04x ",sensor_data[i].data[j]);
        }
    }
    DEBUG("\r\n");
    
    uint32_t DeviceID = 0;
    DeviceID = SpiFlashReadDeviceID( );    
    printf("DeviceID: 0x%08x\r\n", DeviceID);
    if(DeviceID == DEVICEID)
    {       
        SpiFlashBufferWrite((uint8_t*)sensor_data,G_SaveAddr,sizeof(sensor_data));
        G_SaveAddr +=(sizeof(sensor_data));
        DEBUG("22G_SaveAddr:%u\r\n",G_SaveAddr);
        SpiFlashBufferWrite(Bat,G_SaveAddr,1);
        G_SaveAddr +=1;
        
        DEBUG("33G_SaveAddr:%u\r\n",G_SaveAddr);
        DEBUG("---time:%s\r\n",TimeNow);
        SpiFlashBufferWrite((uint8_t*)TimeNow,G_SaveAddr,sizeof(TimeNow));
        G_SaveAddr +=(sizeof(TimeNow));
        DEBUG("44G_SaveAddr:%u\r\n",G_SaveAddr);
        FlashWrite32(SAVE_FLASH_ADD,&G_SaveAddr,1);
    }
}
/*
 * 若存在未发送的数据，返回1，否则返回0
*/
uint8_t ReadOldData(uint8_t *Bat)
{
    G_SendAddr = FlashRead32(SEND_FLASH_ADD);
    
    if(G_SendAddr==0xFFFFFFFF)
    {
        G_SendAddr = 0;
    }
//    G_SendAddr = 8200;
    if(G_SaveAddr!=G_SendAddr)
    {
        DEBUG("111G_SendAddr:%u\r\n",G_SendAddr);
        uint32_t temp =sizeof(sensor_data)+sizeof(TimeNow)+1;//要读出的字节数，必须与写入的相同
        if(G_SendAddr+temp>SPI_FLASH_SIZE)
        {
            G_SaveAddr = 0;
        }
        
        DEBUG("有数据需要发送到服务器: len : %d ",sizeof(sensor_data));
        SpiFlashBufferRead((uint8_t*)sensor_data,G_SendAddr,sizeof(sensor_data));
        
        for(uint8_t i = 0; i < 2; i++)
        {
            for(uint8_t j = 0; j < 2; j++)
            {
                DEBUG("0x%04x ",sensor_data[i].data[j]);
            }
        }
        DEBUG("\r\n");
        G_SendAddr += sizeof(sensor_data);
        
        DEBUG("222G_SendAddr:%d\r\n",G_SendAddr);
        SpiFlashBufferRead(Bat,G_SendAddr,1);
        DEBUG("Bat:%d\r\n",*Bat);
        G_SendAddr += 1;
        DEBUG("333G_SendAddr:%d\r\n",G_SendAddr);
        SpiFlashBufferRead((uint8_t*)TimeNow,G_SendAddr,sizeof(TimeNow));
        DEBUG("TimeNow:%s\r\n",TimeNow);
        G_SendAddr += sizeof(TimeNow);
        DEBUG("444G_SendAddr:%d\r\n",G_SendAddr);
        //FlashWrite32(SEND_FLASH_ADD,&G_SendAddr,1);//需要在发送成功的情况下才把值更新
        return 1;
    }
    else
    {
        return 0;
    }
}


static uint32_t simsevercount = 0;
void UserMain(void)
{
	uint8_t Battery;
	uint16_t len=0;

	uint32_t seq_send;
    uint32_t seq_ctrl;
	int8_t ret;
        
    //HAL_UART_Receive_IT(&huart1,(uint8_t *)Uart1ReChar,1);//重新开启uart1的中断使能
    
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
    {
        DEBUG("定时器2启动失败\r\n");	            //用于喂狗
        HAL_NVIC_SystemReset();	                //直接软件复位
    }
    G_htim2TimeOut=0;
	RtcGetTime(TimeNow);
	if(ReadSerialNumber(DeviceID,Stream,ServerAdd,ServerPort)!=0)
        HAL_NVIC_SystemReset();

    if(BatInit()==0)
        HAL_NVIC_SystemReset();
    else
    {   
        BatEnableCharge();
        HAL_Delay(1000);                        //电容充电,此时获取电池电量是不准确的
        BatCheck(&Battery);
        if(Battery==0)
        {
            LedChangeState(E_LED_ERROR);
            HAL_Delay(3000);                    //灯闪烁
            return;                             //进入休眠
        }
    }

    //启动并进入初始化，读取传感器数据过程,LED常亮
    LedChangeState(E_LED_INIT);
    
    SensorInit();
	SernsorGetData(sensor_data);
    
//    DEBUG("--111温度: %d, 湿度: %d\r\n",sensor_data[1].data[0],sensor_data[1].data[1]);
    //保存数据到外部flash
    SaveSensorData(&Battery);
//    DEBUG("--222温度: %d, 湿度: %d\r\n",sensor_data[1].data[0],sensor_data[1].data[1]);
    //联网过程
    LedChangeState(E_LED_DISCONNECT);
        
//    while(1);

 #if 1
    
	if(SimInit()==0)
    {
		DEBUG("Sim Init fail\r\n");
        LedChangeState(E_LED_ERROR);    //闪灯3秒后休眠
        SimPowerOff();					//关闭电源，实测需要5秒
		HAL_Delay(3000);
		return;
	}
    else
		DEBUG("Sim Init OK\r\n");

    simsevercount=FlashRead32(START_COUNT_ADD);
    
    simsevercount ++;
    
    FlashWrite32(START_COUNT_ADD,&simsevercount,1);
    
    printf("----- simsevercount : %d\r\n",simsevercount);
    
//    if(simsevercount>=10)
    {
        if(SimConnectServer(ServerAdd,ServerPort)==0)
        {
            DEBUG("Connect Server fail\r\n");
        }
        else
        {
            DEBUG("Connect Server OK\r\n");
            LedChangeState(E_LED_CONNECT);
            
            DEBUG("--333温度: %d, 湿度: %d\r\n",sensor_data[1].data[0],sensor_data[1].data[1]);

            while(ReadOldData(&Battery))
            {
                DEBUG("--444温度: %d, 湿度: %d\r\n",sensor_data[1].data[0],sensor_data[1].data[1]);

                //设备ID、数据流、6个485传感器(mini的无，所以全为0)、电池电量
                len=0;
    //            DEBUG("Battery : %d\r\n",Battery);
                len += sprintf(SendData+len,"D%s,stream=%s,data=%02x000000000000%02x",DeviceID,Stream,STRUCT_VERSION,Battery);
    //            DEBUG("SendData : %s\r\n",SendData);

                //然后拼接传感器数据
                for(uint8_t i=0;i<2;i++)
                {
    //                DEBUG("sensor_data[i].count : %d\r\n",sensor_data[i].count);
                    for(uint8_t j=0;j<2;j++)
                    {
    //                    DEBUG("line : %d\r\n",__LINE__);
                        len += sprintf(SendData+len,"%04x",sensor_data[i].data[j]);
                        DEBUG("senddata : %d, 0x%04x\r\n",i, sensor_data[i].data[j]);
                    }
                }
                
                DEBUG("无GPS模块\r\n");
                //len += sprintf(SendData+len,"|null|%s",TimeNow);
                len += sprintf(SendData+len,"|null");

                seq_send=FlashRead32(SEND_SEQ);
                if(seq_send==0)	//不发送0
                    seq_send=1;
                len += sprintf(SendData+len,",seq=%u\n",seq_send);
                if(CheckFlag(DOWNLINK_CONFIRM))		//有下行数据需要确认，把确认消息拼接在后面
                {
                    char name[CTRL_NAME_SIZE]={0};
                    FlashRead16More(CTRL_NAME,(uint16_t *)name,CTRL_NAME_SIZE/2+1);
                    seq_ctrl=FlashRead32(RECIEVE_SEQ)  ;

                    len += sprintf(SendData+len,"n=%s,seq=%u,state=%u\n",name,seq_ctrl,FlashRead32(CTRL_STATE));//seq收到什么就回什么
                    if(FlashRead16(CTRL_STATE)==1)	//命令执行失败(目前只有更新命令才会执行失败)，需要把保存的seq修改一下,防止服务器重发指令时不执行。
                    {
                        seq_ctrl +=1;
                        FlashWrite32(RECIEVE_SEQ,&seq_ctrl,1);
                    }
                    //DEBUG("Reply:%s",SendData);
                }
                DEBUG("%s\r\n",SendData);
            
                for(uint8_t i=0;i<2;i++)
                {
                    if(SimSendData(SendData,len)==0)
                    {
                        DEBUG("Send data Fail\r\n");
                    }
                    else
                    {
                        DEBUG("Send data OK\r\n");
                        FlashWrite32(SEND_FLASH_ADD,&G_SendAddr,1);//需要在发送成功的情况下才把值更新
                        ret=ServerAck(&seq_send);
                        //Flash操作应放在接收服务器数据之后，因为写Flash时会上锁（__HAL_LOCK()），导致服务器数据的丢包
                        uint32_t temp=seq_send+1;
                        FlashWrite32(SEND_SEQ,&temp,1);     //发送的seq自增1
                        if(CheckFlag(DOWNLINK_CONFIRM))		//有下行数据需要确认
                            CleanFlag(DOWNLINK_CONFIRM);	//清除确认标志
                        if(ret==0)//服务器回复成功
                        {
                            break;
                        }
                        else if(ret==3)
                        {
                            char *ch=strstr(SendData,",seq=");
                            seq_send++;
                            sprintf(ch,",seq=%u\n",seq_send);
                            DEBUG("SendData:%s",SendData);
                        }
                    }
                }
                ret=ServerDowmsteam(&seq_ctrl);
                if(ret!=-1)
                {
                    uint32_t a=ret;
                    FlashWrite32(CTRL_STATE,&a,1);
                }
                memset(SendData,0,sizeof(SendData));
                memset(TimeNow,0,sizeof(TimeNow));
            }
            LedChangeState(E_LED_SENDOK);                   //数据发送完成
            SimPowerOff();
            HAL_Delay(1000);
        }
    }
	if(FlashRead32(SAMPLE_PERIOD_ADDR)==0||FlashRead32(SAMPLE_PERIOD_ADDR)==0xffffffff)
	{
		uint32_t time=300;//默认300秒，加上发送过程大概20秒
		FlashWrite32(SAMPLE_PERIOD_ADDR,&time,1);
	}
	if(CheckFlag(SOFT_RESET))
	{
		CleanFlag(SOFT_RESET);
		DEBUG("复位\r\n");
		HAL_Delay(2000);
		HAL_NVIC_SystemReset();
	}
    
#endif    
/**************传感器数据获取测试*************/
}

/*
 *ResendToSim:将串口接收到的数据转发给SIM模块
*/
//void ResendToSim(void)
//{
//	Uart1ReBuff[Uart1ReCount++]=Uart1ReChar[0];		//保存接收到的数据
//	//DEBUG("%c",Uart1ReChar[0]);

//	if ( Uart1ReCount>=4 && Uart1ReBuff[Uart1ReCount-2]!='\r' && Uart1ReBuff[Uart1ReCount-1]=='\n' )	// 收到"\r\n"，有新命令回复
//	{
//        Uart1ReBuff[Uart1ReCount]=0;
//        SimRxSvMsgCallback(Uart1ReBuff,Uart1ReCount);
//		Uart1ReCount = 0;
//	}
//	HAL_UART_Receive_IT(&huart1,(uint8_t *)Uart1ReChar,1);//重新开启uart1的中断使能
//}


//extern char *service_ctrl;
//extern uint32_t ServerReply;
//extern char service_ack[64];

//void CheckServerReply(void)
//{
//    Uart1ReBuff[Uart1ReCount++]=Uart1ReChar[0];		//保存接收到的数据
//	//DEBUG("%c",Uart1ReChar[0]);
//
//    if ( Uart1ReCount>=4 && Uart1ReBuff[Uart1ReCount-2]=='\r' && Uart1ReBuff[Uart1ReCount-1]=='\n' )
//    {
//        DEBUG("%s",Uart1ReBuff);
//        char *pt;
//        if (Uart1ReCount>15 && *Uart1ReBuff == 'C')//收到服务器下行控制
//        {
//            pt = strstr(Uart1ReBuff,"$ack@");
//            if(pt!=NULL)
//            {
//                memcpy(service_ctrl,Uart1ReBuff,pt-Uart1ReBuff);
//                ServerReply |=SERVER_REPLY_CTRL;
//            }
//        }
//        if(Uart1ReCount>=16 && (pt = strstr(Uart1ReBuff,"$ack@"))!=NULL )//服务器回复ack
//        {
//            memcpy(service_ack,pt,Uart1ReBuff+Uart1ReCount-pt);
//            ServerReply |= SERVER_REPLY_ACK;
//        }
//    }
//	HAL_UART_Receive_IT(&huart1,(uint8_t *)Uart1ReChar,1);//重新开启uart1的中断使能
//}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(&htim2 == htim)//50ms一次
	{
        G_htim2TimeOut++;
        if(G_htim2TimeOut==20)//1s喂一次
        {
            HAL_IWDG_Refresh(&hiwdg);
            G_htim2TimeOut=0;
        }
        LedDisplay(50);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
	if(UartHandle==&huart1)				//与电脑连接，串口调试
    {
		//ResendToSim();
		//ResendToGps();
        //CheckServerReply();//要用9600
	}
	else if(UartHandle==&hlpuart1)		//sim800模块
	{
		SimCheckReply();
	}
//	else if(UartHandle==&huart2)		//GPS模块
//	{
//		GpsCheckReply();
//	}
//	else if(UartHandle==&huart5)		//Rs485模块
//	{
//		Rs485CheckReply();
//	}
}



