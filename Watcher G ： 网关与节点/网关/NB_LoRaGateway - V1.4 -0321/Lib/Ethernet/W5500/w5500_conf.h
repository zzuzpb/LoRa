#ifndef _W5500_CONF_H_
#define _W5500_CONF_H_

#include "stm32f2xx_hal.h"
#include "stdio.h"
#include "types.h"
#include "debug.h"

#define ON										1
#define OFF										0
#define HIGH									1
#define LOW										0
	
#define CONFIG_MSG_LEN        sizeof(CONFIG_MSG) - 4 // the 4 bytes OP will not save to EEPROM
#define MAX_BUF_SIZE					1460
#define KEEP_ALIVE_TIME	      30	// 30sec
// SRAM address range is 0x2000 0000 ~ 0x2000 BFFF (48KB)
#define SOCK_BUF_ADDR 				0x20000000
#define AppBackAddress        0x08020000 //from 128K
#define ConfigAddr						0x0800FC00
#define NORMAL_STATE          0
#define NEW_APP_IN_BACK       1 //there is new app in back address
#define CONFIGTOOL_FW_UP      2 //configtool update f/w in app

#define HIGH	           	 			1
#define LOW		             			0

#define ON	                 		1
#define OFF	                 		0

#define FW_VER_HIGH   	1
#define FW_VER_LOW    	0

#define W550OPEN          GPIO_PIN_RESET
#define W550CLOSE         GPIO_PIN_SET

extern uint8 txsize[MAX_SOCK_NUM];		// 选择8个Socket每个Socket发送缓存的大小，在w5500.c的void sysinit()有设置过程
extern uint8 rxsize[MAX_SOCK_NUM];

#pragma pack(1)
typedef struct _CONFIG_MSG
{
  uint8 op[4];//header: FIND;SETT;FACT...
  uint8 mac[6];
  uint8 sw_ver[2];
  uint8 lip[4];
  uint8 sub[4];
  uint8 gw[4];
  uint8 dns[4];	
  uint8 dhcp;
  uint8 debug;

  uint16 fw_len;
  uint8 state;
  
}CONFIG_MSG;
#pragma pack()
extern CONFIG_MSG  ConfigMsg, RecvMsg;

/*W5500SPI相关函数*/
uint16 wiz_write_buf(uint32 addrbsb,uint8* buf,uint16 len);	/*向W5500写入len字节数据*/
uint16 wiz_read_buf(uint32 addrbsb, uint8* buf,uint16 len);	/*从W5500读出len字节数据*/

/*W5500基本配置相关函数*/
void reset_w5500(void);		
																	/*硬复位W5500*/
void W550Power(uint8_t State);

void wiz_cs(uint8_t val);

uint8_t SPI_SendByte(uint8_t byte);

void set_w5500_mac(void);

void set_network(void);

void set_default(void);														// 设置默认MAC、IP、GW、SUB、DNS
    

#endif
