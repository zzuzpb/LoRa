/**
  ******************************************************************************
  * File Name          : main.hpp
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
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
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//#include "debug.h"

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define SIM_PWR_KEY_Pin  GPIO_PIN_4
#define SIM_EN_Pin GPIO_PIN_2
#define SIM_EN_GPIO_Port GPIOE
#define SIM_RESET_Pin GPIO_PIN_3
#define SIM_RESET_GPIO_Port GPIOE
#define SPI1_CNS_Pin GPIO_PIN_4
#define SPI1_CNS_GPIO_Port GPIOA
#define SPI2_CNS_Pin GPIO_PIN_12
#define SPI2_CNS_GPIO_Port GPIOB
#define LoRa1_REST_Pin GPIO_PIN_12
#define LoRa1_REST_GPIO_Port GPIOD
#define LoRa1_EN_Pin GPIO_PIN_13
#define LoRa1_EN_GPIO_Port GPIOD
#define SPI3_CNS_Pin GPIO_PIN_15
#define SPI3_CNS_GPIO_Port GPIOA
#define LoRa2_REST_Pin GPIO_PIN_4
#define LoRa2_REST_GPIO_Port GPIOD
#define LoRa2_EN_Pin GPIO_PIN_5
#define LoRa2_EN_GPIO_Port GPIOD

#define W5500_EN_Pin GPIO_PIN_0
#define W5500_EN_GPIO_Port GPIOB
#define W5500_SCS_Pin GPIO_PIN_4
#define W5500_SCS_GPIO_Port GPIOA
#define W5500_RST_Pin GPIO_PIN_4
#define W5500_RST_GPIO_Port GPIOC
#define W5500_INT_Pin GPIO_PIN_5
#define W5500_INT_GPIO_Port GPIOC
#define W5500_INT_EXTI_IRQn EXTI9_5_IRQn

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);
	 
void MX_NVIC_Init(void);

void RFTXDONE(void);	 
     
void Netsend_post(void);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
