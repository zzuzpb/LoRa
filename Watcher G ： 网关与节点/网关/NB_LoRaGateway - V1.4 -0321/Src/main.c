/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f2xx_hal.h"
#include "dma.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

#include "board.h"
#include "etimer.h"
#include "autostart.h"
#include "sys/mt.h"
#include "sim7600.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/


/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/*!***********************************???D?¤??************************************/

#if  OVER_THE_AIR_ACTIVATION

extern uint8_t DevEui[8];
static uint8_t AppEui[] = LORAWAN_APPLICATION_EUI;
static uint8_t AppKey[] = LORAWAN_APPLICATION_KEY;

extern TimerEvent_t JoinReqTimer;
extern volatile bool IsNetworkJoined;
extern bool JoinReq_flag;

#endif

LoRaMacRxInfo *loramac_rx_info;
mac_evt_t loramac_evt;


void app_mac_cb (mac_evt_t evt, void *msg)
{
    switch(evt){
    case MAC_STA_TXDONE:                
    case MAC_STA_RXDONE:
    case MAC_STA_RXTIMEOUT:
    case MAC_STA_ACK_RECEIVED:
    case MAC_STA_ACK_UNRECEIVED:
    case MAC_STA_CMD_JOINACCEPT:         
    case MAC_STA_CMD_RECEIVED:
         loramac_rx_info = msg;   
         loramac_evt = evt;
         
         break;
    }
}


/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

extern TimerEvent_t SleepTimer;

void OnSleepTimerTimerEvent( void )
{
	DEBUG(2,"%s\r\n",__func__);

	TimerStop( &SleepTimer );	
	TimerSetValue( &SleepTimer, 5000 );
	TimerStart( &SleepTimer );
}

PROCESS(Netsend_process,"Netsend_process");
PROCESS(NetReceive_process,"NetReceive_process");
PROCESS(NetProtect_process,"NetProtect_process");
AUTOSTART_PROCESSES(&NetProtect_process,&Netsend_process); //

static process_event_t LoRaReceiveDone;

void Netsend_post(void)
{
    process_post(&Netsend_process,LoRaReceiveDone,NULL);
}

/*
*RFTXDONE：设置发送完成优先级
*参数：    无
*返回值：  无
*/
void RFTXDONE(void)
{
	process_poll(&Netsend_process); ///设置进程优先级，响应发送完成
}

PROCESS_THREAD(NetReceive_process,ev,data)
{
	static struct etimer et;
	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System SX1278Receive Process..."); 
	etimer_set(&et,CLOCK_SECOND*40);
	while(1)
	{
		PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
		DEBUG(2,"hello world\r\n");
        
        ///接收服务器下行控制数据命令：服务器要求在A类模式下必须有下行数据时才能下行，否则数据锁死等待有上行才下行
        ///要求网络数据帧分为两种A/C类作为服务器判断类型使用
        ///LoRaMacDevAddr == 下行设备ID：
        ///user_app_send 发送数据注意：CLASS A开启A窗口只要1S时间，需要处理好延时，否则回出现node接收失败
        
        ///广播数据处理，LoRaMacDevAddr为特殊地址：只针对C类
		etimer_reset(&et);
	}
	
	PROCESS_END();
}

extern bool Send_Ack;

/*
*Function:NetProtect Process：网络异常保护机制
*Input Paragramed:
*Output Paragramed:
*Remarks:
*Note: 4G模块不需要添加心跳包机制，硬件心跳包有效/以太网模块功能待测试
*/
PROCESS_THREAD(NetProtect_process,ev,data) 
{
    static struct etimer timer;
    
    static uint32_t SimcomTimer = 0;
	
	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System NetProtect Process..."); 
    	
	etimer_set(&timer,CLOCK_SECOND*1);
    while(1)
	{	
		PROCESS_YIELD();
		
		if(ev == PROCESS_EVENT_TIMER) ///网络异常保护机制
		{	 
            if(InternetMode == LAN) ///以太网监测
            {
                DEBUG(2,"----InternetMode----LAN\r\n");    
                do_tcp_client(  );  ///以太网发送数据 
            }
            else if(InternetMode == SIMCOM) ///4G模式监测
            {
                DEBUG(2,"----InternetMode----SIMCOM\r\n");   
                ///网络被断开，再次初始化网络 REPLY_NONE
                if(commandid_reply == REPLY_CIPCLOSE || commandid_reply == REPLY_ERROR || commandid_reply == REPLY_NONE) ///服务器断开/上电后联网服务器不在线
                {
                    DEBUG(2,"SimcomConnectServer\r\n"); 
                    SimcomConnectServer( );
                }
                uint8_t dhcpret = check_DHCP_state(SOCK_DHCP);
                if(DHCP_RET_UPDATE == dhcpret)
                    InternetMode = LAN;    
            }                

            etimer_reset(&timer);
        }        
	}
	PROCESS_END();
}

/*
*Function:Netsend Process: 4G/以太网发送函数
*Input Paragramed:
*Output Paragramed:
*Remarks:
*/
PROCESS_THREAD(Netsend_process,ev,data)
{
	static struct etimer timer;
    static uint8_t  buf[250] = {0};
    static uint8_t  len = 0;

	PROCESS_BEGIN();
	
	USR_UsrLog("Contiki System Netsend Process..."); 
    LoRaReceiveDone = process_alloc_event();
        	
	while(1)
	{	
		PROCESS_YIELD();

        if(ev == LoRaReceiveDone)///异步机制
        {  
            DEBUG(2,"----reply111----%d %d\r\n",Send_Ack,Net_Buffers.Receive);
            
            /**********************4G网络发送数据***************************/
            if(InternetMode == SIMCOM)
            {
                if(commandid_reply == REPLY_CIPOPEN)
                {
                   ///接收LORa数据，进行封包处理：异步信号处理模式                                            
                    DEBUG(2,"----reply0123----%d\r\n",commandid_reply); 
                    SpliceSend(buf, len);
                    SimcomSendData((char *)buf,len);
                    memset(buf, 0, strlen((char *)buf));
                    HAL_Delay(100);
                }
            }
            /**********************以太网发送数据***************************/
            else if(InternetMode == LAN)
            {
                uint16_t SR_Data = getSn_SR(SOCK_TCPC);
                
                if(SOCK_ESTABLISHED == SR_Data)
                {
                    SpliceSend(buf, len);
                                        
                    send(SOCK_TCPC,(uint8_t *)buf,strlen((char *)buf));
                    DEBUG(2," send....……%s\r\n",buf);
                    memset(buf, 0, strlen((char *)buf));   
                }                    
            }
            
            if(Net_Buffers.Receive)  ///服务器下发数据：4G、以太网
            {
               HAL_Delay(1000); ///end node时间匹配：可能需要更改
               Net_Buffers.Receive = false;
               ///LoRaMacDevAddr == 下行设备ID：
//               user_app_send(UNCONFIRMED, "helloworld", strlen("helloworld"), 2);接收数据处理
            }
            else if(!Net_Buffers.Receive && Send_Ack)///非服务器下行
            {
                HAL_Delay(1000); ///end node时间匹配
                Send_Ack = false;               
                DEBUG(2,"----reply222----%d\r\n",commandid_reply);
                if(LoRapp_SenSor_States.loramac_evt_flag)
                {
                    user_app_send(UNCONFIRMED, "helloworld", strlen("helloworld"), 2);                                   
                    DEBUG(2,"----reply333----%d\r\n",commandid_reply);
                    LoRapp_SenSor_States.loramac_evt_flag = 0;                                        
                }           
            }
        }    
	}
	PROCESS_END();
}

/*******************************************************************************
  * @函数名称	main
  * @函数说明   主函数 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无

	版本说明：
	【1】：V1.0.1：MCU---stm32F02，网关WG;

	优化功能：
	【1】： 实现LORAWAN与网关通信。
	【2】： SPI2 --- TX  SPI3 --- RX
    【3】： 收发数据格式、状态独立分开，解决长期工作收发异常现象
    【4】： 增加信道规划：TX对应node RX1 = RX2+200e3
    【5】： 以太网IP：server_ip

	
  *****************************************************************************/
/* variable functions ---------------------------------------------------------*/	

/* USER CODE END 0 */

int main(void)
{

    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* USER CODE BEGIN Init */

    BoardInitMcu(  );

    /* USER CODE END Init */

    /* USER CODE BEGIN SysInit */

    user_app_init(app_mac_cb);

    LoRaMacTestRxWindowsOn( false ); ///关闭接收窗口

    LoRaMacChannelAddFun(  );

    Channel = 6; ///获取信道ID 4G模块获取

    LoRaCad.Iq_Invert = true;  ///使能节点间通信

    ReportTimerEvent = true;
    LoRapp_SenSor_States.loramac_evt_flag = 1;

    LoRapp_SenSor_States.AT_PORT = randr( 1, 0xDF );
    Net_Buffers.SensorBuf = (uint8_t *)malloc(sizeof(uint8_t)*64); ///使用指针必须分配地址空间，否则会出现HardFault_Handler错误
    Net_Buffers.Device_Id = (char *)malloc(sizeof(char)*16); 

    /* USER CODE END SysInit */

    /* Initialize interrupts */
    MX_NVIC_Init();

    /* USER CODE BEGIN 2 */
      
    clock_init();

    TimerInit( &ReportTimer, OnReportTimerEvent );

    LoRaMacSetDeviceClass( CLASS_C );

    Systick_Init(100);

    reset_w5500();                     /* W5500硬件复位 */  
    printf("verson: %02x\r\n", IINCHIP_READ(VERSIONR)); ///verson==4 则SPI正常 0x003900而不是0x0039否则读取失败

    SimcomPower(OPEN);
  /*******************************4G、以太网选择：默认以太网模式*****************************/
  
    DEBUG(2,"-----Init Ethernet-----\r\n");
    set_w5500_mac();
    init_dhcp_client();	

    uint32_t timeover = HAL_GetTick();
    do_dhcp_ip( ); ///需要超时机制，切换为4G初始化.10S超时机制

    DEBUG(2,"W5500 Init Complete! %d\r\n",HAL_GetTick()-timeover);
    timeover = 0;
    
    if(INITSIMDONE == InitSimcom(  ))
    {
        DEBUG(2,"-----Init Simcom Done-----\r\n");
        SimcomExecuteCmd(cmds[CTCPKA]); ///设置心跳包
        
        SimcomExecuteCmd(cmds[CIPMODE]);
        
        SimcomOpenNet(  );
    
        SimcomConnectServer(  );
    }
    else
    {
        DEBUG(2,"-----Init Simcom Fail-----\r\n");
    }	
    
    ///03022820TEST0001
    memcpy(Net_Buffers.Device_Id, "03022820TEST0002", 16); ///设备ID
    Net_Buffers.Versions = VERSIONS;     ///版本号
    Net_Buffers.Type = TYPE;  
    Net_Buffers.Command  = COMMANDS;      ///数据指令

    DEBUG(2,"Net_Buffers.Device_Id: %s\r\n",Net_Buffers.Device_Id);
        
    process_init();
    process_start(&etimer_process,NULL); ///自动包含下面的线程
    autostart_start(autostart_processes);
      
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
     do
    {
    }while(process_run() > 0);
    }
    /* USER CODE END 3 */

}

/** NVIC Configuration
*/
void MX_NVIC_Init(void)
{
  /* TIM2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(TIM2_IRQn);
 
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 8, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
