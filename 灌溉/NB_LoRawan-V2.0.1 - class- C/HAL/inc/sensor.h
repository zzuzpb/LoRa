/*
**************************************************************************************************************
*	@file	sensor.h
*	@author 
*	@version 
*	@date    
*	@brief	≈‰÷√Œƒº˛
***************************************************************************************************************
*/
#ifndef __SENSOR_H
#define __SENSOR_H	 

#include "stm32l0xx_hal.h"

#ifdef __cplusplus
	extern "C" {
#endif

#define WaterFlow_PORT			GPIOA
#define WaterFlow_IO			GPIO_PIN_0
		
		
typedef struct{
	uint16_t temp[2];
	uint32_t pulsecount;
}Sensor_t;

extern Sensor_t WaterSensorsData;

void SamplingData(uint16_t adctemp[2]);
void WaterFlowInit(void);
		
#ifdef __cplusplus
}
#endif

#endif
