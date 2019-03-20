/*
**************************************************************************************************************
*	@file	parse_hex_file.h
*	@author 
*	@version 
*	@date    
*	@brief	解析hex程序文件，并烧写到FLASH
***************************************************************************************************************
*/
#ifndef __PARSE_HEX_FILE
#define __PARSE_HEX_FILE

#include "stm32l0xx_hal.h"
#include "bootloader_config.h"
#include "user_flash_L072.h"
#include <string.h>
#include <stdio.h>
#ifdef __cplusplus
	extern "C" {
#endif


// IAP状态
#define IAP_FAILURE	   -1	// IAP失败，基本就是数据下载错误
#define IAP_CONTINUE	0	// IAP进行中
#define IAP_SUCCESS		1	// IAP成功结束
#define IAP_UNKNOWN_CMD	2	// 非IAP命令
#define IAP_TIMEOUT		3	//超时失败
//#define IAP_DOWN_ERROR	4	//下载数据错误

// hex文件处理结构体
#define HEX_DATA_LEN    45 //一行最少45字符(包括\r\n)
typedef struct
{
	uint32_t linear_base_addr;	// Linear Base Address
	uint32_t empty_addr;		// 目前已经擦除的最高地址
	int32_t  status;			// IAP状态
	uint16_t vct_offset;		//中断向量表的动态偏移(a区b区不一样)
	uint8_t Start;
}
HexObjFile_t;	

/*
 *	InitHexObjFile:	初始化Hex文件结构体
 *	p：				HexObjFile结构体
 *	返回值：		无	
 */
extern void InitHexObjFile(HexObjFile_t *p);

/*
 *	ParseHexObjFile:	解析Hex文件并下载
 *	p_hex：				HexObjFile结构体
 *	str:				字符串行，以'\0'或者'\n'结尾
 *	返回值：			0--成功 -1--失败				
 */
extern int8_t ParseHexObjFile(HexObjFile_t *p_hex, char *str);

/*
 *	ShowFlash:			将Flash中的计数器显示出来
 *	参数：				无
 *	返回值：			无				
 */
extern void ShowFlash(void);
/*
 *	GetURL:				将Flash中的URL获取出来，同时将相关参数配置好
 *	data:				下载http文件的命令
 *	server:				URL中的服务器网址
 *	返回值：			1--成功 -1--失败				
 */
extern int8_t GetURL(char *data,char *server);
#ifdef __cplusplus
}
#endif

#endif
