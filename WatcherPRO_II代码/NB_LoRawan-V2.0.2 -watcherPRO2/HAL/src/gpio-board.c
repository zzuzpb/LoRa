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

void EXTI0_1_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );

}

void EXTI2_3_IRQHandler( void )
{
#if !defined( USE_NO_TIMER )
 //   RtcRecoverMcuStatus( );
#endif
	
  HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
	HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
	
}


void EXTI4_15_IRQHandler( void )
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

/**
  * @brief  EXTI callback
  * @param  EXTI : EXTI handle
  * @retval None
	* @brief	中断回调函数：处理中断事件----进行IO判断，处理相应的DIO0---DIO5
  */
void HAL_GPIO_EXTI_Callback( uint16_t GPIO_Pin )
{
	switch(GPIO_Pin)
    {
        case GPIO_PIN_1:
             SX1276OnDio0Irq(  );	
			 DEBUG(3,"SX1276OnDio0Irq\r\n");

            break;
        case GPIO_PIN_2:
			 SX1276OnDio1Irq(  );
		    DEBUG(3,"SX1276OnDio1Irq\r\n");

            break;
				
		case GPIO_PIN_8:  //外部中断5，PB5 INT充电错误中断	
						
			DEBUG(2,"CHARGE INTERRUP\r\n");

            break;

        case GPIO_PIN_10:
             SX1276OnDio2Irq(  );	
			 DEBUG(2,"SX1276OnDio2Irq\r\n");
     
            break;

		default	:
			break;
    }
}
