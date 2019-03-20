/**
  ******************************************************************************
  * File Name          : KEY.h
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __KEY_H
#define __KEY_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
	 
#include <stdint.h>	 
	 
#define KEY_PORT		   GPIOB   

#define KEY_A_PIN      	   GPIO_PIN_7
#define KEY_B_PIN          GPIO_PIN_6 
	 
#define WAKEUP_PORT		   GPIOC   	 
#define WAKEUP_PIN         GPIO_PIN_13 	 

#define KEY_ON			   1
#define KEY_OFF			   0
	 
#define PWR_KEY_RCC_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define PWR_KEY_EXTI_IRQHandler          EXTI4_15_IRQHandler
#define PWR_KEY_EXTI_IRQn                EXTI4_15_IRQn

/* 扩展变量 ------------------------------------------------------------------*/
/* 函数声明 ------------------------------------------------------------------*/
void StandbyInit(void);
uint8_t StandbyCheckPwrkey(void);
void StandbyEnterMode(void);
void StandbyInit(void);
void SYSCLKConfig_STOP(void); 
void Key_Init(void);
void Led_Init(void);
void WorkStatusJudgment(void);
void Standy_Io_Mode(void);
	 
	 
#ifdef __cplusplus
}
#endif
#endif /* __KEY_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
