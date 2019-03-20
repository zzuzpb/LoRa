/*
 * led.c	LED驱动文件
*/

#include "user_led.h"

E_LedState  G_Ledstate;          //led的状态
uint16_t    G_TimeOut=0;           //计时
/*
 * InitLed:				初始化LED
 * 参数:				无
 * 返回值:				1成功 0失败
*/
void InitLed(void)
{
	//MX_GPIO_Init();已初始化
	
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//	HAL_GPIO_WritePin(GPIOA,LED_Pin, GPIO_PIN_RESET);
//	GPIO_InitStruct.Pin |= LED_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/*
 * LedOpen:		打开Led
 * 参数:				无
 * 返回值:				1成功 0失败
*/
void LedOpen(void)
{
	//led无电源开关，直接与VDD连接，低电平打开
	HAL_GPIO_WritePin(OUT_LED_GPIO_Port,OUT_LED_Pin, GPIO_PIN_SET);
}

/*
 * LedClose:			关闭Led
 * 参数:				无
 * 返回值:				1成功 0失败
*/
void LedClose(void)
{
	//led无电源开关，直接与VDD连接，高电平关闭
	HAL_GPIO_WritePin(OUT_LED_GPIO_Port,OUT_LED_Pin, GPIO_PIN_RESET);
}

/*
 * LedChangeState:		切换LED闪烁方式
 * State			    LED闪烁方式
 * 返回值:				无
*/
void LedChangeState(E_LedState State)
{
    G_Ledstate = State;
}

/*
 * 函数放在定时器中断使用
 * BaseTime 为定时器的周期,单位是毫秒,最小周期是50ms
*/
void LedDisplay(uint16_t BaseTime)
{
     G_TimeOut++;
    if( G_Ledstate == E_LED_LOCATION )
    {
        if( (3000/BaseTime) == G_TimeOut)       //3秒切换一次，慢闪
        {
            HAL_GPIO_WritePin(OUT_LED_GPIO_Port,OUT_LED_Pin, GPIO_PIN_SET);
            G_TimeOut = 0;
        }
    }
    else if( G_Ledstate == E_LED_CONNECT )
    {
        if( (1000/BaseTime) == G_TimeOut)       //1秒切换一次，慢闪
        {
            HAL_GPIO_TogglePin(OUT_LED_GPIO_Port,OUT_LED_Pin);
            G_TimeOut=0;
        }
    }
    else if( G_Ledstate == E_LED_DISCONNECT )   //200ms切换一次，快闪
    {
        if( (200/BaseTime) ==G_TimeOut)
        {
            HAL_GPIO_TogglePin(OUT_LED_GPIO_Port,OUT_LED_Pin);
            G_TimeOut=0;
        }
    }
    else if( G_Ledstate == E_LED_ERROR )        //异常,常亮1秒，短灭200ms
    {
        if( BaseTime*G_TimeOut<1000)
        {
            LedOpen();
        }
        else
        {
            LedClose();
            if(BaseTime*G_TimeOut>1200)
                G_TimeOut=0;
        }
    }
    else if( G_Ledstate == E_LED_SENDOK )       //发送完成   
    {
        if( (50/BaseTime) ==G_TimeOut)
        {
            HAL_GPIO_TogglePin(OUT_LED_GPIO_Port,OUT_LED_Pin);
            G_TimeOut=0;
        }
    }
    else if( G_Ledstate == E_LED_INIT )         //初始化    
    {
        LedOpen();//常亮
        G_TimeOut=0;
    }
}
