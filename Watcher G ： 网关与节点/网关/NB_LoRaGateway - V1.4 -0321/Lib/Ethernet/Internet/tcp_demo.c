/**
************************************************************************************************
* @file   		tcp_demo.c
* @author  		WIZnet Software Team 
* @version 		V1.0
* @date    		2015-02-14
* @brief   		TCP 演示函数
* @attention  
************************************************************************************************
**/
#include "stm32f2xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tcp_demo.h"
#include "W5500_conf.h"
#include "w5500.h"
#include "socket.h"
#include "dhcp.h"
#include "user-app.h"

uint32_t Socketopencounter = 0;

static uint32_t  fac_us = 0;//us延时倍乘数

uint32_t seq = 0;

uint8 server_ip[4] = {60,205,184,237};   //{120,77,213,80};   ///{192,168,1,187};   				// 配置远程服务器IP地址
uint16 server_port = 3389;								// 配置远程服务器端口
uint16 local_port = 6000;									// 初始化一个本地端口
uint16 len = 0;

uint8 buffer[2048];	

void Systick_Init (uint8_t SYSCLK)
{
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick频率为HCLK
	fac_us=SYSCLK;	
}								    

void Delay_ms( uint32_t time_ms )
{	 		  	  
	uint32_t i;
	for(i=0;i<time_ms;i++) 
	Delay_us(1000);  	    
}   
//延时nus
//nus为要延时的us数.		    								   
void Delay_us( uint32_t time_us )
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=time_us*fac_us; 						//需要的节拍数 
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	};	 
}

/**
*@brief		do_dhcp_ip获取动态IP函数。
*@param		无
*@return	无
*/
void do_dhcp_ip(void)
{
   uint8 dhcpret=0;
   uint32 dhcptime = HAL_GetTick( );
     
   /* 查询DHCP状态标志位的值
        dhcpret取值：
        0			DHCP_RET_NONE
        1			DHCP_RET_ERR
        2			DHCP_RET_TIMEOUT
        3			DHCP_RET_UPDATE
        4			DHCP_RET_CONFLICT
    */
    do
    {
        dhcpret = check_DHCP_state(SOCK_DHCP);
        DEBUG(2,"dhcpret : %d\r\n",dhcpret);
        
        switch(dhcpret)
        {
            case DHCP_RET_NONE:						// DHCP标志位为空
                 if(HAL_GetTick( ) - dhcptime > 10000) /// 10S超时机制
                 {
                    InternetMode = SIMCOM; ///连接超时，切换为4G mode
                    return;
                 }
                 DEBUG(2,"---DHCP_RET_NONE---\r\n");
            break;
            case DHCP_RET_TIMEOUT:				// DHCP超时
                 DEBUG(2,"---DHCP_RET_TIMEOUT---\r\n");
            break;
            case DHCP_RET_UPDATE:					// 获取了新的IP地址
                    set_default();						// 复制IP地址
                    set_network();						// 设置IP地址
                    InternetMode = LAN;
            break;
            case DHCP_RET_CONFLICT:				// DHCP标志位冲突
                 DEBUG(2,"---DHCP_RET_CONFLICT---\r\n");
            while(1);
            default:
            break;
        }
        HAL_Delay(100);
    } while(dhcpret != STATE_DHCP_LEASED);
   	
}

/**
*@brief		TCP Server回环演示函数。
*@param		无
*@return	无
*/
void do_tcp_server(void)
{	
	uint16 len=0;  
	switch(getSn_SR(SOCK_TCPS))											            	/*获取socket的状态*/
	{
		case SOCK_CLOSED:													                  /*socket处于关闭状态*/
			socket(SOCK_TCPS ,Sn_MR_TCP,local_port,Sn_MR_ND);	        /*打开socket*/
		  break;     
    
		case SOCK_INIT:														                  /*socket已初始化状态*/
			listen(SOCK_TCPS);												                /*socket建立监听*/
		  break;
		
		case SOCK_ESTABLISHED:												              /*socket处于连接建立状态*/
		
			if(getSn_IR(SOCK_TCPS) & Sn_IR_CON)
			{
				setSn_IR(SOCK_TCPS, Sn_IR_CON);								          /*清除接收中断标志位*/
			}
			len=getSn_RX_RSR(SOCK_TCPS);									            /*定义len为已接收数据的长度*/
			if(len>0)
			{
				recv(SOCK_TCPS,buffer,len);								              	/*接收来自Client的数据*/
				buffer[len]=0x00; 											                  /*添加字符串结束符*/
				printf("%s\r\n",buffer);
				send(SOCK_TCPS,buffer,len);									              /*向Client发送数据*/
		  }
		  break;
		
		case SOCK_CLOSE_WAIT:												                /*socket处于等待关闭状态*/
			close(SOCK_TCPS);
		  break;
	}
}

extern bool InternetSend;

static uint8_t connect_count = 0;

/**
*@brief		TCP Client回环演示函数。
*@param		无
*@return	无
*/
void do_tcp_client(void)
{	
    /*Socket状态机，MCU通过读Sn_SR(1)的值进行判断Socket应该处于何种状态
        Sn_SR状态描述：
        0x00		SOCK_CLOSED
        0x13		SOCK_INIT
        0x14		SOCK_LISTEN
        0x17		SOCK_ESTABLISHED
        0x1C		SOCK_CLOSE_WAIT
        0x22		SOCK_UDP
    */
    uint16_t SR_Data = getSn_SR(SOCK_TCPC);
    
    uint8_t buf[256] = {0};
        
    switch(SR_Data)														// 获取SOCK_TCPC的状态
    {
        case SOCK_INIT:															// Socket处于初始化完成(打开)状态
             DEBUG(2,"SOCK_INIT: %02x\r\n",SOCK_INIT);
             uint8_t ret = connect(SOCK_TCPC, server_ip,server_port);			// 配置Sn_CR为CONNECT，并向TCP服务器发出连接请求
             if(ret == 0)///连接中断，切换为4G模式
             {
                connect_count ++;
                if(connect_count>=3)
                {
                    InternetMode = SIMCOM;
                    connect_count = 0;
                }
             }
             DEBUG(2,"connect ret: %02x\r\n",ret);
        break;
        case SOCK_ESTABLISHED:											// Socket处于连接建立状态
                if(getSn_IR(SOCK_TCPC) & Sn_IR_CON)   					
                {
                    DEBUG(2,"%d:Connected to - %d.%d.%d.%d : %d\r\n",SOCK_TCPC, server_ip[0], server_ip[1], server_ip[2], server_ip[3], server_port);
                    setSn_IR(SOCK_TCPC, Sn_IR_CON);								// Sn_IR的CON位置1，通知W5500连接已建立
                }
                // 数据回环测试程序：数据从上位机服务器发给W5500，W5500接收到数据后再回给服务器
                len=getSn_RX_RSR(SOCK_TCPC);										// len=SOCK_TCPC接收缓存中已接收和保存的数据大小
                if(len>0)
                {
                    recv(SOCK_TCPC,buffer,len);										// W5500接收来自服务器的数据，并通过SPI发送给MCU
                    DEBUG(2,"%s\r\n",buffer);							// 串口打印接收到的数据
                    send(SOCK_TCPC,buffer,len);										// 接收到数据后再回给服务器，完成数据回环
                }               
 #if 0  
                if(InternetSend)
                {
                    InternetSend = false;

                    SpliceSend(buf, len);
                                        
                    send(SOCK_TCPC,(uint8_t *)buf,strlen((char *)buf));
                    DEBUG(2," send....……%s\r\n",buf);
                    memset(buf, 0, strlen((char *)buf));
                }
 #endif
        break;
        case SOCK_CLOSE_WAIT:												// Socket处于等待关闭状态
                DEBUG(2,"Socket Server Close\r\n");      ///服务器关闭
                close(SOCK_TCPC);																// 关闭SOCK_TCPC
        break;
        case SOCK_CLOSED:														// Socket处于关闭状态
                DEBUG(2,"Socket Server Nonentity\r\n");   ///服务器不存在或者没开启
                socket(SOCK_TCPC,Sn_MR_TCP,local_port,Sn_MR_ND);		// 打开SOCK_TCPC，并配置为TCP无延时模式，打开一个本地端口
                DEBUG(2,"Socket opened\r\n");              
        break;
    }          
}

