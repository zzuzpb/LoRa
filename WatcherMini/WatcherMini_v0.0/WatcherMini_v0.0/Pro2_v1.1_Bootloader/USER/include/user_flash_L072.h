/*
 * user_flashL072.h	L072芯片的flash读写操作驱动头文件
*/

#ifndef __USER_FLASH_L072_H__
#define __USER_FLASH_L072_H__

#include "stm32l0xx_hal.h"

#define FLAG	FlashRead32(FLAG_ADD)		

typedef enum FLAGTYPE						//标志类型,最大值为31
{
	SOFT_RESET=0x00,						//软件复位标志
	UPDATE_RESET,							//更新固件标志
	DOWNLINK_CONFIRM,						//下行确认标志,用于回复服务器的下行控制命令
	CLEAN_FLASH,							//复位所有数据Flash中的数据
	GET_LOCATION,							//获取位置信息
	UPDATE_SUCCEED,
	UPDATE_FAIL,
	END_OF_FLAGTYPE
}FlagType;

/*
 *	FlashReadPage:		读取1页数据
 *	参数PageAddr：		页地址
 *	参数pBuffer：		保存读取到的数据的指针
 *	返回值：			1成功 0失败	
 */
uint8_t FlashReadPage(uint32_t PageAddr, uint32_t *pBuffer);
/*
 *	FlashWritePage:		写1页数据
 *	参数PageAddr：		页地址
 *	参数pBuffer：		用于写入Flash中的数据指针
 *	返回值：			1成功 0失败	
 */
uint8_t FlashWritePage( uint32_t PageAddr, uint32_t *pPageBuffer);

/*
 *	FlashWrite32:		写4字节(32位)数据
 *	参数WriteAddr：		该数据在Flash中的地址
 *	pBuffer:			用于写入Flash中的数据指针
 *	NumToWrite			数据长度(小于页大小/4)
 *	返回值：			1成功 0失败			
 */
uint8_t FlashWrite32( uint32_t WriteAddr, uint32_t * pBuffer, uint16_t NumToWrite );
/*
 *	FlashWrite16:		写2字节(16位)数据
 *	参数WriteAddr：		该数据在Flash中的地址
 *	pBuffer:			用于写入Flash中的数据指针
 *	NumToWrite			数据长度(小于页大小/2)
 *	返回值：			1成功 0失败			
 */
uint8_t FlashWrite16( uint32_t WriteAddr, uint16_t * pBuffer, uint16_t NumToWrite );

/*
 *	FlashRead32:		读取4字节(32位)数据
 *	参数ReadAddr：		该数据在Flash中的地址
 *	返回值：			返回读取到的数据		
 */
uint32_t FlashRead32(uint32_t ReadAddr );
/*
 *	FlashRead16:		读取2字节(16位)数据
 *	参数ReadAddr：		该数据在Flash中的地址
 *	返回值：			返回读取到的数据		
 */
uint16_t FlashRead16(uint32_t ReadAddr );

/*
 *	FlashRead8:		读取1字节(8位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	返回值：		返回读取到的数据		
 */
uint8_t FlashRead8(uint32_t ReadAddr );
/*
 *	FlashReadChar:	读取字节(8位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	参数pBuffer：	保存读取到的数据的指针
 *	参数NumToRead：	读取的长度
 *	返回值：		实际读取到的字符长度,读取错误返回0
 */
uint16_t FlashReadChar(uint32_t ReadAddr,char* pBuffer,uint16_t NumToRead);
/*
 *	FlashRead16More:	读取多个2字节(16位)数据
 *	参数ReadAddr：	该数据在Flash中的地址
 *	参数pBuffer：	保存读取到的数据的指针
 *	参数NumToRead：	读取的长度
 *	返回值：		1成功 0失败
 */
uint16_t FlashRead16More(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);
/*
 *	FlashIncrease32:	将Flash中以32位储存的计数器自增1
 *	参数：				该计数器在Flash中的地址
 *	返回值：			1--成功 0--失败				
 */
uint8_t FlashIncrease32(uint32_t add);
/*
 *	FlashIncrease16:	将Flash中以16位储存的计数器自增1
 *	参数：				该计数器在Flash中的地址
 *	返回值：			1--成功 0--失败				
 */
uint8_t FlashIncrease16(uint32_t add);

/*
 *	SetFlag:			修改FLAG_ADD地址的值，设置相关标志位
 *	参数flag：			只能为FlagType类型
 *	返回值：			无	
 */
void SetFlag(FlagType flag);

/*
 *	CleanFlag:			修改FLAG_ADD地址的值，清除相关标志位
 *	参数flag：			只能为FlagType类型
 *	返回值：			无	
 */
void CleanFlag(FlagType flag);

/*
 *	CheckFlag:			查询相关标志位是否被标志
 *	参数flag：			只能为FlagType类型
 *	返回值：			被标志返回>0的值，未标志返回0
 */
uint8_t CheckFlag(FlagType flag);

#endif /* __USER_FLASH_L072_H__ */


