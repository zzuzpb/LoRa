/*
**************************************************************************************************************
*	@file	gpio-board.c
*	@author Ysheng
*	@version 
*	@date    
*	@brief	GPIO
***************************************************************************************************************
*/
#include "board.h"
#include "gpio-board.h"
#include "stm32f0xx.h"
#include "stm32f0xx_hal_gpio.h"

void EXTI0_1_IRQHandler(void){
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
	  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_3_IRQHandler(void)
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
}

void EXTI4_15_IRQHandler(void)
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
		HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_7 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_8 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
	  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_10 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_11 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_12 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_13 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_14 );
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_15 );
}


/*!
 * \brief DIO 0 IRQ callback
 */
extern void SX1276OnDio0Irq( void );

/*!
 * \brief DIO 1 IRQ callback
 */
extern void SX1276OnDio1Irq( void );

/*!
 * \brief DIO 2 IRQ callback
 */
extern void SX1276OnDio2Irq( void );

/*!
 * \brief DIO 3 IRQ callback
 */
extern void SX1276OnDio3Irq( void );

/*!
 * \brief DIO 4 IRQ callback
 */
extern void SX1276OnDio4Irq( void );

/*!
 * \brief DIO 5 IRQ callback
 */
extern void SX1276OnDio5Irq( void );

extern bool test_cad;

/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	中断回调函数：处理中断事件----进行IO判断，处理相应的DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	DEBUG(4, "%s\r\n",__func__);
	switch(GPIO_Pin)
    {
        case GPIO_PIN_1:	
             SX1276OnDio0Irq(  );	

            break;
        case GPIO_PIN_2:
						 SX1276OnDio1Irq(  );

            break;
        case GPIO_PIN_10:
             SX1276OnDio2Irq(  );	
     
            break;

        case GPIO_PIN_11:
             SX1276OnDio3Irq(  );	
    
            break;
        case GPIO_PIN_13:  ///手动模式下为高电平
							if( (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)) == KEY_ON )
							 {
									delay_ms(20);
									
									if( (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)) == KEY_ON )
									{																						
										Control_States.Auto_Mode = false;		
										DEBUG(2,"GPIO_PIN_13_222\r\n");
										///关闭所有继电器开关
											///发送对应控制指令控制设备：手动模式下，全部关闭
	 
										//	HAL_UART_Transmit(&huart5, data, strlen(data), 0xFFFF);
	 
									///等待USART5接收数据处理完成状态
			
									///发送：当前处于手动模式
									} 
								}	
							else
								Control_States.Auto_Mode = true;

		default	:
			break;
    }
}
