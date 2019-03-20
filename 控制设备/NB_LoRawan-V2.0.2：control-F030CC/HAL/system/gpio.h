/*
**************************************************************************************************************
*	@file	gpio.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#ifndef __GPIO_H__
#define __GPIO_H__

//#include "pinName-board.h"
//#include "pinName-ioe.h"
#include "board.h"

/*!
 * \brief GPIO初始化
 * \param SX1276 RESET引脚初始化---PB13
 * \retval None
 */
void SX1276GPIO_Init(void);

/**
  * @brief GPIO初始化
  * @param  LoRa状态灯  HOST_2_Lora_DFU_EN引脚初始化---PB15 
  * @retval None
  */
void LoRaLed_Init(void);

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

#endif // __GPIO_H__
