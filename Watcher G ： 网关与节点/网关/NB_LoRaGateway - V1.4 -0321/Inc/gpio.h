/**
  ******************************************************************************
  * File Name          : gpio.h
  * Description        : This file contains all the functions prototypes for 
  *                      the gpio  
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
#ifndef __gpio_H
#define __gpio_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_hal.h"
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
	 
#define  LORA_IO					GPIOB
#define  LORA_REST_PIN		GPIO_PIN_0
#define  LORA_POWER_ON		GPIO_PIN_12
#define  LORA_DIO0				GPIO_PIN_1
#define  LORA_DIO1        GPIO_PIN_2
#define  LORA_DIO2        GPIO_PIN_10


#define  LORA_LED					GPIOA
#define  LORA_LED_PIN			GPIO_PIN_15


#define  GPS_IO						GPIOB
#define  GPS_Power_ON     GPIO_PIN_7


#define  UART_485_IO			GPIOB
#define  UART_485_DE			GPIO_PIN_5

#define  POWER_IO					GPIOA
#define  POWER_12V_ON     GPIO_PIN_8

#define  USART1_IO				GPIOA
#define  USART1_TX				GPIO_PIN_9
#define  USART1_RX				GPIO_PIN_10

#define  USART2_IO				GPIOA
#define  USART2_TX				GPIO_PIN_2
#define  USART2_RX				GPIO_PIN_3

#define  USART5_IO				GPIOB
#define  USART5_TX				GPIO_PIN_3
#define  USART5_RX				GPIO_PIN_4

#define  I2C2_IO					GPIOB
#define  I2C2_SCL					GPIO_PIN_13
#define  I2C2_SDA					GPIO_PIN_14

#define  GPS_IO						GPIOB
#define  GPS_IO_PIN				GPIO_PIN_7

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief GPIO初始化
 * \param SX1276 RESET引脚初始化---PB13
 * \retval None
 */
void SX1276GPIO_Init(void);

/**
  * @brief GPIO初始化
  * @param  LoRa电源控制  HOST_2_Lora_DFU_EN引脚初始化---PB12 
  * @retval None
  */
void LoRaPower_Init(void);

/*!
 * \brief GPIO IRQ Initialization
 *
 */
void SX1276EXTI_Init(void);


/*!
 * \brief Writes the given value to the GPIO output
 *
 * \param [IN] obj   Pointer to the GPIO object
 * \param [IN] value New GPIO output value
 */
void GpioWrite( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState );

/*!
 * \brief Toggle the value to the GPIO output
 *
 * \param [IN] obj   Pointer to the GPIO object
 */
void GpioToggle( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin );

/*!
 * \brief Reads the current GPIO input value
 *
 * \param [IN] obj Pointer to the GPIO object
 * \retval value   Current GPIO input value
 */
uint32_t GpioRead( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin );


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ pinoutConfig_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
