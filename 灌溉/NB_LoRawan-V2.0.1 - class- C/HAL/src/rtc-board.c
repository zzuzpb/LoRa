/**
  ******************************************************************************
  * File Name          : RTC.c
  * Description        : This file provides code for the configuration
  *                      of the RTC instances.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdbool.h>
#include "utilities.h"
#include "board.h"
#include "rtc-board.h"

RTC_HandleTypeDef RtcHandle;

/*!
 * RTC Time base in us
 */
#define RTC_ALARM_TIME_BASE                             244.14 /// 122.07  //122.07 1000 //

/*!
 * MCU Wake Up Time
 */
#define MCU_WAKE_UP_TIME                                3400

/*!
 * \brief Start the Rtc Alarm (time base 1s)
 */
static void RtcStartWakeUpAlarm( uint32_t timeoutValue );

/*!
 * \brief Read the MCU internal Calendar value
 *
 * \retval Calendar value
 */
static uint64_t RtcGetCalendarValue( void );

/*!
 * \brief Clear the RTC flags and Stop all IRQs
 */
static void RtcClearStatus( void );

/*!
 * \brief RTC config from wake up Alarm
 */

void SystemClockConfig_STOP(void);

/*!
 * \brief Indicates if the RTC is already Initalized or not
 */
bool RtcInitialized = false;

/*!
 * \brief Flag to indicate if the timestamps until the next event is long enough
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the LowPower Mode even if the timestamps until the
 * next event is long enough to allow Low Power mode
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * Keep the value of the RTC timer when the RTC alarm is set
 */
static TimerTime_t RtcTimerContext = 0;

/*!
 * Number of seconds in a minute
 */
static const uint8_t SecondsInMinute = 60;

/*!
 * Number of seconds in an hour
 */
static const uint16_t SecondsInHour = 3600;

/*!
 * Number of seconds in a day
 */
static const uint32_t SecondsInDay = 86400;

/*!
 * Number of hours in a day
 */
static const uint8_t HoursInDay = 24;

/*!
 * Number of days in a standard year
 */
static const uint16_t DaysInYear = 365;

/*!
 * Number of days in a leap year
 */
static const uint16_t DaysInLeapYear = 366;

/*!
 * Number of days in a century
 */
static const double DaysInCentury = 36524.219;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Hold the previous year value to detect the turn of a century
 */
static uint8_t PreviousYear = 0;

//static TimerTime_t calendarValue = 0;

/*!
 * Century counter
 */
static uint8_t Century = 0;

/***********************************以下为RTC底层代码部分***************************************/


/* RTC init function */
void RTC_Init(void)
{
	RTC_TimeTypeDef RTC_TimeStruct;
  RTC_DateTypeDef RTC_DateStruct;
	
	/**Initialize RTC Only 
	*/
	if( RtcInitialized == false )
 {

		RtcHandle.Instance = RTC;
		RtcHandle.Init.HourFormat = RTC_HOURFORMAT_24;
	 
		RtcHandle.Init.AsynchPrediv = 3; //3; 127
		RtcHandle.Init.SynchPrediv = 1; ///3; 255
	 
		RtcHandle.Init.OutPut = RTC_OUTPUT_DISABLE;
		RtcHandle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
		RtcHandle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
		if (HAL_RTC_Init(&RtcHandle) != HAL_OK)
		{
			printf("HAL_RTC_Init error \r\n");
			Error_Handler();
		}

		/**Initialize RTC and set the Time and Date 
		*/
		RTC_TimeStruct.TimeFormat = RTC_HOURFORMAT12_AM;	
		RTC_TimeStruct.Hours = 0;
		RTC_TimeStruct.Minutes = 0;
		RTC_TimeStruct.Seconds = 0;
		RTC_TimeStruct.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		RTC_TimeStruct.StoreOperation = RTC_STOREOPERATION_RESET;
		if (HAL_RTC_SetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BCD) != HAL_OK)
		{
			Error_Handler();
		}

		RTC_DateStruct.WeekDay = RTC_WEEKDAY_MONDAY;
		RTC_DateStruct.Month = RTC_MONTH_JANUARY;
		RTC_DateStruct.Date = 1;
		RTC_DateStruct.Year = 17;

		if (HAL_RTC_SetDate(&RtcHandle, &RTC_DateStruct, RTC_FORMAT_BCD) != HAL_OK)
		{
			Error_Handler();
		}
		
		RtcInitialized = true;
	}
}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();

    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ(RTC_IRQn);

  }
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
} 

/* USER CODE BEGIN 1 */

/*******************************以上为RTC底层代码部分****************************************/

void RtcStopTimer( void )
{
    RtcClearStatus( );
}

uint32_t RtcGetMinimumTimeout( void )
{
    return( (uint32_t)ceil( 10 * RTC_ALARM_TIME_BASE ) );
}

void RtcSetTimeout( uint32_t timeout )
{
    uint32_t timeoutValue = 0;

    timeoutValue = timeout;

    if( timeoutValue < ( 10 * RTC_ALARM_TIME_BASE ) )
    {
        timeoutValue = (uint32_t)(10.0 * RTC_ALARM_TIME_BASE);
    }
    
    if( timeoutValue < 55000 ) 
    {
        // we don't go in Low Power mode for delay below 50ms (needed for LEDs)
        RtcTimerEventAllowsLowPower = false;
    }
    else
    {
        RtcTimerEventAllowsLowPower = true;
    }

    if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
    {
        timeoutValue = timeoutValue - MCU_WAKE_UP_TIME;
    }

    RtcStartWakeUpAlarm( timeoutValue );
}

uint32_t RtcGetTimerElapsedTime( void )
{
    TimerTime_t CalendarValue = 0;

    CalendarValue = RtcGetCalendarValue( );

    return( ( uint32_t )( ceil ( ( ( CalendarValue - RtcTimerContext ) + 2 ) * RTC_ALARM_TIME_BASE ) ) );
}

uint64_t RtcGetTimerValue( void )
{
    TimerTime_t CalendarValue = 0;
    CalendarValue = RtcGetCalendarValue( );
    return( ( CalendarValue + 2 ) * RTC_ALARM_TIME_BASE );
}

static void RtcClearStatus( void )
{
    /* Clear RTC Alarm Flag */
	__HAL_RTC_ALARM_CLEAR_FLAG(&RtcHandle, RTC_FLAG_ALRAF );

	/* Enable RTC Alarm A Interrupt */
	__HAL_RTC_ALARM_EXTI_DISABLE_IT();

	/* Enable the Alarm A */
	__HAL_RTC_ALARM_DISABLE_IT(&RtcHandle, RTC_IT_ALRA);
}

static void RtcStartWakeUpAlarm( uint32_t timeoutValue )
{
    uint16_t rtcSeconds = 0;
    uint16_t rtcMinutes = 0;
    uint16_t rtcHours = 0;
    uint16_t rtcDays = 0;

    uint8_t rtcAlarmSeconds = 0;
    uint8_t rtcAlarmMinutes = 0;
    uint8_t rtcAlarmHours = 0;
    uint16_t rtcAlarmDays = 0;

    RTC_AlarmTypeDef RTC_AlarmStructure;
    RTC_TimeTypeDef  RTC_TimeStruct;
    RTC_DateTypeDef  RTC_DateStruct;

    RtcClearStatus( );
    
    RtcTimerContext = RtcGetCalendarValue( );
    HAL_RTC_GetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &RTC_DateStruct, RTC_FORMAT_BIN);
       
    timeoutValue = timeoutValue / RTC_ALARM_TIME_BASE;

    if( timeoutValue > 2160000 ) // 25 "days" in tick 
    {                            // drastically reduce the computation time
        rtcAlarmSeconds = RTC_TimeStruct.Seconds;
        rtcAlarmMinutes = RTC_TimeStruct.Minutes;
        rtcAlarmHours = RTC_TimeStruct.Hours;
        rtcAlarmDays = 25 + RTC_DateStruct.Date;  // simply add 25 days to current date and time

        if( ( RTC_DateStruct.Year == 0 ) || ( RTC_DateStruct.Year % 4 == 0 ) )
        {
            if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_DateStruct.Month - 1 ] )
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_DateStruct.Month - 1];
			    DEBUG(3,"rtcAlarmDays11 = %d\r\n",rtcAlarmDays);
            }
        }
        else
        {
            if( rtcAlarmDays > DaysInMonth[ RTC_DateStruct.Month - 1 ] )
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_DateStruct.Month - 1];
			    DEBUG(3,"rtcAlarmDays22 = %d\r\n",rtcAlarmDays);
            }
        }   
    }
    else
    {
        rtcSeconds = ( timeoutValue % SecondsInMinute ) + RTC_TimeStruct.Seconds;
        rtcMinutes = ( ( timeoutValue / SecondsInMinute ) % SecondsInMinute ) + RTC_TimeStruct.Minutes;
        rtcHours = ( ( timeoutValue / SecondsInHour ) % HoursInDay ) + RTC_TimeStruct.Hours;
        rtcDays = ( timeoutValue / SecondsInDay ) + RTC_DateStruct.Date;

        rtcAlarmSeconds = ( rtcSeconds ) % 60;
        rtcAlarmMinutes = ( ( rtcSeconds / 60 ) + rtcMinutes ) % 60;
        rtcAlarmHours   = ( ( ( ( rtcSeconds / 60 ) + rtcMinutes ) / 60 ) + rtcHours ) % 24;
        rtcAlarmDays    = ( ( ( ( ( rtcSeconds / 60 ) + rtcMinutes ) / 60 ) + rtcHours ) / 24 ) + rtcDays;

        if( ( RTC_DateStruct.Year == 0 ) || ( RTC_DateStruct.Year % 4 == 0 ) )
        {
            if( rtcAlarmDays > DaysInMonthLeapYear[ RTC_DateStruct.Month - 1 ] )            
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonthLeapYear[ RTC_DateStruct.Month - 1 ];
				DEBUG(3,"rtcAlarmDays44 = %d\r\n",rtcAlarmDays);
            }
        }
        else
        {
            if( rtcAlarmDays > DaysInMonth[ RTC_DateStruct.Month - 1 ] )            
            {   
                rtcAlarmDays = rtcAlarmDays % DaysInMonth[ RTC_DateStruct.Month - 1 ];
				DEBUG(3,"rtcAlarmDays55 = %d\r\n",rtcAlarmDays);
            }
        }
    }
    HAL_RTC_WaitForSynchro(&RtcHandle);

		RTC_AlarmStructure.AlarmTime.Hours = rtcAlarmHours;
		RTC_AlarmStructure.AlarmTime.Minutes = rtcAlarmMinutes;
		RTC_AlarmStructure.AlarmTime.Seconds = rtcAlarmSeconds;
		RTC_AlarmStructure.AlarmTime.SubSeconds = 0;
		
		RTC_AlarmStructure.AlarmDateWeekDay = ( uint8_t )rtcAlarmDays;
		
		RTC_AlarmStructure.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		RTC_AlarmStructure.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
		RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_NONE;
	
		RTC_AlarmStructure.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
		RTC_AlarmStructure.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
		
		RTC_AlarmStructure.Alarm = RTC_ALARM_A;

    if( HAL_RTC_SetAlarm_IT( &RtcHandle, &RTC_AlarmStructure, RTC_FORMAT_BIN ) != HAL_OK )
    {
        assert_param( FAIL );
    }

		/* Wait for RTC APB registers synchronisation */
    HAL_RTC_WaitForSynchro(&RtcHandle);
}

void RtcEnterLowPowerStopMode( void )
{   
//     if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
//     {   
//         // Disable IRQ while the MCU is being deinitialized to prevent race issues
//         __disable_irq( );
//     
//         BoardDeInitMcu( );
//     
//         __enable_irq( );
//     
//         /* Disable the Power Voltage Detector */
//         PWR_PVDCmd( DISABLE );

//         /* Set MCU in ULP (Ultra Low Power) */
//         PWR_UltraLowPowerCmd( ENABLE );

//         /*Disable fast wakeUp*/
//         PWR_FastWakeUpCmd( DISABLE );

//         /* Enter Stop Mode */
//         PWR_EnterSTOPMode( PWR_Regulator_LowPower, PWR_STOPEntry_WFI );
//     }
}

void RtcRecoverMcuStatus( void )
{    
//     if( TimerGetLowPowerEnable( ) == true )
//     {
//         if( ( LowPowerDisableDuringTask == false ) && ( RtcTimerEventAllowsLowPower == true ) )
//         {    
//             // Disable IRQ while the MCU is not running on HSE
//             __disable_irq( );
//     
//             /* After wake-up from STOP reconfigure the system clock */
//             /* Enable HSE */
//             RCC_HSEConfig( RCC_HSE_ON );
//             
//             /* Wait till HSE is ready */
//             while( RCC_GetFlagStatus( RCC_FLAG_HSERDY ) == RESET )
//             {}
//             
//             /* Enable PLL */
//             RCC_PLLCmd( ENABLE );
//             
//             /* Wait till PLL is ready */
//             while( RCC_GetFlagStatus( RCC_FLAG_PLLRDY ) == RESET )
//             {}
//             
//             /* Select PLL as system clock source */
//             RCC_SYSCLKConfig( RCC_SYSCLKSource_PLLCLK );
//             
//             /* Wait till PLL is used as system clock source */
//             while( RCC_GetSYSCLKSource( ) != 0x0C )
//             {}
//     
//             /* Set MCU in ULP (Ultra Low Power) */
//             PWR_UltraLowPowerCmd( DISABLE ); // add up to 3ms wakeup time
//             
//             /* Enable the Power Voltage Detector */
//             PWR_PVDCmd( ENABLE );
//                 
//             BoardInitMcu( );
//     
//             __enable_irq( );
//         }
//     }
}

void BlockLowPowerDuringTask( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus( );
    }
    LowPowerDisableDuringTask = status;
}

void RtcDelayMs( uint32_t delay )
{
    uint64_t delayValue = 0;
    uint64_t timeout = 0;

    delayValue = ( uint64_t )( delay * 1000 );
  
    // Wait delay ms
    timeout = RtcGetTimerValue( );
 
    while( ( ( RtcGetTimerValue( ) - timeout ) ) < delayValue )
    {
        __NOP( );      
    }
  
}

uint64_t RtcGetCalendarValue( void )
{
    uint64_t calendarValue = 0;
    uint8_t i = 0;

    RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
    HAL_RTC_GetTime(&RtcHandle, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&RtcHandle, &RTC_DateStruct, RTC_FORMAT_BIN);

    if( ( PreviousYear == 99 ) && ( RTC_DateStruct.Year == 0 ) )
    {
        Century++;
    }
    PreviousYear = RTC_DateStruct.Year;

    // century
    for( i = 0; i < Century; i++ )
    {
        calendarValue += ( uint64_t )( DaysInCentury * SecondsInDay );
    }

    // years
    for( i = 0; i < RTC_DateStruct.Year; i++ )
    {
        if( ( i == 0 ) || ( i % 4 == 0 ) )
        {
            calendarValue += DaysInLeapYear * SecondsInDay;
        }
        else
        {
            calendarValue += DaysInYear * SecondsInDay;
        }
    }

    // months
    if( ( RTC_DateStruct.Year == 0 ) || ( RTC_DateStruct.Year % 4 == 0 ) )
    {
        for( i = 0; i < ( RTC_DateStruct.Month - 1 ); i++ )
        {
            calendarValue += DaysInMonthLeapYear[i] * SecondsInDay;
        }
    }
    else
    {
        for( i = 0;  i < ( RTC_DateStruct.Month - 1 ); i++ )
        {
            calendarValue += DaysInMonth[i] * SecondsInDay;
        }
    }       

    // days
    calendarValue += ( ( uint32_t )RTC_TimeStruct.Seconds + 
                      ( ( uint32_t )RTC_TimeStruct.Minutes * SecondsInMinute ) +
                      ( ( uint32_t )RTC_TimeStruct.Hours * SecondsInHour ) + 
                      ( ( uint32_t )( RTC_DateStruct.Date * SecondsInDay ) ) );
    DEBUG(3,"Hours = %d : Minutes = %d : Seconds = %d \r\n", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes,
					RTC_TimeStruct.Seconds);
    DEBUG(3,"line = %d\r\n", __LINE__);
    return( calendarValue );
}


/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
