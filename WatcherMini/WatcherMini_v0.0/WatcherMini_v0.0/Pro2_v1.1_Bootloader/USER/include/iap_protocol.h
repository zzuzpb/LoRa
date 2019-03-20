/*
**************************************************************************************************************
*	@file	iap_protocol.h
*	@author 
*	@version 
*	@date    
*	@brief	在线升级In-Application-Programming协议
***************************************************************************************************************
*/
#ifndef __IAP_PROTOCOL_H
#define __IAP_PROTOCOL_H

#include "stm32l0xx_hal.h"
#include "parse_hex_file.h"
#include "bootloader_config.h"
#include <string.h>

#ifdef __cplusplus
	extern "C"{
#endif


/*
 *	InitGprsIAP:	初始化GPRS在线编程
 *	参数：			无
 *	返回值：		无	
 */
extern void InitGprsIAP(void);

/*
 *	UpdateProgramFromGprs:	通过GPRS在线升级程序
 *	参数：					无
 *	返回值：				无	
 */
extern int32_t UpdateProgramFromGprs(void);


/*
 *	HandleIAPData:	处理IAP数据
 *	p：				命令缓冲区指针
 *	len:			数据长度
 *	返回值：		无
 */
extern void HandleIAPData(char *p, uint32_t len);

/*
 *	GetIAPStatus:	获取IAP状态
 *	返回值：		IAP状态
 *					IAP_FAILURE    --IAP失败
 *					IAP_CONTINUE   --IAP进行中
 *					IAP_SUCCESS	   --IAP成功结束
 * 					IAP_UNKNOWN_CMD--非IAP命令
 */
extern int32_t GetIAPStatus(void);
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
