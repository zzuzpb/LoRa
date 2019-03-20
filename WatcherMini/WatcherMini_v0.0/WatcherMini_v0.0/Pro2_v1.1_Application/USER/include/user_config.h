/*
 * user_config.h	配置文件
*/
/**
  ******************************************************************************
  * @file    user_config.h
  * @author  LWY
  * @version V1.0
  * @date    18/01/24
  * @brief   主函数模块，包括一些配置
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
*/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "stm32l0xx_hal.h"
#include <string.h>
#include "bootloader_config.h"

/* 宏定义 --------------------------------------------------------------------*/
/*
__LINE__：在源代码中插入当前源代码行号；
__FILE__：在源文件中插入当前源文件名；
__DATE__：在源文件中插入当前的编译日期
__TIME__：在源文件中插入当前编译时间；
__STDC__：当要求程序严格遵循ANSI C标准时该标识被赋值为1；
__cplusplus：当编写C++程序时该标识符被定义。
*/

#define __DEBUG__

#ifdef __DEBUG__
    /*串口重定向*/
    #ifdef __GNUC__
      #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
    #else
      #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
    #endif
    //#define DEBUG(format,...) printf("%s,%05d\r\n"format, __FILE__,__LINE__, ##__VA_ARGS__)
    #define DEBUG(format,...) printf(format,##__VA_ARGS__)
#else  
    #define DEBUG(format,...)  do{}while(0)
#endif  

#define STRUCT_VERSION	3			        //传感器数据格式

extern char ServerAdd[SERVER_SIZE];         // 服务器IP地址
extern char ServerPort[SERVER_PORT_SIZE];   // 服务器端口
extern char DeviceID[DEVICE_ID_SIZE];       // 设备ID
extern char Stream[STREAM_SIZE];            // 数据流

/*备用服务器*/
#define RESERVE_IP1         "60.205.184.237"
#define RESERVE_PORT1       "1111"

#define RESERVE_IP2         "39.108.76.246"
#define RESERVE_PORT2       "1111"

#endif /* __USER_CONFIG_H__ */
