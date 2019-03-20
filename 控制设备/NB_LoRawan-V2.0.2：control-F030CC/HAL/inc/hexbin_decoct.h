/*
**************************************************************************************************************
*	@file	hexbin_decoct.h
*	@author Ysheng
*	@version 
*	@date    
*	@brief  进制转换函数
***************************************************************************************************************
*/

#ifndef __HEXBIN_DECOCT__H
#define __HEXBIN_DECOCT__H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


int Convert16To10(int number);

uint32_t Read_DecNumber(char *str);

void String_Conversion(char *str, uint8_t *src, uint8_t len);

#endif 
