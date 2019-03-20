/*
 * led.c	LED驱动文件
*/

#include "user_led.h"

/*
 * InitLed:				初始化LED
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t InitLed(void)
{
	//MX_GPIO_Init();已初始化
	
//	__HAL_RCC_GPIOA_CLK_ENABLE();
//	HAL_GPIO_WritePin(GPIOA,LED_Pin, GPIO_PIN_RESET);
//	GPIO_InitStruct.Pin |= LED_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	return 1;
}

/*
 * LedPowerOn:			打开LED供电电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t LedPowerOn(void)
{
	//led无电源开关，直接与VDD连接
	return 1;
}

/*
 * LedPowerOff:			打开LED供电电源
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t LedPowerOff(void)
{
	//led无电源开关，直接与VDD连接，所以无法关闭Led的电源
	return 0;
}

/*
 * OpenSolenoid:		打开Led
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t OpenLed(void)
{
	//led无电源开关，直接与VDD连接，低电平打开
	HAL_GPIO_WritePin(Out_LED_GPIO_Port,Out_LED_Pin, GPIO_PIN_SET);
	return 1;
}

/*
 * CloseLed:			关闭Led
 * 参数:				无
 * 返回值:				1成功 0失败
*/
uint8_t CloseLed(void)
{
	//led无电源开关，直接与VDD连接，高电平关闭
	HAL_GPIO_WritePin(Out_LED_GPIO_Port,Out_LED_Pin, GPIO_PIN_RESET);
	return 1;
}

/*
 * FlashLed:			闪烁Led
 * rate:				闪烁频率Hz 有效值为1~1000
 * count:				闪烁次数,0表示一直闪烁
 * 返回值:				1成功 0失败
*/
uint8_t FlashLed(uint32_t rate , uint32_t count)
{
	for(uint32_t i=0;i<count;i++)
	{
		uint32_t time=1000/rate;
		OpenLed();
		HAL_Delay(time/2);
		CloseLed();
		HAL_Delay(time/2);
	}
	return 1;
}
