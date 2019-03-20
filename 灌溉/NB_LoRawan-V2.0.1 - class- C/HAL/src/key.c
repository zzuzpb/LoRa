
#include "key.h"
#include "rtc-board.h"
#include "stm32l0xx_hal.h"
#include "delay.h"
#include "debug.h"


void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOB_CLK_ENABLE();               		 //开启GPIOB时钟

	GPIO_Initure.Pin=KEY_A_PIN|KEY_B_PIN;  
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      			//上升沿触发
	GPIO_Initure.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(KEY_PORT,&GPIO_Initure);
	
		//中断线4-PC15
	HAL_NVIC_SetPriority(EXTI4_15_IRQn,4,0);       //抢占优先级为0，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);             //使能中断线9

}

void Led_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();               		 //开启GPIOB时钟

	GPIO_Initure.Pin=GPIO_PIN_15;  
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;      			//上升沿触发
	GPIO_Initure.Pull=GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOA,&GPIO_Initure);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);	//PB12置1
}

/*
 * 初始化待机按键
*/
void StandbyInit(void)
{
	/* 定义IO硬件初始化结构体变量 */
	GPIO_InitTypeDef GPIO_InitStruct;

	/* 使能(开启)KEY引脚对应IO端口时钟 */  
	PWR_KEY_RCC_CLK_ENABLE(); 

	/* 配置KEY2 GPIO:中断模式，下降沿触发 */
	GPIO_InitStruct.Pin = WAKEUP_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING; // 特别注意这里要使用中断模式,下拉，上升沿触发
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(WAKEUP_PORT, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(PWR_KEY_EXTI_IRQn, 1, 0);	//此处的中断优先级不能比系统时钟的优先级高，否则按键消抖就用不了了
	HAL_NVIC_EnableIRQ(PWR_KEY_EXTI_IRQn);			//中断使用
}


/**
  * 进入休眠模式(关机)
  */
void StandbyEnterMode(void)
{
	__HAL_RCC_PWR_CLK_ENABLE();
	Standy_Io_Mode(  );
		
  __HAL_RCC_BACKUPRESET_FORCE();      //复位备份区域
  HAL_PWR_EnableBkUpAccess();         //后备区域访问使能  
	
	Radio.Sleep( );  ///LoRa进休眠状态
	
	LoRaPower_Disable(  );
	
	GPS_Disable(  );
		
	__HAL_RTC_WRITEPROTECTION_DISABLE(&RtcHandle);//关闭RTC写保护
	
	
	//关闭RTC相关中断，可能在RTC实验打开了
	__HAL_RTC_WAKEUPTIMER_DISABLE_IT(&RtcHandle,RTC_IT_WUT);
	__HAL_RTC_TIMESTAMP_DISABLE_IT(&RtcHandle,RTC_IT_TS);
	__HAL_RTC_ALARM_DISABLE_IT(&RtcHandle,RTC_IT_ALRA|RTC_IT_ALRB);
	
	//清除RTC相关中断标志位
	__HAL_RTC_ALARM_CLEAR_FLAG(&RtcHandle,RTC_FLAG_ALRAF|RTC_FLAG_ALRBF);
	__HAL_RTC_TIMESTAMP_CLEAR_FLAG(&RtcHandle,RTC_FLAG_TSF); 
	__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&RtcHandle,RTC_FLAG_WUTF);
	
	/* 禁用唤醒源:唤醒引脚PA0 由于PA0接在水流传感器上，为避免意外，将其禁用 */
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
	HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN2);

//	/* 清除所有唤醒标志位 */
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
  __HAL_RCC_BACKUPRESET_RELEASE();                    //备份区域复位结束
  __HAL_RTC_WRITEPROTECTION_ENABLE(&RtcHandle);     //使能RTC写保护

	/* 使能唤醒引脚：PC13做为系统唤醒输入 */
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN2);

	/* 进入待机模式 */
	HAL_PWR_EnterSTANDBYMode();
	
	
}

/**
  * 函数功能: 用于检测按键是否被长时间按下
  * 输入参数: 无
  * 返 回 值: 无
  * 说    明：1 ：按键被长时间按下  0 ：按键没有被长时间按下
  */
uint8_t StandbyCheckPwrkey(void)
{			
	uint8_t downCnt =0;																//记录按下的次数
	uint8_t upCnt =0;																//记录松开的次数			
 
	while(1)																		//死循环，由return结束
	{	
		delay_ms(10);																//延迟一段时间再检测
		if(HAL_GPIO_ReadPin(WAKEUP_PORT,WAKEUP_PIN) == KEY_ON)			//检测到按下按键
		{		
			downCnt++;																//记录按下次数
			upCnt=0;																//清除按键释放记录
			if(downCnt>=100)														//按下时间足够
			{
				DEBUG(2,"长按电源按钮 \r\n");
				while(HAL_GPIO_ReadPin(WAKEUP_PORT,WAKEUP_PIN) == KEY_ON); ///等待按键释放
				return 1; 															//检测到按键被时间长按下
			}
		}
		else 
		{
			upCnt++; 																//记录释放次数
			if(upCnt>5)																//连续检测到释放超过5次
			{
				DEBUG(2,"按下时间不足\r\n");		
				while(HAL_GPIO_ReadPin(WAKEUP_PORT,WAKEUP_PIN) == KEY_ON); ///等待按键释放
				return 0;															//按下时间太短，不是按键长按操作
			}
		}
	}
}

/*
*设备工作状态判断
*/
void WorkStatusJudgment(void)
{
	 /* 检测系统是否是从待机模式启动的 */ 	
	if(StandbyCheckPwrkey())  ///读IO信息，无触发中断
	{
		printf("开机\r\n");		
	
			/* 清除待机标志位 */
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
		__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		__HAL_RCC_CLEAR_RESET_FLAGS();
		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_SET);	//PB12置1
		
	}
	else
	{
        DEBUG(3,"系统开机休眠\r\n");
        StandbyEnterMode( );
	}

	if(__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
  {
    DEBUG(3,"\n软件复位不重置\n");
		__HAL_RCC_CLEAR_RESET_FLAGS();
  }
}


void Standy_Io_Mode(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	
	  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
	
	HAL_UART_MspDeInit(&huart1);
	
	GPIO_InitStructure.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9|GPIO_PIN_10,GPIO_PIN_SET);	//PB12置1

  /* Disable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
//  __HAL_RCC_GPIOC_CLK_DISABLE();
//  __HAL_RCC_GPIOH_CLK_DISABLE();

}
