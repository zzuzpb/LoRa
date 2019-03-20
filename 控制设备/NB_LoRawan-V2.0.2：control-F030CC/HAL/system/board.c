/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"

/*!
 * Battery level ratio (battery dependent)
 */
#define BATTERY_STEP_LEVEL                0.23

#define CLOCK_16MHZ												1


#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif

/*!
 * Initializes the unused GPIO to a know status
 */
//static void BoardUnusedIoInit( void );


/*!
 * Used to measure and calibrate the system wake-up time from STOP mode
 */
//static void CalibrateSystemWakeupTime( void );

/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
//static void SystemClockReConfig( void );

/*!
 * Timer used at first boot to calibrate the SystemWakeupTime
 */
//static TimerEvent_t CalibrateSystemWakeupTimeTimer;

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

/*!
 * Flag to indicate if the SystemWakeupTime is Calibrated
 */
//static bool SystemWakeupTimeCalibrated = false;

/*!
 * Callback indicating the end of the system wake-up time calibration
 */
//static void OnCalibrateSystemWakeupTimeTimerEvent( void )
//{
//    SystemWakeupTimeCalibrated = true;
//}

extern bool RtcInitialized;

void BoardInitPeriph( void )
{
 

}

void BoardInitMcu( void )
{
	if( McuInitialized == false )
	{

		HAL_Init( );
	
 /***************时钟初始化********************/
		SystemClockConfig( );
			
		delay_init( 16 ); ///产生US
		
		/* Enable Power Control clock */
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOF_CLK_ENABLE();
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOB_CLK_ENABLE();
	
/***************串口初始化********************/
		MX_USART1_UART_Init( );  
		
		MX_USART5_UART_Init( );

		RTC_Init( );
	
		/*******************开启RTC中断*******************/
		HAL_NVIC_SetPriority(RTC_IRQn, 1, 0);
		HAL_NVIC_EnableIRQ(RTC_IRQn);
		
		
		/*******************开启UART5中断*******************/
		HAL_NVIC_SetPriority(USART3_6_IRQn, 3, 0);
		HAL_NVIC_EnableIRQ(USART3_6_IRQn);

	
	/***************SX1276 I/O初始化********************/
		SX1276IoInit( );
		
		/***************LoRa状态灯 I/O初始化********************/
		LoRaLed_Init( );
	
	/****************SPI初始化*******************/
		SPI1_NSS( );						///片选初始化
		SPI1_Init( );					///SPI初始化	

		McuInitialized = true;
	} 
       
}

extern SPI_HandleTypeDef            hspi1;  //SPI句柄
extern UART_HandleTypeDef 					huart1;

void Rs485_Turn_Off(void) ///关闭485电源
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOC_CLK_ENABLE();           //开启GPIOB时钟
	
		//PB0、PB1 --- 读取硬件版本   PB8 --- 485_RE  PB9 ---- 485_DE  
    GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_9; 
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_NOPULL;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
	
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8,GPIO_PIN_SET);	//PB15置1
	  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_9,GPIO_PIN_RESET);	//PB15置1
}

/*
 *	BoardDeInitMcu:	进入低功耗模式：停机，需要设置相应IO模式
 *	返回值: 				无
 */
void BoardDeInitMcu( void )
{ 
	GPIO_InitTypeDef GPIO_InitStructure;
	
	__HAL_RCC_HSI_DISABLE();
	__HAL_RCC_PLL_DISABLE();
	  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_DISABLE();
	
	  /* Enable GPIOs clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	
	/****************************************/
 /* Disable the Peripheral */
	__HAL_UART_DISABLE(&huart1);
//	HAL_UART_MspDeInit(&UartHandle);
		
	/*******************关闭SPI*********************/
	/* Disble the selected SPI peripheral */
  HAL_SPI_DeInit(&hspi1);

  GPIO_InitStructure.Pin = 0xFF0F;  /// LED5---PA15  UART TXIO---PA9 Power_Insert --- PA0 设置为输出
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	GPIO_InitStructure.Pin = 0xF338;  /// LED7---PB15   485控制IO: PB8、PB9  I2C: SCL --- PB6  SDA --- PB7   PB0  PB1 设置为输出0x7C2F
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.Pin = GPIO_PIN_All;
  GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStructure.Pull = GPIO_NOPULL;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_HIGH;
	
	HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);	
	HAL_GPIO_Init(GPIOF, &GPIO_InitStructure);
		
	/******************* Disable Systick*********************/
	SysTick->CTRL  &= ~SysTick_CTRL_TICKINT_Msk;    // Systick IRQ off 
	SCB->ICSR |= SCB_ICSR_PENDSTCLR_Msk;            // Clear SysTick Exception pending flag
	
	McuInitialized = false;
	RtcInitialized = false;
  /* Disable GPIOs clock */
	__HAL_RCC_GPIOA_CLK_DISABLE();
  __HAL_RCC_GPIOB_CLK_DISABLE();
  __HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOF_CLK_DISABLE();	
	DEBUG(4,"__WFI\r\n");
 }

uint32_t BoardGetRandomSeed( void )
{
    return ( ( *( uint32_t* )ID1 ) ^ ( *( uint32_t* )ID2 ) ^ ( *( uint32_t* )ID3 ) );
}

void BoardGetUniqueId( uint8_t *id )
{
    id[7] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 24;
    id[6] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 16;
    id[5] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) ) >> 8;
    id[4] = ( ( *( uint32_t* )ID1 )+ ( *( uint32_t* )ID3 ) );
    id[3] = ( ( *( uint32_t* )ID2 ) ) >> 24;
    id[2] = ( ( *( uint32_t* )ID2 ) ) >> 16;
    id[1] = ( ( *( uint32_t* )ID2 ) ) >> 8;
    id[0] = ( ( *( uint32_t* )ID2 ) );
}

uint8_t BoardGetBatteryLevel( void )
{
    uint8_t batteryLevel = 0;

//    batteryLevel =  CheckBattery();
    return batteryLevel;
}


/**
  * @brief 系统时钟初始化
  * @param 内部时钟16MHZ
  * @retval None
  */
void SystemClockConfig( void )
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure LSE Drive Capability 
    */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);


}

//void CalibrateSystemWakeupTime( void )
//{
//    if( SystemWakeupTimeCalibrated == false )
//    {
//        TimerInit( &CalibrateSystemWakeupTimeTimer, OnCalibrateSystemWakeupTimeTimerEvent );
//        TimerSetValue( &CalibrateSystemWakeupTimeTimer, 1000 );
//        TimerStart( &CalibrateSystemWakeupTimeTimer );
//        while( SystemWakeupTimeCalibrated == false )
//        {
//            TimerLowPowerHandler( );
//        }
//    }
//}

//void SystemClockReConfig( void )
//{
//   
//}

uint8_t GetBoardPowerSource( void )
{
#if defined( USE_USB_CDC )
    if( UartUsbIsUsbCableConnected( ) == 0 )
    {
        return BATTERY_POWER;
    }
    else
    {
        return USB_POWER;
    }
#else
    return BATTERY_POWER;
#endif
}

#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif
