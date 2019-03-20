/*
**************************************************************************************************************
*	@file	gpio.c
*	@author Jason_531@163.com
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#ifndef __GPIO_H__
#define __GPIO_H__

#include "board.h"


/********************Commation IO******************/
#define  LORA_IO				    GPIOB
#define  LORA_REST_PIN		        GPIO_PIN_0
#define  LORA_POWER_ON		        GPIO_PIN_13
#define  LORA_DIO0				    GPIO_PIN_1
#define  LORA_DIO1                  GPIO_PIN_2
#define  LORA_DIO2                  GPIO_PIN_10

#define  LORA_LED			        GPIOB
#define  LORA_LED_PIN			    GPIO_PIN_15

/********************DEBUG UART******************/
#define  USART1_IO				    GPIOA
#define  USART1_TX				    GPIO_PIN_9
#define  USART1_RX				    GPIO_PIN_10

/********************GPS UART******************/
#define  USART2_IO				    GPIOA
#define  USART2_TX				    GPIO_PIN_2
#define  USART2_RX				    GPIO_PIN_3

/********************GPS Power******************/
#define  GPS_IO					    GPIOB
#define  GPS_Power_ON               GPIO_PIN_12

/********************485 UART******************/
#define  USART4_IO				    GPIOC
#define  USART4_TX				    GPIO_PIN_10
#define  USART4_RX				    GPIO_PIN_11

/**************485收发切换开关**************/
#define Out_485_DE_Pin_Pin          GPIO_PIN_15
#define Out_485_DE_Pin_GPIO_Port    GPIOA

/**********************12V电源开关：485供电****************************/
#define Out_12V_ON_Pin_Pin          GPIO_PIN_3
#define Out_12V_ON_Pin_GPIO_Port    GPIOB

/***********************485IC电源IO***************************/
#define POWER_485IC_Pin             GPIO_PIN_14
#define POWER_485IC_Port            GPIOB

/**************************充电IC异常中断IO*****************************/
#define In_CH_INT_Pin_Pin           GPIO_PIN_0
#define In_CH_INT_Pin_GPIO_Port     GPIOA

/**************************充电IC使能IO*****************************/
#define Out_CH_CE_Pin_Pin           GPIO_PIN_6
#define Out_CH_CE_Pin_GPIO_Port     GPIOC

/**************************充电IC状态IO*****************************/
#define IN_CH_STATE1_GPIO_Port      GPIOC
#define IN_CH_STATE1_Pin            GPIO_PIN_8
#define IN_CH_STATE2_GPIO_Port      GPIOC
#define IN_CH_STATE2_Pin            GPIO_PIN_9
#define IN_CH_PG_GPIO_Port          GPIOC
#define IN_CH_PG_Pin                GPIO_PIN_7


#define RS485_TO_TX()	 HAL_GPIO_WritePin(Out_485_DE_Pin_GPIO_Port,Out_485_DE_Pin_Pin, GPIO_PIN_SET);	// RS485总线切换到发送模式
#define RS485_TO_RX()	 HAL_GPIO_WritePin(Out_485_DE_Pin_GPIO_Port,Out_485_DE_Pin_Pin, GPIO_PIN_RESET);// RS485总线切换到接收模式


/*!
 * \brief GPIO初始化
 * \param SX1276 RESET引脚初始化---PB13
 * \retval None
 */
void SX1276GPIO_Init(void);

void SenSor_Close(void);

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

void _rs485_openpin(int index);

void _rs485_clsoepin(void);

void Rs485Pin_Init(void);


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
