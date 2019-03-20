/*
**************************************************************************************************************
*	@file		control.c
*	@author Ysheng
*	@version V.0.1
*	@date    2017/8/9
*	@brief 控制设备文件
***************************************************************************************************************
*/
#include <math.h>
#include "control.h"
#include "stm32f0xx_hal.h"
#include "delay.h"
#include "debug.h"
#include "user-app.h"

Control_State Control_States = {true, {01,0,0,0}, {0}, 0};

////////////////////////////////////////////////////////// 
// 
//功能：16进制转为10进制
// 
//输入：uint8_t *hex         待转化的16进制数据
//      uint8_t length       16进制数据长度
// 
//?输出：
// 
//返回结果:int  rslt        转化后的10进制数据
// 
//思路:16进制每个字符位所表示的10进制范围：0 ~255,
//      左移8位(<<8)等价于256 
// 
///////////////////////////////////////////////////////// 
uint8_t HextoDec(uint8_t *hex, uint8_t length) 
{ 
    int i; 
    uint8_t rslt = 0;
    for(i=0; i<length; i++) 
    { 
        rslt += (uint8_t)(hex[i])<<(8*(length-1-i)); 
                                                         
    }
		delay_ms(2);
		DEBUG(2,"rslt = %d\r\n",rslt);
    return rslt; 
}


/*功能：		初始化获取手动/自动模式
*返回参数： 无
*/
void Get_Work_ModeInit( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOC_CLK_ENABLE();               		 //开启GPIOB时钟

	GPIO_InitStruct.Pin=GPIO_PIN_13;  
	GPIO_InitStruct.Mode=GPIO_MODE_IT_RISING;      			//上升沿触发
	GPIO_InitStruct.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOC,&GPIO_InitStruct);
	
		//中断线4-PC15
	HAL_NVIC_SetPriority(EXTI4_15_IRQn,7,0);       //抢占优先级为0，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);             //使能中断线9
}


/*功能：		初始化继电器
*返回参数： 无
*/
void Control_Relay_Init( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
 
	/* Peripheral clock enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
  
  GPIO_InitStruct.Pin=GPIO_PIN_1; 
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;//GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_InitStruct.Pull=GPIO_PULLUP;          //上拉
	GPIO_InitStruct.Speed=GPIO_SPEED_FREQ_HIGH;     //高速
	HAL_GPIO_Init(GPIOA,&GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);	
	
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8|GPIO_PIN_9,GPIO_PIN_RESET);
}

/*功能：		开启STM8电源
*返回参数： 无
*/
void Enable_Stm8_Power( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();               		 //开启GPIOB时钟

	GPIO_InitStruct.Pin=GPIO_PIN_12;  
	GPIO_InitStruct.Mode=GPIO_MODE_OUTPUT_PP;      			//下拉
	GPIO_InitStruct.Pull=GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);	
}

/*功能：		查询手动、自动按键切换开关
*返回参数： 无
*/
void Check_Key_Mode( void )
{
	///增加轮询按键功能，防止MCU断电没查询出中断状态，导致不工作
	if( (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)) == KEY_ON ) ///手动模式
	{
		delay_ms(20);
		
		if( (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)) == KEY_ON ) ///手动模式
		{
			Control_States.Auto_Mode = false;
		}  
	}
	else
	{
		delay_ms(20);
		
		if( (HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)) == KEY_OFF ) ///自动模式
		{
			Control_States.Auto_Mode = true;
		}
	}	
}

/*功能：		处理控制信息
*输入参数： RF96接收到的数据经转换为10进制
*返回参数： 无
*/
void Control_Relay(uint8_t *rfdata)
{
	 Control_States.retry_conter = 0;
	 memset(Control_States.send_buf, 0, 4);
	 DEBUG(2,"tx_data000 = %s\r\n",Control_States.control_buf);
	 Control_States.control_buf[0] = 01;
	 Control_States.control_buf[1] = 00;
//	 Control_States.control_buf[2] = HextoDec(&rfdata[0],1);
//	 Control_States.control_buf[3] = HextoDec(&rfdata[1],1);	
	 Control_States.control_buf[2] = rfdata[0];
	 Control_States.control_buf[3] = rfdata[1];	
	
	if(Control_States.control_buf[2] != 0) ///不执行查询命令
	{
		do
		{			
			HAL_UART_Transmit(&huart5, Control_States.control_buf, 4, 0xFFFF);
			DEBUG(2,"tx_data555 = %s\r\n",Control_States.control_buf);
			delay_ms(50);
			
			///等待USART5接收数据处理完成状态
			if(UART_RX_DATA.USART_TX_STATE)
			{
			 for(uint8_t i = 0; i < 4; i++)
			 DEBUG(2,"%02d",UART_RX_DATA.USART_RX_BUF[i]);
			 DEBUG(2,"\r\n");

			 UART_RX_DATA.USART_TX_STATE = false;
			 memcpy(Control_States.send_buf, UART_RX_DATA.USART_RX_BUF, UART_RX_DATA.USART_RX_Len); ///USART5接收缓存copy
			 memset(UART_RX_DATA.USART_RX_BUF, 0, UART_RX_DATA.USART_RX_Len);
			 UART_RX_DATA.USART_RX_Len = 0;
			}			 		 
		Control_States.retry_conter++;
		}while((0 != strcmp((char *)Control_States.control_buf,(char *)Control_States.send_buf)) && Control_States.retry_conter<=2);
	}
	else  ///执行查询命令
	{
		HAL_UART_Transmit(&huart5, Control_States.control_buf, 4, 0xFFFF);
		DEBUG(2,"tx_data666 = %s\r\n",Control_States.control_buf);
		delay_ms(50);
		
		///等待USART5接收数据处理完成状态
		if(UART_RX_DATA.USART_TX_STATE)
		{
		 for(uint8_t i = 0; i < 4; i++)
		 DEBUG(2,"%02d",UART_RX_DATA.USART_RX_BUF[i]);
		 DEBUG(2,"\r\n");
		 UART_RX_DATA.USART_TX_STATE = false;
		 memcpy(Control_States.send_buf, UART_RX_DATA.USART_RX_BUF, UART_RX_DATA.USART_RX_Len);
		 memset(UART_RX_DATA.USART_RX_BUF, 0, UART_RX_DATA.USART_RX_Len);
		 UART_RX_DATA.USART_RX_Len = 0;
		}			 
	}
}
