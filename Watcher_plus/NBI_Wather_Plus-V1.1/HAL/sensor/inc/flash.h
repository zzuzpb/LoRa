#ifndef __FLASH_H_
#define __FLASH_H_
#include "stm32l0xx_hal.h"


/* 类型定义 ------------------------------------------------------------------*/
/* 宏定义 --------------------------------------------------------------------*/
/************************** STM32 内部 FLASH 配置 *****************************/
#define STM32_FLASH_SIZE        192  // 所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN        1    // stm32芯片内容FLASH 写入使能(0，禁用;1，使能)

#define FLASH_ALINE    4

#if FLASH_ALINE == 2
  #define FLASH_TYPE uint16_t //字节
#else 
  #define FLASH_TYPE uint32_t
#endif

/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/
void STMFLASH_Read_AllWord( uint32_t ReadAddr, uint32_t *pBuffer, uint32_t NumToRead );
uint16_t STMFLASH_ReadHalfWord ( uint16_t faddr );
uint32_t STMFLASH_ReadAllWord ( uint32_t faddr );
HAL_StatusTypeDef STMFLASH_Write( uint32_t WriteAddr, FLASH_TYPE * pBuffer, uint32_t NumToWrite );		//从指定地址开始写入指定长度的数据
void STMFLASH_Read( uint32_t ReadAddr, uint16_t * pBuffer, uint32_t NumToRead );   	//从指定地址开始读出指定长度的数据
HAL_StatusTypeDef STMFLASH_WriteStruct(uint32_t WriteAddr, void * pBuffer, uint16_t NumToWrite);
void STMFLASH_ReadStruct(uint32_t WriteAddr, void * pBuffer, uint16_t NumToWrite);
extern HAL_StatusTypeDef STMFLASH_writeString(uint32_t WriteAddr, void * pBuffer, uint16_t NumToWrite);
void STMFLASH_ReadString(uint32_t ReadAddr, char* pBuffer, uint16_t NumToRead);
#endif

