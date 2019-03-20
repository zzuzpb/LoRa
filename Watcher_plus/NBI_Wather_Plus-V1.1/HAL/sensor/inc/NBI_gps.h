#ifndef __NBI_GPS_
#define __NBI_GPS_
/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"
#include "main.h"

/* USER CODE BEGIN Includes */
#include "FIFO_Uart.h"
#include "minmea.h"
#include "usart.h"
#include "string.h"
/* USER CODE END Includes */

typedef struct
{
	int fix;
	int day;
	int month;
	int year;

	int hours;
	int minutes;
	int seconds;
	int microseconds;
	
	float latitude;
	float longitude;
	int islocal;
	
}NBI_Gpsdata;


extern NBI_Gpsdata gpsdata;

extern void gps_input(void);
extern HAL_StatusTypeDef getGpsStatus(void);


#endif





