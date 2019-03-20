/*
**************************************************************************************************************
*	@file	gpio.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#include "board.h"

/**
  * @brief GPIO初始化
  * @param SX1276 RESET引脚初始化---PB0  PB14---lora_poweron: 考虑到功耗问题，直接去掉R112  R111电阻，由外部直接供电
  * @retval None
  */
void SX1276GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟
	
    GPIO_Initure.Pin=GPIO_PIN_13; //PB0  
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);	//PB13置1 
}

/**
  * @brief GPIO初始化
  * @param  LoRa状态灯  HOST_2_Lora_DFU_EN引脚初始化---PB15 
  * @retval None
  */
void LoRaLed_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟
	
    GPIO_Initure.Pin=GPIO_PIN_15; //PB15  
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;//GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
    HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);	//PB15置1
}

/**
  * @brief 外部中断初始化
  * @param huart: DIO0--PB1   DIO1--PB2   DIO2--PB10   DIO3--PB11   
  * @retval None
  */
void SX1276EXTI_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();               		 //开启GPIOB时钟

	GPIO_Initure.Pin=GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10|GPIO_PIN_11;  
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      			//上升沿触发
	GPIO_Initure.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);
			
	//中断线2-PC2
	HAL_NVIC_SetPriority(EXTI0_1_IRQn,5,0);       //抢占优先级为2，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);             //使能中断线2
	
	//中断线3-PC3
	HAL_NVIC_SetPriority(EXTI2_3_IRQn,6,0);       //抢占优先级为3，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);             //使能中断线3
	
	//中断线4-PC15
	HAL_NVIC_SetPriority(EXTI4_15_IRQn,7,0);       //抢占优先级为0，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);             //使能中断线9
}


void GpioWrite( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState )
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin,  PinState);
}

void GpioToggle( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin )
{
   HAL_GPIO_TogglePin(GPIOx,  GPIO_Pin);   
}

uint32_t GpioRead( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin )
{
	return HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
}
