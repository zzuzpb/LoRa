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

#define CLOCK_16MHZ			              1

volatile uint32_t system_time = 0;

#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif

/*!
 * Flag to indicate if the MCU is Initialized
 */
bool McuInitialized = false;

extern bool RtcInitialized;

extern RadioEvents_t *RadioEvents;

bool sendwkup = false;

void BoardInitMcu( void )
{
	if( McuInitialized == false )
	{
		HAL_Init( );
	
        /***************时钟初始化********************/
		SystemClockConfig( );
        
        /* Enable Power Control clock */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE(); ///开启时钟
						
        /***************SX1276 I/O初始化********************/
		SX1276IoInit( );
		
		/***************LoRa电源控制 I/O初始化********************/
		LoRaPower_Init( );
	
        /****************SPI初始化*******************/
		SPI1_NSS( );						///片选初始化
						
		SPI1_Init( );					///SPI初始化	
		
		McuInitialized = true;
	} 
    else
    {
        SystemClockReConfig( );
        
        /* Enable Power Control clock */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOH_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE(); ///开启时钟
        
        /***************SX1276 I/O初始化********************/
        SX1276IoInit( );

        /***************LoRa电源控制 I/O初始化********************/
        LoRaPower_Init( );

        /****************SPI初始化*******************/
        SPI1_NSS( );						///片选初始化
                    
        SPI1_Init( );	
//        Radio.Init( RadioEvents );
    }
      
    /***************串口初始化********************/
    MX_USART1_UART_Init( );  
            
    MX_USART2_UART_Init(  );
    
    /*****************I2C初始化********************/
    MX_I2C2_Init( );
    
    /****************ADC初始化*******************/
    MX_ADC_Init( );
    
    /*****************电源管理********************/
    InitPower( );

    RTC_Init( );
        
    /*******************开启RTC中断*******************/
    HAL_NVIC_SetPriority(RTC_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);
    
    Read_Flash_Data(  );
}

extern SPI_HandleTypeDef            hspi1;  //SPI句柄
extern UART_HandleTypeDef 			huart1;

/*
 *	BoardDeInitMcu:	进入低功耗模式：停机，需要设置相应IO模式
 *	返回值: 				无
 */
void BoardDeInitMcu( void )
{ 
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Enable GPIOs clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();
	
	/****************************************/
    /* Disable the Peripheral */	
	HAL_ADC_MspInit(&hadc);  ///OK
	hadc.State = HAL_ADC_STATE_RESET;
	
	 /* Disable the selected I2C peripheral */
    HAL_I2C_DeInit(&hi2c2);
	hi2c2.State = HAL_I2C_STATE_RESET;
	
	///关闭UART1时钟
    HAL_UART_DeInit(&huart1);
	huart1.gState = HAL_UART_STATE_RESET;
	
	///关闭UART2时钟
	HAL_UART_DeInit(&huart2);
	huart2.gState = HAL_UART_STATE_RESET;
	
	///关闭UART2时钟
	HAL_UART_DeInit(&huart5);
	huart5.gState = HAL_UART_STATE_RESET;
	
	/*******************关闭SPI*********************/
	/* Disble the selected SPI peripheral */
	HAL_SPI_DeInit(&hspi1);
	hspi1.State = HAL_SPI_STATE_RESET;
		
    GPIO_InitStructure.Pin = GPIO_PIN_All;   ///GPIO_PIN_All
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG; ///low_power,其它较高
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure); 
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);
    HAL_GPIO_Init(GPIOH, &GPIO_InitStructure);
            
    GPIO_InitStructure.Pin = 0xEDFF;  /// PB9：CH_CE(ok)/PB5：485_DE/PB0: RESET/PB1: DIO0 保留充电IO配置
	GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStructure.Pull = GPIO_PULLDOWN;
	GPIO_InitStructure.Speed     = GPIO_SPEED_FREQ_LOW;	
	HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
            
	RtcInitialized = false;
    
    HAL_SuspendTick();
	
	/* Disable GPIOs clock */
	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
	__HAL_RCC_GPIOH_CLK_DISABLE();
    
}

void SystemClockReConfig( void )
{
    __HAL_RCC_PWR_CLK_ENABLE( );
    __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

    /* Enable HSE */
    __HAL_RCC_HSE_CONFIG( RCC_HSE_ON );

    /* Wait till HSE is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_HSERDY ) == RESET )
    {
    }

    /* Enable PLL */
    __HAL_RCC_PLL_ENABLE( );

    /* Wait till PLL is ready */
    while( __HAL_RCC_GET_FLAG( RCC_FLAG_PLLRDY ) == RESET )
    {
    }

    /* Select PLL as system clock source */
    __HAL_RCC_SYSCLK_CONFIG ( RCC_SYSCLKSOURCE_PLLCLK );

    /* Wait till PLL is used as system clock source */
    while( __HAL_RCC_GET_SYSCLK_SOURCE( ) != RCC_SYSCLKSOURCE_STATUS_PLLCLK )
    {
    }
    /* Resume Tick interrupt if disabled prior to sleep mode entry*/
    HAL_ResumeTick();
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

void BoardUniquedeviceID( uint8_t *id )
{
    uint32_t ID[3] = {0};
    
    ID[0] = *(uint32_t *)ID1;
    ID[1] = *(uint32_t *)ID2;
    ID[2] = *(uint32_t *)ID3;
    
    DEBUG(2, "ID[0] = %02x ID[1] = %02x ID[2] = %08x\r\n",ID[0], ID[1], ID[2]);
    
    id[0] = ( ( *( uint32_t* )ID3 ) ) >> 24;
    id[1] = ( ( *( uint32_t* )ID3 ) ) >> 16;
    id[2] = ( ( *( uint32_t* )ID3 ) ) >> 8;
    id[3] = ( ( *( uint32_t* )ID3 ) );
    
    DEBUG(2,";Device:%02x%02x%02x%02x;\r\n",id[0], id[1], id[2], id[3]);
}

uint8_t BoardGetBatteryLevel( void )
{
    uint8_t batteryLevel = 0;

    batteryLevel =  (RF_Send_Data.Battery*300/100);
    return (batteryLevel);
}

/**
  * @brief 系统时钟初始化
  * @param 外部时钟72MHZ
  * @retval None
  */
void SystemClockConfig( void )
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

    /**Configure the main internal regulator output voltage 
    */
#if CLOCK_16MHZ
	
  /**Configure the main internal regulator output voltage 
    */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }

	delay_init(16);
	
#endif

    /**Configure the Systick interrupt time 1ms
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

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
