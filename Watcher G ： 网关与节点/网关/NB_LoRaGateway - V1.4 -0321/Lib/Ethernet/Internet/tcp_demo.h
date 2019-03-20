#ifndef __TCP_DEMO_H
#define __TCP_DEMO_H

#include <stdbool.h>
#include "types.h"

/*Socket 端口选择，可按自己的习惯定义*/
#define SOCK_TCPS             0
#define SOCK_HUMTEM			  0
#define SOCK_PING			  0
#define SOCK_TCPC             1
#define SOCK_UDPS             2
#define SOCK_WEIBO      	  2
#define SOCK_DHCP             3
#define SOCK_HTTPS            4
#define SOCK_DNS              5
#define SOCK_SMTP             6
#define SOCK_NTP              7
//#define NETBIOS_SOCK    6 //在netbios.c已定义

extern bool socket_connet;
extern uint32_t Socketopencounter;

extern uint8 server_ip[4];				// 配置远程服务器IP地址
extern uint16 server_port;

void Systick_Init (uint8_t SYSCLK);
void Delay_ms( uint32_t time_ms );
void Delay_us( uint32_t time_us );

void do_dhcp_ip(void);
void do_tcp_server(void);//TCP Server回环演示函数
void do_tcp_client(void);//TCP Clinet回环演示函数
#endif 

