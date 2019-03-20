/*
**************************************************************************************************************
*	@file	flash.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief  FLASHº¯Êý
***************************************************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_H
#define __FLASH_H

#include <stdint.h>

#define READ_FLASH(faddr)						(*(volatile uint16_t*)faddr) 
#define STMFLASH_ReadWord(faddr)		(*(volatile uint32_t*)faddr) 
	

void STMFLASH_Read(uint32_t ReadAddr,uint16_t *pBuffer,uint16_t NumToRead);

void STMFLASH_Read_Word(uint32_t ReadAddr, uint32_t *pBuffer,uint32_t NumToRead);

#endif /* __FLASH_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
