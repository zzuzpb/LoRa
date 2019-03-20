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

/*!
 * \brief DIO 0 IRQ callback
 */
extern void SX1276OnDio0Irq( void );

/*!
 * \brief DIO 0 IRQ callback
 */
extern void SX1276TxOnDio0Irq( void );

/*!
 * \brief DIO 1 IRQ callback
 */
extern void SX1276OnDio1Irq( void );

/*!
 * \brief DIO 0 IRQ callback
 */
extern void SX1276TxOnDio1Irq( void );

/*!
 * \brief DIO 2 IRQ callback
 */
extern void SX1276OnDio2Irq( void );

/*!
 * \brief DIO 2 IRQ callback
 */
extern void SX1276TxOnDio2Irq( void );

/*!
 * \brief DIO 3 IRQ callback
 */
extern void SX1276OnDio3Irq( void );

/*!
 * \brief DIO 3 IRQ callback
 */
extern void SX1276TxOnDio3Irq( void );

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
	if( GPIO_Pin == GPIO_PIN_0 ) ///SPI3
	{   
        DEBUG(2,"GPIO_PIN_0 SX1276OnDio0Irq\r\n");        
        SX1276OnDio0Irq(  );
	}
    
    else if( GPIO_Pin == GPIO_PIN_1 )
	{
        DEBUG(2,"GPIO_PIN_1 SX1276OnDio1Irq\r\n");
		SX1276OnDio1Irq(  );
	}
	else if( GPIO_Pin == GPIO_PIN_2 )
	{        
        DEBUG(2,"GPIO_PIN_2 SX1276OnDio2Irq\r\n");
		SX1276OnDio2Irq(  );	        
	}
 
    else if( GPIO_Pin == GPIO_PIN_8 )
    {
        DEBUG(2,"GPIO_PIN_8 SX1276TxOnDio0Irq\r\n");           
        SX1276TxOnDio0Irq(  );
    }
    
    else if( GPIO_Pin == GPIO_PIN_9 )
    {       
        DEBUG(2,"GPIO_PIN_9 SX1276TxOnDio1Irq\r\n");           
        SX1276TxOnDio1Irq(  );
    }
    
    else if( GPIO_Pin == GPIO_PIN_10 )
    {
        DEBUG(2,"GPIO_PIN_10 SX1276TxOnDio2Irq\r\n");           
        SX1276TxOnDio2Irq(  );
    }
}
